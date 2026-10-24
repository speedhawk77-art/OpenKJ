#!/usr/bin/env python3
"""Preserve eight recipe-pinned upstream sources; no recipe code is executed.

This is a partial source mirror, not proof of the shipped DLL build provenance.
Downloads and verifies inputs, then writes a deterministic ZIP and manifest.
It never installs binaries, changes release state, or uploads files itself.
"""
from __future__ import annotations

import argparse
import ast
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
from pathlib import Path
import subprocess
import tarfile
import zipfile

COMMIT = 'c8981c6e66a7c877a9a9a9cdc7a14668296f3f64'
CERBERO_SHA = '1a5732762e29b3db189ddc0ac68f6efba7bc31d7e8f73d4f0fd61ca4a991d86a'
PACKAGES = ('gdk-pixbuf', 'glib', 'glib-networking', 'json-glib',
            'libiconv', 'libsoup', 'pango', 'fribidi')
BUNDLE = 'OpenKJ-v9-Transitive-Source-Batch1.zip'
MANIFEST = 'OpenKJ-v9-Transitive-Source-Batch1-Manifest.json'


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open('rb') as f:
        for block in iter(lambda: f.read(1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()


def download(urls: list[str], path: Path, expected: str) -> None:
    if path.exists():
        if sha256(path) != expected:
            raise RuntimeError('Existing file has wrong checksum: ' + str(path))
        return
    partial = path.with_name(path.name + '.partial')
    try:
        for url in urls:
            result = subprocess.run(['curl', '-fLsS', '--retry', '2',
                '--connect-timeout', '20', '--max-time', '180', '-o', str(partial), url])
            if result.returncode:
                continue
            if sha256(partial) != expected:
                raise RuntimeError('Downloaded checksum mismatch: ' + url)
            partial.replace(path)
            print('Verified source: ' + path.name, flush=True)
            return
        raise RuntimeError('All source URLs failed: ' + path.name)
    finally:
        partial.unlink(missing_ok=True)


def read_recipe(text: str, name: str) -> dict[str, str]:
    classes = [n for n in ast.parse(text).body
               if isinstance(n, ast.ClassDef) and n.name == 'Recipe']
    if len(classes) != 1:
        raise RuntimeError('Recipe class missing: ' + name)
    fields = {}
    for node in classes[0].body:
        if isinstance(node, ast.Assign):
            for target in node.targets:
                if isinstance(target, ast.Name) and target.id in (
                        'name', 'version', 'url', 'tarball_checksum'):
                    fields[target.id] = ast.literal_eval(node.value)
    if fields.get('name') != name or len(fields.get('tarball_checksum', '')) != 64:
        raise RuntimeError('Incomplete source pin: ' + name)
    return fields


def source_url(fields: dict[str, str]) -> str:
    # These expansions are from the checksum-pinned Cerbero build/source.py.
    templates = {
        'gnome': ('https://download.gnome.org/sources/',
                  '%(name)s/%(maj_ver)s/%(name)s-%(version)s', '.tar.xz'),
        'gnu': ('https://ftp.gnu.org/gnu/',
                '%(name)s/%(name)s-%(version)s', '.tar.xz'),
    }
    url = fields['url']
    for scheme, (base, middle, suffix) in templates.items():
        if url.startswith(scheme + '://'):
            tail = url.split('://', 1)[1]
            url = base + (middle + suffix if not tail else
                          middle + tail if tail.startswith('.') else tail)
            break
    values = dict(fields, maj_ver='.'.join(fields['version'].split('.')[:2]))
    url = url % values
    if not url.startswith('https://'):
        raise RuntimeError('Unsupported source URL: ' + url)
    return url


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    cache = output / 'verified-inputs'
    cache.mkdir(exist_ok=True)
    cerbero = cache / 'cerbero-1.26.9-source-and-recipes.tar.gz'
    download(['https://codeload.github.com/GStreamer/cerbero/tar.gz/' + COMMIT],
             cerbero, CERBERO_SHA)
    sources = []
    with tarfile.open(cerbero, 'r:gz') as archive:
        for name in sorted(PACKAGES):
            member = f'cerbero-{COMMIT}/recipes/{name}.recipe'
            stream = archive.extractfile(member)
            if stream is None:
                raise RuntimeError('Missing recipe: ' + member)
            raw = stream.read()
            fields = read_recipe(raw.decode('utf-8'), name)
            url = source_url(fields)
            filename = url.rsplit('/', 1)[1]
            sources.append({
                'recipe': name, 'version': fields['version'],
                'source_url': url,
                'fallback_url': f'https://gstreamer.freedesktop.org/src/mirror/{name}/{filename}',
                'sha256': fields['tarball_checksum'],
                'archive': 'upstream/' + filename,
                'recipe_sha256': hashlib.sha256(raw).hexdigest(),
                'recipe_path_in_cerbero_archive': member,
            })

    def fetch(item: dict) -> None:
        path = cache / Path(item['archive']).name
        download([item['source_url'], item['fallback_url']], path, item['sha256'])
        item['bytes'] = path.stat().st_size

    with ThreadPoolExecutor(max_workers=4) as pool:
        list(pool.map(fetch, sources))
    manifest = {
        'purpose': 'Partial recipe-pinned source mirror; draft review only',
        'cerbero_commit': COMMIT, 'cerbero_archive_sha256': CERBERO_SHA,
        'cerbero_archive_in_bundle': 'build-metadata/' + cerbero.name,
        'source_package_count': len(sources), 'sources': sources,
        'complete_corresponding_source': False, 'binary_build_match_proven': False,
        'release_approved': False,
        'remaining_gaps': [
            'All other transitive libraries and any additional build dependencies',
            'Exact shipped SDK build configuration and source/binary matching',
            'MinGW/GCC toolchain sources and libffi Git source',
            'Application executable build revision attestation',
            'Windows playback and rebuilt-installer validation',
        ],
    }
    data = (json.dumps(manifest, indent=2, sort_keys=True) + '\n').encode()
    (output / MANIFEST).write_bytes(data)
    with zipfile.ZipFile(output / BUNDLE, 'w', compression=zipfile.ZIP_STORED) as bundle:
        def add(name: str, content: bytes) -> None:
            entry = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
            entry.external_attr = 0o100644 << 16
            bundle.writestr(entry, content)
        add(MANIFEST, data)
        add(manifest['cerbero_archive_in_bundle'], cerbero.read_bytes())
        for item in sources:
            add(item['archive'], (cache / Path(item['archive']).name).read_bytes())
    print(f'PASS: {len(sources)} source archives verified and bundled with pinned recipes/patches.')
    print('Bundle SHA-256: ' + sha256(output / BUNDLE))


if __name__ == '__main__':
    main()
