#!/usr/bin/env python3
"""CI-only compatibility experiment with a separately fetched Cisco DLL.

Copies a test candidate; never alters the installer candidate, uploads a codec,
or claims that the product meets Cisco's binary-license integration conditions.
"""
import argparse
import bz2
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys

from MIRROR_TRANSITIVE_SOURCES_V9 import download


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    for name in ('candidate', 'original', 'output', 'fixture', 'report'):
        ap.add_argument('--' + name, type=Path, required=True)
    args = ap.parse_args()
    if args.output.exists(): raise RuntimeError('Refusing to overwrite an experiment folder')
    shutil.copytree(args.candidate, args.output)
    archive = args.output.parent / 'cisco-official-2.6.0.dll.bz2'
    url = 'https://ciscobinary.openh264.org/openh264-2.6.0-win64.dll.bz2'
    download([url], archive, 'dab5f2a872777f9a58b69bfa9fbcf20d9f82f2d6ec91383fd70bff49bd34ac9f')
    data = bz2.decompress(archive.read_bytes())
    expected = '2076cb5675ec6c1a4c70e7a2a322552f547b6eeed649d6dfcd9e02a543b24691'
    if hashlib.sha256(data).hexdigest() != expected:
        raise RuntimeError('Decoded Cisco codec checksum mismatch')
    codec = (args.output / 'libopenh264-7.dll').resolve()
    codec.write_bytes(data)
    # Renaming changes the filename only, not the signed binary bytes.
    command = "$s=Get-AuthenticodeSignature -LiteralPath $args[0]; @{status=$s.Status.ToString();subject=$s.SignerCertificate.Subject} | ConvertTo-Json -Compress"
    signature = json.loads(subprocess.check_output(['pwsh', '-NoProfile', '-Command',
        '& { ' + command + ' }', str(codec)], text=True))
    args.report.parent.mkdir(parents=True, exist_ok=True)
    if signature.get('status') != 'Valid' or 'Cisco' not in signature.get('subject', ''):
        args.report.write_text(json.dumps({'signature': signature, 'codec_executed': False,
            'release_approved': False}, indent=2), encoding='utf-8')
        raise RuntimeError('Cisco signature did not validate; experiment stopped')
    plugin = Path('gst-plugins/libgstopenh264.dll')
    shutil.copy2(args.original / plugin, args.output / plugin)
    subprocess.run([sys.executable, str(Path(__file__).with_name('PROBE_GSTREAMER_CANDIDATE_V9.py')),
        '--candidate', str(args.output), '--fixture', str(args.fixture), '--report', str(args.report)], check=True)
    result = json.loads(args.report.read_text())
    result.update(experiment='Official Cisco codec, separately downloaded on temporary CI runner',
                  official_download_url=url, codec_sha256=expected, signature=signature,
                  codec_was_bundled_in_installer=False,
                  product_license_integration_completed=False)
    args.report.write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    print('Cisco compatibility H.264 result:', result['pipelines']['h264_mp4'])


if __name__ == '__main__': main()
