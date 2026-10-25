#!/usr/bin/env python3
"""Preserve source and attribution data inside retained GStreamer plugins.

Reads only pinned archives/lockfiles/recipes. Never executes source build scripts.
This supplements, and does not replace, the existing transitive-DLL mirrors.
"""
import argparse,hashlib,json,re,tarfile,tomllib,zipfile
from pathlib import Path,PurePosixPath
from concurrent.futures import ThreadPoolExecutor
from MIRROR_TRANSITIVE_SOURCES_V9 import download,read_recipe,source_url,COMMIT,CERBERO_SHA
from COLLECT_UPSTREAM_NOTICES_V9 import legal_name
GIT_COMMIT='901616eec08a80516b4787dcf52e4889ee6cd1ba'
GIT_TAG_OBJECT='390e98cc9e936f8c4ab2cce8afe1ca82d5dd03fb'
RUST_PIN='d6f227151f66259629a94c9a27673a9766d7ae37c99daa28816c2cc99b5e4784'
EXTRA=('libvpx','soundtouch','qrencode','webrtc-audio-processing','directxmath','directx-headers','ladspa')
EVIDENCE='OpenKJ-v9-Upstream-Notice-Evidence-v2.json'
def sha(raw):return hashlib.sha256(raw).hexdigest()
def put(z,name,raw):
 i=zipfile.ZipInfo(name,(1980,1,1,0,0,0));i.compress_type=zipfile.ZIP_DEFLATED;i.external_attr=0o100644<<16;z.writestr(i,raw)
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args();a.output.mkdir(parents=True,exist_ok=True)
 cache=a.output/'inputs';cache.mkdir(exist_ok=True)
 recipe_archive=cache/'cerbero-source-and-recipes.tar.gz';download(['https://codeload.github.com/GStreamer/cerbero/tar.gz/'+COMMIT],recipe_archive,CERBERO_SHA)
 source=cache/('gst-plugins-rs-'+GIT_COMMIT+'.tar.gz')
 download([f'https://gitlab.freedesktop.org/gstreamer/gst-plugins-rs/-/archive/{GIT_COMMIT}/gst-plugins-rs-{GIT_COMMIT}.tar.gz'],source,RUST_PIN)
 with tarfile.open(source) as t:
  lock=t.extractfile(f'gst-plugins-rs-{GIT_COMMIT}/Cargo.lock').read()
 packages=tomllib.loads(lock.decode())['package'];registry=[];git_sources=set()
 for p in packages:
  if p.get('source','').startswith('git+'):
   git_sources.add(p['source']);continue
  if 'source' not in p:continue
  if p['source']!='registry+https://github.com/rust-lang/crates.io-index':raise RuntimeError('Unknown Cargo source')
  if not re.fullmatch('[A-Za-z0-9_-]+',p['name']) or not re.fullmatch('[0-9A-Za-z.+_-]+',p['version']) or not re.fullmatch('[0-9a-f]{64}',p.get('checksum','')):raise RuntimeError('Invalid crate source pin')
  registry.append({'name':p['name'],'version':p['version'],'filename':f"{p['name']}-{p['version']}.crate",'sha256':p['checksum']})
 pins=json.loads(Path(__file__).with_name('data').joinpath('plugin-source-git-pins-v9.json').read_text())
 if git_sources!={p['cargo_source'] for p in pins}:raise RuntimeError('Locked Git source set changed')
 inputs=[{'filename':source.name,'sha256':RUST_PIN,'kind':'GStreamer Rust plugin workspace','git_commit':GIT_COMMIT},
         {'filename':recipe_archive.name,'sha256':CERBERO_SHA,'kind':'Cerbero source, build recipes and patches'}]
 def fetch(p):
  download([f"https://static.crates.io/crates/{p['name']}/{p['filename']}"],cache/p['filename'],p['sha256'])
 with ThreadPoolExecutor(max_workers=8) as pool:list(pool.map(fetch,registry))
 inputs.extend(dict(p,kind='locked registry source') for p in registry)
 for p in pins:
  if not re.fullmatch('[0-9a-f]{40}',p['git_commit']) or not re.fullmatch('[A-Za-z0-9_.-]+',p['filename']):raise RuntimeError('Unsafe Git source pin')
  download([p['archive_url']],cache/p['filename'],p['archive_sha256'])
  inputs.append({'filename':p['filename'],'sha256':p['archive_sha256'],'kind':'locked Git source snapshot',**p})
 with tarfile.open(recipe_archive) as t:
  for name in EXTRA:
   raw=t.extractfile(f'cerbero-{COMMIT}/recipes/{name}.recipe').read()
   fields=read_recipe(raw.decode(),name);url=source_url(fields);filename=name+'-source.tar.gz'
   download([url,f'https://gstreamer.freedesktop.org/src/mirror/{url.rsplit("/",1)[1]}'],cache/filename,fields['tarball_checksum'])
   inputs.append({'filename':filename,'sha256':fields['tarball_checksum'],'kind':'static/header plugin dependency','recipe':name,'recipe_source_sha256':sha(raw),'source_url':url,'version':fields['version']})
 for item in json.loads(Path(__file__).with_name('data').joinpath('plugin-module-source-pins-v9.json').read_text()):
  download([item['url']],cache/item['filename'],item['sha256'])
  inputs.append(dict(item,kind='retained C GStreamer module source and vendored headers'))
 documents={};records={}
 def remember(raw,item,member,kind):
  key=sha(raw);name='plugin-source-notices/'+kind+'/'+key+'.txt'
  documents[name]=raw
  record=records.setdefault(name,{'installed_supplement_path':name,'sha256':key,'bytes':len(raw),'original_sources':[]})
  record['original_sources'].append({'archive':item['filename'],'archive_sha256':item['sha256'],'member':member,'preservation':kind})
 # Legal documents and actual source attribution prefixes are retained bytewise.
 # Identical prefixes are stored once with all original locations in evidence.
 for item in inputs:
  with tarfile.open(cache/item['filename'],'r|*') as t:
   for m in t:
    if not m.isfile() or m.size>2*1024*1024:continue
    p=PurePosixPath(m.name)
    if p.is_absolute() or '..' in p.parts or '\\' in m.name:raise RuntimeError('Unsafe source member')
    wanted=legal_name(m.name) or p.name in ('Cargo.toml','Cargo.toml.orig') or p.suffix.lower() in ('.c','.cpp','.cc','.h','.hpp','.rs')
    if not wanted:continue
    raw=t.extractfile(m).read()
    if b'\0' in raw:continue
    if legal_name(m.name) or p.name in ('Cargo.toml','Cargo.toml.orig'):remember(raw,item,m.name,'original-documents');continue
    prefix=[];inside=False
    for line in raw.splitlines(keepends=True):
     s=line.lstrip()
     if inside:
      prefix.append(line)
      if b'*/' in line:inside=False
     elif not s.strip() or s.startswith(b'//'):prefix.append(line)
     elif s.startswith(b'/*'):prefix.append(line);inside=b'*/' not in s
     else:break
    header=b''.join(prefix)
    if re.search(rb'copyright|licen[cs]e|SPDX|permission|patent|public domain',header,re.I):remember(header,item,m.name,'source-headers')
 data={'workspace_git_tag':'gstreamer-1.26.9','workspace_tag_object':GIT_TAG_OBJECT,'workspace_git_commit':GIT_COMMIT,
  'workspace_archive_sha256':RUST_PIN,'cargo_lock_sha256':sha(lock),'registry_sources':len(registry),'git_source_snapshots':len(pins),
  'extra_static_header_recipe_sources':list(EXTRA),'inputs':inputs,'source_package_count':len(inputs),
  'document_count':len(documents),'documents':list(records.values()),'upstream_build_code_executed':False,
  'offline_vendor_tree_generated':False,'exact_build_reproduced':False,'legal_completeness_verified':False,'release_approved':False}
 encoded=(json.dumps(data,indent=2,sort_keys=True)+'\n').encode()
 (a.output/'OpenKJ-v9-Retained-Plugin-Source-Manifest.json').write_bytes(encoded)
 with zipfile.ZipFile(a.output/'OpenKJ-v9-Retained-Plugin-Sources.zip','w',zipfile.ZIP_DEFLATED) as z:
  for item in inputs:put(z,item['filename'],(cache/item['filename']).read_bytes())
  put(z,'OpenKJ-v9-Retained-Plugin-Source-Manifest.json',encoded);put(z,'Cargo.lock',lock)
  put(z,'README.txt',b'Additional retained-plugin sources. Preserve the companion Cerbero recipes/patches and use the exact lockfile/Git commits. Original registry archives and Git snapshots are included; an offline cargo vendor tree is not pre-generated. This archive supplements the main GStreamer and transitive-library sources. No build or legal certification is asserted.\n')
 with zipfile.ZipFile(a.output/'OpenKJ-v9-Retained-Plugin-Notices.zip','w',zipfile.ZIP_DEFLATED) as z:
  for name,raw in sorted(documents.items()):put(z,name,raw)
  put(z,EVIDENCE,encoded)
 print('Preserved',len(registry),'registry sources,',len(pins),'Git snapshots,',len(EXTRA),'static/header sources and',len(documents),'original notice/header documents.')
if __name__=='__main__':main()
