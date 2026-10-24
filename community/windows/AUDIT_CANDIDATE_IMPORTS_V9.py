#!/usr/bin/env python3
"""Read normal/delay PE import tables without executing application binaries.

This narrowly detects static imports of omitted OpenH264 components. It does
not validate LoadLibrary calls, forwarded exports, decoder availability or
media playback, and does not resolve all Windows/system DLL dependencies.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
from pathlib import Path

import pefile


def scan(root: Path) -> list[dict]:
    rows = []
    for path in sorted(root.rglob('*')):
        if not path.is_file() or path.suffix.lower() not in {'.dll', '.exe', '.pyd', '.ocx'}:
            continue
        if path.name.lower() == 'vc_redist.x64.exe':
            continue
        if path.is_symlink():
            raise RuntimeError('Unexpected binary symlink: ' + str(path))
        with pefile.PE(str(path), fast_load=True) as pe:
            directories = [pefile.DIRECTORY_ENTRY['IMAGE_DIRECTORY_ENTRY_IMPORT'],
                           pefile.DIRECTORY_ENTRY['IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT'],
                           pefile.DIRECTORY_ENTRY['IMAGE_DIRECTORY_ENTRY_RESOURCE']]
            pe.parse_data_directories(directories=directories)
            imports = {}
            for label, index, attr in (
                ('normal', directories[0], 'DIRECTORY_ENTRY_IMPORT'),
                ('delay', directories[1], 'DIRECTORY_ENTRY_DELAY_IMPORT'),
            ):
                directory = pe.OPTIONAL_HEADER.DATA_DIRECTORY[index]
                if directory.VirtualAddress and not hasattr(pe, attr):
                    raise RuntimeError('Could not parse ' + label + ' imports: ' + str(path))
                imports[label] = sorted({entry.dll.decode('ascii').lower()
                                         for entry in getattr(pe, attr, [])})
            versions={}
            for group in getattr(pe,'FileInfo',[]):
                for entry in group:
                    for table in getattr(entry,'StringTable',[]):
                        for key,value in table.entries.items():
                            if key in {b'CompanyName',b'FileDescription',b'FileVersion',b'ProductName',b'ProductVersion'}:
                                versions[key.decode()]=value.decode('utf-8',errors='replace')
        payload=path.read_bytes()
        rows.append({'path': path.relative_to(root).as_posix(),
                     'sha256': hashlib.sha256(payload).hexdigest(),
                     'imports': imports,'version_resources':versions})
        if path.name.lower()=='opengl32sw.dll':
            rows[-1]['embedded_renderer_version_strings']=sorted({
                x.decode('ascii') for x in re.findall(
                    rb'\b(?:Mesa|LLVM) [0-9]+(?:\.[0-9]+){1,3}(?:-[A-Za-z0-9.]+)?',payload)})
            print('Software OpenGL embedded renderer versions:',rows[-1]['embedded_renderer_version_strings'])
    return rows


def codec_edges(rows: list[dict]) -> list[dict]:
    return [{'path': row['path'], 'kind': kind, 'dll': dll}
            for row in rows for kind, names in row['imports'].items()
            for dll in names if 'openh264' in dll]


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--original', type=Path, required=True)
    ap.add_argument('--candidate', type=Path, required=True)
    ap.add_argument('--report', type=Path, required=True)
    ap.add_argument('--isolated-optional-adapter',action='store_true',help='Allow only the byte-identical original adapter outside the scanned plugin directory')
    args = ap.parse_args()
    before = scan(args.original.resolve())
    after = scan(args.candidate.resolve())
    expected_count=339 if args.isolated_optional_adapter else 338
    if len(before) != 340 or len(after) != expected_count:
        raise RuntimeError('Expected exactly 340 original and 338 candidate program binaries')
    dependencies = codec_edges(after)
    isolated=[]
    if args.isolated_optional_adapter:
        original=next(r for r in before if r['path']=='gst-plugins/libgstopenh264.dll')
        adapter=next(r for r in after if r['path']=='optional-codecs/openh264/libgstopenh264.dll')
        if adapter['sha256']!=original['sha256']:raise RuntimeError('Optional adapter differs from pinned original')
        isolated=[e for e in dependencies if e['path']==adapter['path']]
        if not isolated:raise RuntimeError('Expected adapter import was not found')
        dependencies=[e for e in dependencies if e['path']!=adapter['path']]

    result = {
        'check': 'Normal and delay PE imports of omitted OpenH264 components',
        'parser': 'pefile ' + pefile.__version__,
        'original_binary_count': len(before), 'candidate_binary_count': len(after),
        'original_openh264_import_edges': codec_edges(before),
        'candidate_openh264_import_edges': dependencies,
        'isolated_optional_adapter_import_edges':isolated,
        'passed': not dependencies, 'binaries_executed': False,
        'limitations': ['Dynamic library loading and forwarded exports are not checked',
                        'No decoder availability or playback guarantee',
                        'Other Windows/system dependencies are not resolved'],
        'candidate_import_inventory': after,
    }
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    summary = (f"Scanned {len(after)} retained program binaries: "
               f"{len(dependencies)} normal/delay OpenH264 import edges remain. "
               'Playback and dynamic loading still need Windows testing.\n')
    print(summary)
    if os.environ.get('GITHUB_STEP_SUMMARY'):
        with open(os.environ['GITHUB_STEP_SUMMARY'], 'a', encoding='utf-8') as f:
            f.write(summary)
    if dependencies:
        raise RuntimeError('Retained binaries still import omitted OpenH264 DLLs')


if __name__ == '__main__':
    main()
