#!/usr/bin/env python3
"""Byte-match the candidate's three GNU runtime DLLs to pinned toolchains."""
from __future__ import annotations
import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import tarfile

from MIRROR_TRANSITIVE_SOURCES_V9 import download, sha256

PINS = (
    ('windows', '2e9039c6404f68bfce1eecd34172b16715565a15555c76c1652ec26265d5ae89'),
    ('linux', 'ef3bcdd10716578b9300591da68beb73753002252a46e41f316bb94a577e3c04'),
)
NAMES = ('libgcc_s_seh-1.dll', 'libstdc++-6.dll', 'libwinpthread-1.dll')


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--candidate', type=Path, required=True)
    ap.add_argument('--cache', type=Path, required=True)
    ap.add_argument('--report', type=Path, required=True)
    args = ap.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    targets = {n: sha256(args.candidate / n) for n in NAMES}
    matches = {n: [] for n in NAMES}
    references = []
    for platform, pin in PINS:
        name = f'mingw-12.0.0-gcc-14.2.0-{platform}-multilib.tar.xz'
        path = args.cache / name
        url = 'https://gstreamer.freedesktop.org/data/cerbero/toolchain/mingw/14.2.0/' + name
        download([url], path, pin)
        references.append({'archive': name, 'sha256': pin, 'url': url})
        # Stream archive contents and hash matching member basenames. Nothing
        # from the reference toolchain is extracted or executed.
        with tarfile.open(path, 'r|xz') as archive:
            for member in archive:
                base = PurePosixPath(member.name).name.lower()
                if not member.isfile() or base not in targets:
                    continue
                stream = archive.extractfile(member)
                h = hashlib.sha256()
                for block in iter(lambda: stream.read(1024 * 1024), b''):
                    h.update(block)
                if h.hexdigest() == targets[base]:
                    matches[base].append({'archive': name, 'member': member.name})
        if all(matches.values()):
            break
    passed = all(matches.values())
    data = {'check': 'Exact GNU runtime byte matches against pinned Cerbero toolchains',
            'candidate_sha256': targets, 'reference_archives': references,
            'matches': matches, 'passed': passed, 'reference_binaries_executed': False,
            'limits': 'Byte origin only; exact build/source completeness remains a separate review'}
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(data, indent=2) + '\n', encoding='utf-8')
    summary = f"GNU runtime toolchain byte matches: {sum(bool(x) for x in matches.values())}/3.\n"
    print(summary)
    if os.environ.get('GITHUB_STEP_SUMMARY'):
        with open(os.environ['GITHUB_STEP_SUMMARY'], 'a', encoding='utf-8') as f:
            f.write(summary)
    if not passed:
        raise RuntimeError('One or more GNU runtime DLLs did not match the pinned toolchains')


if __name__ == '__main__':
    main()
