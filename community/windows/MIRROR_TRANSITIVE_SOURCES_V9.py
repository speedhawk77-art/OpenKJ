#!/usr/bin/env python3
"""Preserve recipe-pinned upstream source batches; no recipe code is executed.

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
BATCHES = {
    1: ('gdk-pixbuf', 'glib', 'glib-networking', 'json-glib',
        'libiconv', 'libsoup', 'pango', 'fribidi'),
    2: ('cairo', 'libnice', 'mpg123', 'lame', 'libltc', 'taglib',
        'libdv', 'librsvg', 'sbc', 'spandsp', 'zbar', 'srt'),
    3: ('libass', 'bzip2', 'openssl', 'dav1d', 'expat', 'flac',
        'fontconfig', 'freetype', 'graphene', 'harfbuzz', 'libjpeg-turbo',
        'nghttp2', 'libogg', 'openh264', 'openjpeg', 'opus', 'orc',
        'pcre2', 'pixman', 'libpng', 'libpsl', 'speex', 'sqlite3',
        'libsrtp', 'svt-av1', 'svt-jpeg-xs', 'libtheora', 'tiff',
        'libvorbis', 'wavpack', 'libxml2', 'zlib'),
    4: ('libffi',),
    5: ('gcc-sources', 'mingw-w64-sources', 'binutils', 'gmp', 'mpfr', 'mpc', 'isl'),
}
LIBFFI_TAG = 'meson-3.2.9999.3'
LIBFFI_TAG_OBJECT = '7b21e70fa237c4ee5ac465bbb6bfd8caae9219c2'
LIBFFI_COMMIT = '258a091198308063770643cf74a0deb6e121196c'
LIBFFI_SHA = '184d16c0ebcfc7c4bb1e2dff5d0c16b51ea9892eadf6b0fedf0ae34a39f3e355'


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
            actual = sha256(partial)
            if actual != expected:
                print(f'Rejected checksum mismatch from {url}: {actual}', flush=True)
                continue
            partial.replace(path)
            print('Verified source: ' + path.name + ' from ' + url, flush=True)
            return
        raise RuntimeError('All source URLs failed: ' + path.name)
    finally:
        partial.unlink(missing_ok=True)


def source_literal(node: ast.AST, fields: dict[str, str]) -> str:
    # One tagged recipe uses a literal URL's .format(version). Accept only
    # string-literal formatting with previously read literal fields, not code.
    if (isinstance(node, ast.Call) and isinstance(node.func, ast.Attribute)
            and node.func.attr == 'format'
            and isinstance(node.func.value, ast.Constant)
            and isinstance(node.func.value.value, str) and not node.keywords):
        args = [fields[n.id] if isinstance(n, ast.Name) else ast.literal_eval(n)
                for n in node.args]
        return node.func.value.value.format(*args)
    return ast.literal_eval(node)


def read_recipe(text: str, name: str, git_source: bool = False) -> dict[str, str]:
    classes = [n for n in ast.parse(text).body
               if isinstance(n, ast.ClassDef) and n.name == 'Recipe']
    if len(classes) != 1:
        raise RuntimeError('Recipe class missing: ' + name)
    fields = {}
    for node in classes[0].body:
        if isinstance(node, ast.Assign):
            for target in node.targets:
                if isinstance(target, ast.Name) and target.id in (
                        'name', 'version', 'url', 'tarball_checksum', 'commit'):
                    fields[target.id] = source_literal(node.value, fields)
    if fields.get('name') != name or (not git_source and len(fields.get('tarball_checksum', '')) != 64):
        raise RuntimeError('Incomplete source pin: ' + name)
    return fields


def source_url(fields: dict[str, str]) -> str:
    # These expansions are from the checksum-pinned Cerbero build/source.py.
    templates = {
        'gnome': ('https://download.gnome.org/sources/',
                  '%(name)s/%(maj_ver)s/%(name)s-%(version)s', '.tar.xz'),
        'gnu': ('https://ftp.gnu.org/gnu/',
                '%(name)s/%(name)s-%(version)s', '.tar.xz'),
        'sf': ('https://download.sourceforge.net/',
               '%(name)s/%(name)s-%(version)s', '.tar.xz'),
        'xiph': ('https://downloads.xiph.org/releases/',
                 '%(name)s/%(name)s-%(version)s', '.tar.xz'),
        'savannah': ('https://download.savannah.gnu.org/releases/',
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
    parser.add_argument('--batch', type=int, choices=sorted(BATCHES), default=1)
    args = parser.parse_args()
    bundle_name = f'OpenKJ-v9-Transitive-Source-Batch{args.batch}.zip'
    manifest_name = f'OpenKJ-v9-Transitive-Source-Batch{args.batch}-Manifest.json'
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    cache = output / 'verified-inputs'
    cache.mkdir(exist_ok=True)
    cerbero = cache / 'cerbero-1.26.9-source-and-recipes.tar.gz'
    download(['https://codeload.github.com/GStreamer/cerbero/tar.gz/' + COMMIT],
             cerbero, CERBERO_SHA)
    sources = []
    with tarfile.open(cerbero, 'r:gz') as archive:
        for name in sorted(BATCHES[args.batch]):
            recipe_dir = 'recipes-toolchain' if args.batch == 5 else 'recipes'
            member = f'cerbero-{COMMIT}/{recipe_dir}/{name}.recipe'
            stream = archive.extractfile(member)
            if stream is None:
                raise RuntimeError('Missing recipe: ' + member)
            raw = stream.read()
            fields = read_recipe(raw.decode('utf-8'), name, git_source=name == 'libffi')
            if name == 'libffi':
                if fields.get('commit') != LIBFFI_TAG:
                    raise RuntimeError('Unexpected libffi recipe Git ref')
                remote = 'https://gitlab.freedesktop.org/gstreamer/meson-ports/libffi.git'
                refs = subprocess.check_output([
                    'git', 'ls-remote', remote, f'refs/tags/{LIBFFI_TAG}',
                    f'refs/tags/{LIBFFI_TAG}^{{}}'], text=True, timeout=90)
                actual_refs = dict(line.split()[::-1] for line in refs.splitlines())
                if actual_refs != {f'refs/tags/{LIBFFI_TAG}': LIBFFI_TAG_OBJECT,
                                   f'refs/tags/{LIBFFI_TAG}^{{}}': LIBFFI_COMMIT}:
                    raise RuntimeError('libffi upstream tag/commit pin mismatch')
                fields['url'] = ('https://gitlab.freedesktop.org/gstreamer/meson-ports/'
                    f'libffi/-/archive/{LIBFFI_COMMIT}/libffi-{LIBFFI_COMMIT}.tar.gz')
                # This pin is independently recorded from the immutable Git
                # commit archive, not declared as a recipe tarball checksum.
                fields['tarball_checksum'] = LIBFFI_SHA
            url = source_url(fields)
            filename = url.rsplit('/', 1)[1]
            sources.append({
                'recipe': name, 'version': fields['version'],
                'source_url': url,
                'fallback_url': f'https://gstreamer.freedesktop.org/src/mirror/{name}/{filename}',
                'sha256': fields['tarball_checksum'],
                # New batch uses package namespaces because distinct projects
                # publish archives with identical names such as v2.6.0.tar.gz.
                'archive': ('upstream/' + name + '/' + filename
                            if args.batch >= 3 else 'upstream/' + filename),
                'recipe_sha256': hashlib.sha256(raw).hexdigest(),
                'recipe_path_in_cerbero_archive': member,
            })
            if name == 'libffi':
                sources[-1].update(git_tag=LIBFFI_TAG, git_tag_object=LIBFFI_TAG_OBJECT,
                    git_commit=LIBFFI_COMMIT,
                    checksum_origin='Locally verified immutable Git commit archive; not a recipe tarball checksum')

    def fetch(item: dict) -> None:
        path = cache / Path(item['archive']).relative_to('upstream')
        path.parent.mkdir(parents=True, exist_ok=True)
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
            ('Exact compiler/runtime binary build matching; windows-default-manifest Git source'
             if args.batch == 5 else
             'MinGW/GCC toolchain sources; libffi binary build matching is unproven'
             if args.batch == 4 else 'MinGW/GCC toolchain sources and libffi Git source'),
            'Application executable build revision attestation',
            'Windows playback and rebuilt-installer validation',
        ],
    }
    if args.batch == 5:
        manifest['source_recipe_families_addressed'] = ['mingw-runtime', 'winpthreads-runtime']
        manifest['scope'] = 'Compiler/runtime and compiler build dependencies from recipes-toolchain; not proof of shipped DLL build matching'
    data = (json.dumps(manifest, indent=2, sort_keys=True) + '\n').encode()
    (output / manifest_name).write_bytes(data)
    with zipfile.ZipFile(output / bundle_name, 'w', compression=zipfile.ZIP_STORED) as bundle:
        def add(name: str, content: bytes) -> None:
            entry = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
            entry.external_attr = 0o100644 << 16
            bundle.writestr(entry, content)
        add(manifest_name, data)
        add(manifest['cerbero_archive_in_bundle'], cerbero.read_bytes())
        for item in sources:
            add(item['archive'], (cache / Path(item['archive']).relative_to('upstream')).read_bytes())
    print(f'PASS: {len(sources)} source archives verified and bundled with pinned recipes/patches.')
    print('Bundle SHA-256: ' + sha256(output / bundle_name))


if __name__ == '__main__':
    main()
