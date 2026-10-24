#!/usr/bin/env python3
"""Preserve the identified Mesa/LLVM upstream sources and original legal texts."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import tarfile
import zipfile
from MIRROR_TRANSITIVE_SOURCES_V9 import download

SOURCES=(
    ('mesa','12.0.0-rc2','https://archive.mesa3d.org/older-versions/12.x/12.0.0/mesa-12.0.0-rc2.tar.xz',
     '4fea2e873e8baf92cefa58695d4e719bcddb279c601077f0e7fafaed1259fbc0'),
    ('llvm','3.6.2','https://releases.llvm.org/3.6.2/llvm-3.6.2.src.tar.xz',
     'f60dc158bfda6822de167e87275848969f0558b3134892ff54fced87e4667b94'),
)


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output',type=Path,required=True);ap.add_argument('--cache',type=Path)
    a=ap.parse_args();a.output.mkdir(parents=True,exist_ok=True)
    cache=a.cache or a.output/'inputs';cache.mkdir(parents=True,exist_ok=True)
    entries=[];inputs=[]
    for name,version,url,pin in SOURCES:
        path=cache/url.rsplit('/',1)[1];download([url],path,pin)
        inputs.append(path)
        with tarfile.open(path) as t:
            for m in t:
                if not m.isfile():continue
                legal=(name=='mesa' and m.name.endswith('/docs/license.html')) or (
                    name=='llvm' and PurePosixPath(m.name).name.lower()=='license.txt')
                if not legal:continue
                if m.size>2*1024*1024:raise RuntimeError('Oversized legal text')
                data=t.extractfile(m).read();rel='/'.join(PurePosixPath(m.name).parts[1:])
                target=a.output/'notices'/f'{name}-{version}'/rel;target.parent.mkdir(parents=True,exist_ok=True)
                target.write_bytes(data)
                entries.append({'package':name,'version':version,'source_archive_sha256':pin,
                    'source_member':m.name,'notice_path':target.relative_to(a.output).as_posix(),
                    'notice_sha256':hashlib.sha256(data).hexdigest()})
    meta={'purpose':'Sources for identified Qt software renderer; upstream build modifications not proven',
          'binary_sha256':'963641a718f9cae2705d5299eae9b7444e84e72ab3bef96a691510dd05fa1da4',
          'observed_versions':['Mesa 12.0.0-rc2','LLVM 3.6.2'],
          'sources':[{'name':n,'version':v,'url':u,'sha256':s} for n,v,u,s in SOURCES],
          'notices':entries,'exact_binary_build_match_proven':False,'release_approved':False}
    encoded=(json.dumps(meta,indent=2,sort_keys=True)+'\n').encode()
    manifest='OpenKJ-v9-Software-OpenGL-Source-Manifest.json'
    (a.output/manifest).write_bytes(encoded)
    with zipfile.ZipFile(a.output/'OpenKJ-v9-Software-OpenGL-Sources.zip','w') as z:
        def add(name,data):
            info=zipfile.ZipInfo(name,(1980,1,1,0,0,0));info.external_attr=0o100644<<16;z.writestr(info,data)
        add(manifest,encoded)
        for p in inputs:add('upstream/'+p.name,p.read_bytes())
        for p in sorted((a.output/'notices').rglob('*')):
            if p.is_file():add(p.relative_to(a.output).as_posix(),p.read_bytes())
    print('Verified Mesa and LLVM sources; preserved original notice files:',len(entries))


if __name__=='__main__':main()
