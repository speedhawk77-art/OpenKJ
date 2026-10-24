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
                           pefile.DIRECTORY_ENTRY['IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT']]
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
        rows.append({'path': path.relative_to(root).as_posix(),
                     'sha256': hashlib.sha256(path.read_bytes()).hexdigest(),
                     'imports': imports})
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
    args = ap.parse_args()
    before = scan(args.original.resolve())
    after = scan(args.candidate.resolve())
    if len(before) != 340 or len(after) != 338:
        raise RuntimeError('Expected exactly 340 original and 338 candidate program binaries')
    dependencies = codec_edges(after)
    result = {
        'check': 'Normal and delay PE imports of omitted OpenH264 components',
        'parser': 'pefile ' + pefile.__version__,
        'original_binary_count': len(before), 'candidate_binary_count': len(after),
        'original_openh264_import_edges': codec_edges(before),
        'candidate_openh264_import_edges': dependencies,
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
