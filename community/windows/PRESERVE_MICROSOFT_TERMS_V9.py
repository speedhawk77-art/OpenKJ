#!/usr/bin/env python3
"""Validate extracted Burn license-link metadata; preserve the linked original terms.

Does not run the redist or accept terms. Accepts WiX output or raw CAB entries.
"""
import argparse, hashlib, json, urllib.request, zipfile
from pathlib import Path

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--metadata',type=Path,required=True)
    ap.add_argument('--repo',type=Path,default=Path('.'))
    ap.add_argument('--output',type=Path,required=True)
    a=ap.parse_args()
    root=a.repo/'community/licenses/microsoft'
    pins=json.loads((root/'provenance.json').read_text())
    documents={}
    for p in sorted(a.metadata.rglob('*')):
        if not p.is_file() or p.stat().st_size>2*1024*1024:continue
        raw=p.read_bytes()
        texts=[]
        for encoding in ('utf-8-sig','utf-16'):
            try:texts.append(raw.decode(encoding))
            except UnicodeError:pass
        if any(pins['embedded_license_url'] in text for text in texts):
            documents['embedded-metadata/'+p.relative_to(a.metadata).as_posix()]=raw
    if not documents:raise RuntimeError('Expected Microsoft license URL absent from extracted bootstrapper metadata')
    with urllib.request.urlopen(pins['runtime_document_url'],timeout=90) as response:
        raw=response.read()
    if hashlib.sha256(raw).hexdigest()!=pins['runtime_document_sha256']:
        raise RuntimeError('Linked Microsoft terms changed: review original document before changing pin')
    documents['Microsoft-Original-Runtime-Terms.docx']=raw
    for name,key in [('Visual-C-V14-Runtime-Terms.txt','runtime_text_sha256'),('Windows-SDK-LICENSE.txt','sdk_license_sha256')]:
        raw=(root/name).read_bytes()
        if hashlib.sha256(raw).hexdigest()!=pins[key]:raise RuntimeError('Checked-in terms checksum changed')
        documents[name]=raw
    manifest=dict(pins,redist_executed=False,terms_accepted=False,documents=[{'path':name,'bytes':len(raw),'sha256':hashlib.sha256(raw).hexdigest()} for name,raw in documents.items()])
    a.output.mkdir(parents=True,exist_ok=True)
    encoded=(json.dumps(manifest,indent=2)+'\n').encode()
    (a.output/'OpenKJ-v9-Microsoft-Runtime-Legal-Manifest.json').write_bytes(encoded)
    with zipfile.ZipFile(a.output/'OpenKJ-v9-Microsoft-Runtime-Legal-Documents.zip','w',zipfile.ZIP_DEFLATED) as z:
        for name,raw in sorted({**documents,'OpenKJ-v9-Microsoft-Runtime-Legal-Manifest.json':encoded}.items()):
            info=zipfile.ZipInfo(name,(1980,1,1,0,0,0));info.compress_type=zipfile.ZIP_DEFLATED
            info.external_attr=0o100644<<16;z.writestr(info,raw)
    print('Verified linked original terms and',len(documents),'preserved documents; no terms accepted.')
if __name__=='__main__':main()
