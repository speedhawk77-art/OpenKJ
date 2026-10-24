#!/usr/bin/env python3
"""Build/install/re-extract a separate test installer and compare its files.

Runs only on an isolated Windows build runner. No release upload/publication.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys

from MIRROR_TRANSITIVE_SOURCES_V9 import download, sha256
from STAGE_OPENKJ_COMPLIANCE_CANDIDATE_V9 import bins, VC_SHA256


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    for name in ('candidate', 'work', 'report', 'unpacker'):
        ap.add_argument('--' + name, type=Path, required=True)
    ap.add_argument('--source-attestation', type=Path, help='Verified source-built candidate manifest (339 runtime files)')
    args = ap.parse_args()
    root = args.candidate.resolve(); work = args.work.resolve()
    if work.exists(): raise RuntimeError('Refusing to reuse installer test directory')
    work.mkdir(parents=True)
    setup = work / 'innosetup-6.4.3.exe'
    download(['https://github.com/jrsoftware/issrc/releases/download/is-6_4_3/innosetup-6.4.3.exe'],
             setup, 'f3c42116542c4cc57263c5ba6c4feabfc49fe771f2f98a79d2f7628b8762723b')
    compiler_dir = work / 'compiler'
    subprocess.run([str(setup), '/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART', '/SP-',
                    '/DIR=' + str(compiler_dir)], check=True)
    compiler = compiler_dir / 'ISCC.exe'
    if not compiler.is_file(): raise RuntimeError('Pinned Inno compiler not installed')
    env = dict(os.environ, INNO_SETUP_COMPILER=str(compiler))
    builder = Path(__file__).with_name('MAKE_SINGLE_FILE_INSTALLER.py')
    subprocess.run([sys.executable, str(builder), '--source', str(root)], env=env, check=True)
    installer = root.parent / 'OpenKJ-Installer-Output/OpenKJ-v9-Setup.exe'
    extracted = work / 're-extracted'
    subprocess.run([str(args.unpacker.resolve()), '-x', '-a', '-b', '-n', '-y',
                    '-d' + str(extracted), str(installer)], check=True)
    vc = [p for p in extracted.rglob('*') if p.is_file() and p.name.lower() == 'vc_redist.x64.exe']
    if len(vc) != 1 or sha256(vc[0]) != VC_SHA256 or vc[0].stat().st_size != 18731856:
        raise RuntimeError('Rebuilt embedded runtime identity check failed')
    installed = work / 'installed'
    p = subprocess.run([str(installer), '/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART',
        '/SP-', '/NOICONS', '/DIR=' + str(installed), '/LOG=' + str(work / 'install.log')])
    if p.returncode not in (0, 3010): raise RuntimeError('Test installation failed: ' + str(p.returncode))
    attestation = json.loads(args.source_attestation.read_text()) if args.source_attestation else None
    if attestation and (attestation['current_build_source_attested'] is not True or attestation['executable_sha256'] != sha256(root/'openkj.exe') or attestation['cisco_binary_bundled'] is not False):
        raise RuntimeError('Source application attestation mismatch')
    expected_count = 339 if attestation else 338
    expected = bins(root); actual = bins(installed)
    generated = {k for k in actual if k not in expected}
    if len(expected) != expected_count or len(generated) != 1 or not next(iter(generated)).lower().startswith('unins'):
        raise RuntimeError('Unexpected installed runtime inventory')
    if any(actual.get(k) != v for k, v in expected.items()):
        raise RuntimeError('Installed program binary hash mismatch')
    notices = {p.relative_to(root).as_posix(): sha256(p)
               for p in (root / 'licenses').rglob('*') if p.is_file()}
    for name, pin in notices.items():
        target = installed / name
        if not target.is_file() or sha256(target) != pin:
            raise RuntimeError('Installed notice mismatch: ' + name)
    for name in ('libopenh264-7.dll', 'gst-plugins/libgstopenh264.dll'):
        if (installed / name).exists(): raise RuntimeError('Omitted codec returned in installed candidate')
    data = {'check': 'Separate installer build, install and embedded-runtime round trip',
        'installer_sha256': sha256(installer), 'installer_bytes': installer.stat().st_size,
        'embedded_vc_sha256': VC_SHA256, 'installed_runtime_hash_matches': len(expected),
        'installed_notice_hash_matches': len(notices), 'generated_uninstaller': sorted(generated),
        'install_exit_code': p.returncode, 'installed_path': str(installed),
        'candidate_source_commit': os.environ.get('GITHUB_SHA'),
        'application_binary_build_commit_attested': bool(attestation),
        'application_source_commit': attestation['source_commit'] if attestation else None,
        'physical_playback_tested': False, 'release_approved': False, 'passed': True}
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(data, indent=2) + '\n', encoding='utf-8')
    print(f'Installer round trip passed: {len(expected)} program and {len(notices)} notice hashes match.')


if __name__ == '__main__': main()
