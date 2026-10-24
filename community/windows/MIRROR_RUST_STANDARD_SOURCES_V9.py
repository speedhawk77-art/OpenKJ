#!/usr/bin/env python3
"""Preserve Cerbero-pinned Rust standard-library source and locked dependencies."""
import argparse,hashlib,json,re,tarfile,tomllib,zipfile
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
from MIRROR_TRANSITIVE_SOURCES_V9 import download
from COLLECT_UPSTREAM_NOTICES_V9 import legal_name

CHANNEL='009e8b5ff43f12bf644b5e5b9fd89f96453072062a450c623882f64e85405da5'
SOURCE='e8f242b4bdfdc2c3f7e51016e51832ebf3dce214138f2636a37d90db110ea204'
EVIDENCE='OpenKJ-v9-Upstream-Notice-Evidence-v2.json'
def sha(raw):return hashlib.sha256(raw).hexdigest()
def zipitems(path,items):
 with zipfile.ZipFile(path,'w',zipfile.ZIP_DEFLATED) as z:
  for name,raw in sorted(items.items()):
   i=zipfile.ZipInfo(name,(1980,1,1,0,0,0));i.compress_type=zipfile.ZIP_DEFLATED;i.external_attr=0o100644<<16;z.writestr(i,raw)
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--output',type=Path,required=True);a=ap.parse_args();a.output.mkdir(parents=True,exist_ok=True)
 inputs=a.output/'inputs';inputs.mkdir(exist_ok=True)
 channel=inputs/'channel-rust-1.85.0.toml';download(['https://static.rust-lang.org/dist/channel-rust-1.85.0.toml'],channel,CHANNEL)
 c=tomllib.loads(channel.read_text());p=c['pkg']['rust-src']['target']['*'];assert p['xz_hash']==SOURCE
 src=inputs/'rust-src-1.85.0.tar.xz';download([p['xz_url']],src,SOURCE)
 registry={};locks=[]
 with tarfile.open(src) as t:
  for m in t:
   if m.isfile() and m.name.endswith('/Cargo.lock'):
    raw=t.extractfile(m).read();locks.append({'path':m.name,'sha256':sha(raw)})
    for pkg in tomllib.loads(raw.decode())['package']:
     if 'source' not in pkg:continue
     if pkg['source']!='registry+https://github.com/rust-lang/crates.io-index':raise RuntimeError('Unexpected non-registry standard-library dependency')
     if not re.fullmatch('[A-Za-z0-9_-]+',pkg['name']) or not re.fullmatch('[0-9A-Za-z.+_-]+',pkg['version']):raise RuntimeError('Unsafe crate')
     if not re.fullmatch('[0-9a-f]{64}',pkg.get('checksum','')):raise RuntimeError('Missing lock checksum')
     name=f"{pkg['name']}-{pkg['version']}.crate"
     if name in registry and registry[name]!=pkg['checksum']:raise RuntimeError('Conflicting crate checksum')
     registry[name]=pkg['checksum']
 def fetch(item):
  name,pin=item;crate=name.rsplit('-',1)[0];download([f'https://static.crates.io/crates/{crate}/{name}'],inputs/name,pin)
 with ThreadPoolExecutor(max_workers=8) as pool:list(pool.map(fetch,registry.items()))
 docs={};evidence=[]
 for path,pin in [(src,SOURCE)]+[(inputs/n,p) for n,p in sorted(registry.items())]:
  with tarfile.open(path) as t:
   for m in t:
    if not m.isfile() or not legal_name(m.name) or m.size>2*1024*1024:continue
    raw=t.extractfile(m).read()
    if b'\0' in raw:continue
    parts=Path(m.name).parts
    if '..' in parts or Path(m.name).is_absolute() or '\\' in m.name:raise RuntimeError('Unsafe legal path')
    name='rust-standard-library/'+m.name
    if name in docs:raise RuntimeError('Duplicate legal path')
    docs[name]=raw;evidence.append({'installed_supplement_path':name,'sha256':sha(raw),'source_archive':path.name,'source_archive_sha256':pin,'source_member':m.name,'bytes':len(raw)})
 metadata={'rust_version':'1.85.0','cerbero_commit':'c8981c6e66a7c877a9a9a9cdc7a14668296f3f64','channel_sha256':CHANNEL,'rust_src_sha256':SOURCE,
  'locked_registry_sources':registry,'lockfiles':locks,'source_package_count':1+len(registry),'document_count':len(docs),'documents':evidence,
  'actual_shipped_binary_build_match_proven':False,'legal_completeness_verified':False,'release_approved':False}
 encoded=(json.dumps(metadata,indent=2,sort_keys=True)+'\n').encode()
 (a.output/'OpenKJ-v9-Rust-Standard-Source-Manifest.json').write_bytes(encoded)
 zipitems(a.output/'OpenKJ-v9-Rust-Standard-Sources.zip',{**{x.name:x.read_bytes() for x in inputs.iterdir() if x.is_file()},'OpenKJ-v9-Rust-Standard-Source-Manifest.json':encoded})
 zipitems(a.output/'OpenKJ-v9-Rust-Standard-Notices.zip',{**docs,EVIDENCE:encoded})
 print('Rust standard library source: 1.85.0; locked registry sources:',len(registry),'legal documents:',len(docs))
if __name__=='__main__':main()
