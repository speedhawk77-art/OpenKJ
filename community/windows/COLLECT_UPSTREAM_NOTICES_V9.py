#!/usr/bin/env python3
"""Preserve original legal/attribution files from verified source bundles.

Does not assert legal completeness or binary/source build correspondence.
Never extracts executable code to the candidate or executes archive contents.
"""
import argparse
import hashlib
import io
import json
from pathlib import Path, PurePosixPath
import re
import subprocess
import tarfile
import zipfile
from MIRROR_TRANSITIVE_SOURCES_V9 import download

LEGAL = re.compile(r'^(copying\d*|licen[cs]e\d*|licensing|notice|authors|copyright)([._-].*)?$', re.I)
ZIP_NAME = 'OpenKJ-v9-Upstream-Notices-v2.zip'
JSON_NAME = 'OpenKJ-v9-Upstream-Notice-Evidence-v2.json'


def legal_name(name):
    parts=PurePosixPath(name).parts
    return (bool(LEGAL.match(PurePosixPath(name).name))
            or any(p.upper() in {'LICENSES','LICENCES'} for p in parts[:-1])
            or name.endswith('/.reuse/dep5') or PurePosixPath(name).name=='REUSE.toml')


def sha(data): return hashlib.sha256(data).hexdigest()


def get_draft_bundles(folder):
    folder.mkdir(parents=True, exist_ok=True)
    release=json.loads(subprocess.check_output(['gh','api','repos/speedhawk77-art/OpenKJ/releases/421204266']))
    if not release['draft'] or release['published_at'] is not None:
        raise RuntimeError('Expected unpublished draft')
    result=[]
    for batch in range(1,6):
        name=f'OpenKJ-v9-Transitive-Source-Batch{batch}.zip'
        items=[x for x in release['assets'] if x['name']==name]
        if len(items)!=1:raise RuntimeError('Missing/duplicate source bundle: '+name)
        item=items[0];target=folder/name
        with target.open('wb') as f:
            subprocess.run(['gh','api','-H','Accept: application/octet-stream',
                f"repos/speedhawk77-art/OpenKJ/releases/assets/{item['id']}"],stdout=f,check=True)
        if 'sha256:'+sha(target.read_bytes())!=item['digest']:
            raise RuntimeError('Source bundle digest mismatch: '+name)
        result.append(target)
    return result


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output',type=Path,required=True)
    ap.add_argument('--local-bundles',type=Path,nargs='+')
    args=ap.parse_args();out=args.output.resolve();out.mkdir(parents=True,exist_ok=True)
    bundles=list(args.local_bundles or get_draft_bundles(out/'verified-bundles'))
    # proxy-libintl was mirrored separately before the numbered batches.
    proxy=out/'proxy-libintl-0.4-source.tar.gz'
    proxy_pin='13ef3eea0a3bc0df55293be368dfbcff5a8dd5f4759280f28e030d1494a5dffb'
    download(['https://github.com/frida/proxy-libintl/archive/refs/tags/0.4.tar.gz'],proxy,proxy_pin)
    proxy_bundle=out/'proxy-libintl-notice-input.zip'
    with zipfile.ZipFile(proxy_bundle,'w') as z:
        z.writestr('proxy-Manifest.json',json.dumps({'sources':[{'recipe':'proxy-libintl','version':'0.4',
            'archive':'proxy-source.tar.gz','sha256':proxy_pin}]}))
        z.write(proxy,'proxy-source.tar.gz')
    bundles.append(proxy_bundle)
    docs={};evidence=[];packages=[];skipped=[]
    total=0
    for bundle_path in sorted(bundles):
        with zipfile.ZipFile(bundle_path) as bundle:
            names=[n for n in bundle.namelist() if n.endswith('-Manifest.json')]
            if len(names)!=1:raise RuntimeError('Unexpected source bundle manifest')
            manifest=json.loads(bundle.read(names[0]))
            for source in manifest['sources']:
                package=source['recipe'];raw=bundle.read(source['archive'])
                print('Collecting original legal documents:',package,flush=True)
                if sha(raw)!=source['sha256']:raise RuntimeError('Upstream source digest mismatch')
                count=0
                def collect(name,data,output_relative=None,byte_range=None):
                    nonlocal total,count
                    parts=PurePosixPath(name).parts
                    if not parts or '..' in parts or PurePosixPath(name).is_absolute():
                        raise RuntimeError('Unsafe legal-document path')
                    # Strip just the upstream archive's top directory.
                    relative=output_relative or ('/'.join(parts[1:]) if len(parts)>1 else parts[0])
                    relative='/'.join('_dot_'+p[1:] if p.startswith('.') else p
                                      for p in PurePosixPath(relative).parts)
                    target=package+'/'+relative
                    if len(data)>2*1024*1024 or b'\x00' in data:
                        skipped.append({'package':package,'path':name,'reason':'not a small text document'});return
                    if target in docs:
                        if docs[target]!=data:raise RuntimeError('Conflicting legal-document path: '+target)
                        return
                    total+=len(data)
                    if total>50*1024*1024:raise RuntimeError('Notice text size limit exceeded')
                    docs[target]=data;count+=1
                    evidence.append({'installed_supplement_path':target,'sha256':sha(data),
                        'source_archive':source['archive'],'source_archive_sha256':source['sha256'],
                        'source_member':name,'bytes':len(data)})
                    if byte_range is not None:evidence[-1]['source_byte_range']=byte_range
                if zipfile.is_zipfile(io.BytesIO(raw)):
                    with zipfile.ZipFile(io.BytesIO(raw)) as source_zip:
                        for info in source_zip.infolist():
                            if package=='sqlite3' and info.filename.endswith('/sqlite3.c'):
                                data=source_zip.read(info)
                                start=data.find(b'/*\n** 2001 September 15')
                                end=data.find(b'*/',start)+2 if start>=0 else -1
                                if start<0 or end<=start or b'disclaims copyright' not in data[start:end]:
                                    raise RuntimeError('SQLite source dedication header not found')
                                collect(info.filename,data[start:end],'sqlite3-source-dedication.txt',[start,end])
                            if not info.is_dir() and legal_name(info.filename):
                                if info.file_size>2*1024*1024:raise RuntimeError('Oversized legal document')
                                collect(info.filename,source_zip.read(info))
                else:
                    with tarfile.open(fileobj=io.BytesIO(raw),mode='r:*') as source_tar:
                        for info in source_tar:
                            if not legal_name(info.name):continue
                            if not info.isfile():
                                skipped.append({'package':package,'path':info.name,'reason':'non-regular legal-document alias'});continue
                            if info.size>2*1024*1024:raise RuntimeError('Oversized legal document')
                            collect(info.name,source_tar.extractfile(info).read())
                packages.append({'recipe':package,'version':source['version'],
                                 'source_sha256':source['sha256'],'legal_files_preserved':count})
    if len({n.casefold() for n in docs})!=len(docs):
        raise RuntimeError('Notice paths would collide on Windows')
    report={'purpose':'Original source legal/attribution text preservation',
        'source_package_count':len(packages),'document_count':len(docs),
        'packages':packages,'documents':evidence,'skipped':skipped,
        'legal_completeness_verified':False,'binary_source_correspondence_proven':False,
        'release_approved':False,
        'limits':['Filename-based collection; copyrights embedded only in source headers may be absent',
                  'Toolchain Git-only build dependencies and upstream subdependencies may require more sources',
                  'Actual installed notices must still be compared after packaging']}
    encoded=(json.dumps(report,indent=2,sort_keys=True)+'\n').encode()
    (out/JSON_NAME).write_bytes(encoded)
    with zipfile.ZipFile(out/ZIP_NAME,'w',compression=zipfile.ZIP_DEFLATED) as z:
        def add(name,data):
            entry=zipfile.ZipInfo(name,(1980,1,1,0,0,0));entry.compress_type=zipfile.ZIP_DEFLATED
            entry.external_attr=0o100644<<16;z.writestr(entry,data)
        add(JSON_NAME,encoded)
        for name in sorted(docs):add(name,docs[name])
    print(f'Preserved {len(docs)} original legal/attribution documents from {len(packages)} source archives.')
    print('Packages with no standalone legal text:',[p['recipe'] for p in packages if not p['legal_files_preserved']])
    print('Notice bundle SHA-256:',sha((out/ZIP_NAME).read_bytes()))


if __name__=='__main__':main()
