#!/usr/bin/env python3
"""Preserve original notices and declared attribution files from Qt 5.15.2."""
import argparse,fnmatch,hashlib,json,posixpath,re,tarfile,zipfile
from pathlib import Path,PurePosixPath
from MIRROR_TRANSITIVE_SOURCES_V9 import download
from COLLECT_UPSTREAM_NOTICES_V9 import legal_name
PIN='3a530d1b243b5dec00bc54937455471aaa3e56849d2593edb8ded07228202240'
EVIDENCE='OpenKJ-v9-Upstream-Notice-Evidence-v2.json'
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--output',type=Path,required=True);a=ap.parse_args();a.output.mkdir(parents=True,exist_ok=True)
 archive=a.output/'qt-everywhere-src-5.15.2.tar.xz';download(['https://download.qt.io/archive/qt/5.15/5.15.2/single/qt-everywhere-src-5.15.2.tar.xz'],archive,PIN)
 modules={'qtbase','qtsvg','qtimageformats'};documents={};records=[];unresolved=[];attributions=[]
 raw_docs={};members=set();patterns=[]
 # XZ is sequential: retain legal texts in a streaming pass rather than
 # repeatedly seeking/decompressing a multi-gigabyte source tarball.
 with tarfile.open(archive,'r|xz') as t:
  for m in t:
   if not m.isfile():continue
   name=m.name;members.add(name);parts=PurePosixPath(name).parts
   if len(parts)<3 or parts[1] not in modules:continue
   if legal_name(name) or parts[-1]=='qt_attribution.json':
    if m.size>2*1024*1024:raise RuntimeError('Unexpected legal file size')
    raw=t.extractfile(m).read();raw_docs[name]=raw
    if parts[-1]=='qt_attribution.json':
     attributions.append(name)
     for target in re.findall(rb'"LicenseFile"\s*:\s*"([^"\r\n]+)"',raw):
      target=target.decode();pattern=posixpath.normpath(posixpath.join(posixpath.dirname(name),target))
      patterns.append((name,target,pattern))
 needed=set()
 for name,target,pattern in patterns:
  found=[n for n in members if fnmatch.fnmatchcase(n,pattern)]
  if not found:unresolved.append({'attribution':name,'license_file':target})
  needed.update(found)
 needed-=raw_docs.keys()
 if needed:
  with tarfile.open(archive,'r|xz') as t:
   for m in t:
    if m.isfile() and m.name in needed:
     if m.size>2*1024*1024:raise RuntimeError('Unexpected legal reference size')
     raw_docs[m.name]=t.extractfile(m).read()
 for name,raw in sorted(raw_docs.items()):
  if b'\0' in raw:raise RuntimeError('Binary legal reference')
  p=PurePosixPath(name)
  if '..' in p.parts or p.is_absolute() or '\\' in name:raise RuntimeError('Unsafe legal path')
  target='qt-5.15.2/'+ '/'.join(p.parts[1:])
  if p.suffix.lower() in {'.cpp','.c','.h','.hpp','.py','.ps1','.bat','.cmd','.exe','.dll'}:target+='.legal-text.txt'
  documents[target]=raw;records.append({'installed_supplement_path':target,'sha256':hashlib.sha256(raw).hexdigest(),'source_archive':archive.name,'source_archive_sha256':PIN,'source_member':name,'bytes':len(raw)})
 if len({p.casefold() for p in documents})!=len(documents):raise RuntimeError('Windows legal path collision')
 data={'purpose':'Original Qt module legal texts and declared attribution references','source_package_count':1,'modules':sorted(modules),
  'document_count':len(documents),'documents':records,'attribution_metadata_files':attributions,'unresolved_license_file_references':unresolved,
  'exact_shipped_module_build_configuration_proven':False,'legal_completeness_verified':False,'release_approved':False}
 encoded=(json.dumps(data,indent=2,sort_keys=True)+'\n').encode();(a.output/'OpenKJ-v9-Qt-Notice-Evidence.json').write_bytes(encoded)
 with zipfile.ZipFile(a.output/'OpenKJ-v9-Qt-Notices.zip','w',zipfile.ZIP_DEFLATED) as z:
  for name,raw in sorted({**documents,EVIDENCE:encoded}.items()):
   i=zipfile.ZipInfo(name,(1980,1,1,0,0,0));i.compress_type=zipfile.ZIP_DEFLATED;i.external_attr=0o100644<<16;z.writestr(i,raw)
 print('Qt original legal documents:',len(documents),'attribution records:',len(attributions),'unresolved file references:',unresolved)
 if unresolved:raise RuntimeError('Unresolved Qt attribution references require review')
if __name__=='__main__':main()
