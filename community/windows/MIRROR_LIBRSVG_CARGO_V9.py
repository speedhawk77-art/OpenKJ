#!/usr/bin/env python3
"""Preserve registry sources pinned by librsvg's actual shipped Cargo.lock.

Downloads/unpacks source data only; no Cargo/Rust/build-script execution.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
from pathlib import Path,PurePosixPath
import re
import tarfile
import tomllib
import zipfile
from MIRROR_TRANSITIVE_SOURCES_V9 import download
from COLLECT_UPSTREAM_NOTICES_V9 import legal_name

BASE_PIN='ecd293fb0cc338c170171bbc7bcfbea6725d041c95f31385dc935409933e4597'
EVIDENCE='OpenKJ-v9-Upstream-Notice-Evidence-v2.json'


def sha(data):return hashlib.sha256(data).hexdigest()


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output',type=Path,required=True);ap.add_argument('--cache',type=Path)
    a=ap.parse_args();a.output.mkdir(parents=True,exist_ok=True)
    cache=a.cache or a.output/'inputs';cache.mkdir(parents=True,exist_ok=True)
    base=cache/'librsvg-2.59.2.tar.xz'
    download(['https://download.gnome.org/sources/librsvg/2.59/librsvg-2.59.2.tar.xz'],base,BASE_PIN)
    with tarfile.open(base) as t:
        lock=t.extractfile('librsvg-2.59.2/Cargo.lock').read()
    packages=tomllib.loads(lock.decode())['package'];registry=[];local=[]
    for p in packages:
        if 'source' not in p:local.append({'name':p['name'],'version':p['version']});continue
        if p['source']!='registry+https://github.com/rust-lang/crates.io-index':
            raise RuntimeError('Non-registry dependency needs separate Git source handling')
        if not re.fullmatch(r'[A-Za-z0-9_-]+',p['name']) or not re.fullmatch(r'[A-Za-z0-9.+_-]+',p['version']):
            raise RuntimeError('Unsafe crate name/version')
        if not re.fullmatch(r'[0-9a-f]{64}',p.get('checksum','')):raise RuntimeError('Missing crate checksum')
        registry.append(dict(name=p['name'],version=p['version'],sha256=p['checksum']))
    registry.sort(key=lambda p:(p['name'],p['version']))
    def fetch(p):
        p['filename']=f"{p['name']}-{p['version']}.crate"
        p['url']=f"https://static.crates.io/crates/{p['name']}/{p['filename']}"
        download([p['url']],cache/p['filename'],p['sha256'])
    with ThreadPoolExecutor(max_workers=8) as pool:list(pool.map(fetch,registry))
    notices={};docs=[];counts=[];total=0
    source_zip=a.output/'OpenKJ-v9-librsvg-2.59.2-Cargo-Sources.zip'
    with zipfile.ZipFile(source_zip,'w',compression=zipfile.ZIP_DEFLATED) as z:
        def add(name,data):
            i=zipfile.ZipInfo(name,(1980,1,1,0,0,0));i.compress_type=zipfile.ZIP_DEFLATED
            i.external_attr=0o100644<<16;z.writestr(i,data)
        add(base.name,base.read_bytes());add('Cargo.lock',lock)
        add('.cargo/config.toml',b'[source.crates-io]\nreplace-with = "vendored-sources"\n\n[source.vendored-sources]\ndirectory = "vendor"\n')
        for package in registry:
            crate=cache/package['filename'];checks={};count=0
            with tarfile.open(crate,'r:gz') as t:
                for m in t:
                    if m.isdir():continue
                    if not m.isfile():raise RuntimeError('Unexpected non-regular Cargo source member')
                    parts=PurePosixPath(m.name).parts
                    if len(parts)<2 or '..' in parts or PurePosixPath(m.name).is_absolute() or '\\' in m.name:
                        raise RuntimeError('Unsafe crate source path')
                    if m.size>64*1024*1024:raise RuntimeError('Unexpectedly large source member')
                    rel='/'.join(parts[1:]);raw=t.extractfile(m).read();total+=len(raw)
                    if total>1024*1024*1024:raise RuntimeError('Source expansion size limit')
                    if rel in checks:raise RuntimeError('Duplicate crate source path')
                    checks[rel]=sha(raw)
                    add(f"vendor/{package['name']}-{package['version']}/{rel}",raw)
                    if (legal_name(m.name) and len(raw)<2*1024*1024 and b'\0' not in raw
                            and PurePosixPath(rel).suffix.lower() not in {'.py','.ps1','.bat','.cmd','.exe','.dll'}):
                        clean='/'.join('_dot_'+x[1:] if x.startswith('.') else x for x in PurePosixPath(rel).parts)
                        target=f"rust-crates/{package['name']}-{package['version']}/{clean}"
                        if target in notices:raise RuntimeError('Duplicate crate notice path')
                        notices[target]=raw;count+=1
                        docs.append({'installed_supplement_path':target,'sha256':sha(raw),
                            'source_archive':package['filename'],'source_archive_sha256':package['sha256'],
                            'source_member':m.name,'bytes':len(raw)})
            add(f"vendor/{package['name']}-{package['version']}/.cargo-checksum.json",
                (json.dumps({'files':checks,'package':package['sha256']},sort_keys=True)+'\n').encode())
            package['source_files']=len(checks)
            counts.append({'recipe':package['name'],'version':package['version'],
                           'legal_files_preserved':count,'source_sha256':package['sha256']})
        meta={'purpose':'Pinned librsvg registry dependency sources and build-ready vendor tree',
              'librsvg_source_sha256':BASE_PIN,'cargo_lock_sha256':sha(lock),
              'registry_package_count':len(registry),'local_workspace_packages_in_base_source':local,
              'packages':registry,'source_files_bytes':total,'cargo_build_executed':False,
              'actual_shipped_binary_build_match_proven':False,'release_approved':False}
        encoded=(json.dumps(meta,indent=2,sort_keys=True)+'\n').encode()
        (a.output/'OpenKJ-v9-librsvg-Cargo-Source-Manifest.json').write_bytes(encoded)
        add('OpenKJ-v9-librsvg-Cargo-Source-Manifest.json',encoded)
        add('README-SOURCES.txt',b'Unpack librsvg-2.59.2.tar.xz beside this vendor directory. The parent .cargo/config.toml selects the pinned vendored registry source. Apply the pinned Cerbero patches/build configuration separately. A compatible Rust toolchain and other library build dependencies are still required. No successful build or shipped-binary correspondence is asserted.\n')
    if len({n.casefold() for n in notices})!=len(notices):raise RuntimeError('Windows crate notice path collision')
    legal={'purpose':'Original Rust crate legal text preservation',
           'source_package_count':len(registry),'document_count':len(notices),'packages':counts,
           'documents':docs,'legal_completeness_verified':False,'binary_source_correspondence_proven':False,
           'release_approved':False,'limits':['Standalone filename-based notice collection; inspect any crate with zero collected notices']}
    with zipfile.ZipFile(a.output/'OpenKJ-v9-librsvg-Rust-Notices.zip','w',compression=zipfile.ZIP_DEFLATED) as z:
        def put(name,raw):
            i=zipfile.ZipInfo(name,(1980,1,1,0,0,0));i.compress_type=zipfile.ZIP_DEFLATED;i.external_attr=0o100644<<16;z.writestr(i,raw)
        put(EVIDENCE,(json.dumps(legal,indent=2,sort_keys=True)+'\n').encode())
        for name in sorted(notices):put(name,notices[name])
    print(f'Verified {len(registry)} locked registry sources; preserved {len(notices)} original legal documents.')
    print('Crates with no standalone legal file:',[x['recipe']+' '+x['version'] for x in counts if not x['legal_files_preserved']])


if __name__=='__main__':main()
