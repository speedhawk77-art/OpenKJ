#!/usr/bin/env python3
"""Compare packaged bytes with official checksum-pinned SDK installations."""
import argparse,hashlib,json
from pathlib import Path
from STAGE_OPENKJ_COMPLIANCE_CANDIDATE_V9 import bins

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 ap=argparse.ArgumentParser()
 for n in ('original','gst-root','qt-root','report'):ap.add_argument('--'+n,type=Path,required=True)
 a=ap.parse_args();rows=[];failed=[]
 supplier_by_name={}
 for supplier in (a.gst_root,a.qt_root):
  for file in supplier.rglob('*'):
   if file.is_file() and file.suffix.lower() in ('.dll','.exe'):
    supplier_by_name.setdefault(file.name.casefold(),[]).append(file)
 for name,pin in sorted(bins(a.original).items()):
  path=Path(name)
  if name in ('openkj.exe','OpenKJ-Remote.exe'):
   rows.append({'path':name,'category':'historical application; replaced in source-built candidate','sha256':pin});continue
  if name=='d3dcompiler_47.dll':
   rows.append({'path':name,'category':'separate Microsoft SDK Redist byte match and valid signature supplied earlier','sha256':pin});continue
  if name=='opengl32sw.dll':
   rows.append({'path':name,'category':'separately matched exact Qt-published Mesa RC2 reference','sha256':pin});continue
  candidates=[a.gst_root/'bin'/path,a.qt_root/'bin'/path,a.qt_root/'plugins'/path]
  if name.startswith('gst-plugins/'):candidates.insert(0,a.gst_root/'lib/gstreamer-1.0'/path.name)
  if name.startswith('gio-modules/'):candidates.insert(0,a.gst_root/'lib/gio/modules'/path.name)
  candidates+=supplier_by_name.get(path.name.casefold(),[])
  candidates=list(dict.fromkeys(candidates))
  matches=[str(p.relative_to(a.gst_root) if p.is_relative_to(a.gst_root) else p.relative_to(a.qt_root)) for p in candidates if p.is_file() and sha(p)==pin]
  row={'path':name,'sha256':pin,'matched':bool(matches),'matching_supplier_paths':matches}
  rows.append(row)
  if not matches:failed.append(name)
 data={'check':'Direct byte comparison with official Qt 5.15.2 and GStreamer 1.26.9 SDKs',
  'gstreamer_runtime_msi_sha256':'f06f729f965deaff5d499a7feb596a001c25badbc8e9d5a791d0f23a530d66ab',
  'runtime_files_scanned':len(rows),'direct_supplier_matches':sum(bool(r.get('matched')) for r in rows),
  'separately_accounted_files':sum('category' in r for r in rows),'unmatched':failed,'files':rows,
  'passed':not failed,'source_build_reproduction_proven':False,'release_approved':False}
 a.report.write_text(json.dumps(data,indent=2)+'\n')
 print('Official supplier byte matches:',data['direct_supplier_matches'],'separately accounted:',data['separately_accounted_files'],'unmatched:',failed)
 if failed:raise RuntimeError('Supplier mismatches require investigation')
if __name__=='__main__':main()
