#!/usr/bin/env python3
"""Bundle this CI-built executable with its exact tracked application sources.

Draft-only packaging; no codec DLL, SDK binary, credentials or profiles included.
"""
import hashlib,json,os,subprocess,zipfile
from pathlib import Path

def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def writezip(path,items):
    with zipfile.ZipFile(path,'w',zipfile.ZIP_DEFLATED) as z:
        for name,p in items:
            i=zipfile.ZipInfo(name,(1980,1,1,0,0,0));i.compress_type=zipfile.ZIP_DEFLATED;i.external_attr=0o100644<<16
            z.writestr(i,p.read_bytes())
def main():
    evidence=Path('source-build-evidence.json');data=json.loads(evidence.read_text())
    assert data['source_commit']==os.environ['GITHUB_SHA']
    app=Path('source-build/openkj.exe');assert digest(app)==data['executable_sha256']
    launcher=Path('source-build/OpenKJ-Remote.exe');assert digest(launcher)==data['launcher_sha256']
    probe=json.loads(Path('source-app-http-probe.json').read_text());assert probe['passed'] and len(probe['checks'])==11
    tests=json.loads(Path('optional-codec-test-evidence.json').read_text());assert tests['passed']
    names=subprocess.check_output(['git','ls-files','--recurse-submodules','-z']).decode().split('\0')
    files=[]
    for name in sorted(filter(None,names)):
        p=Path(name)
        if p.is_symlink() or not p.is_file():raise RuntimeError('Unexpected tracked source item: '+name)
        if name.startswith('.git/') or '/.git/' in name:raise RuntimeError('Unexpected git metadata')
        files.append((name,p))
    source=Path('OpenKJ-v9-Application-Source.zip');writezip(source,files)
    data['application_source_archive_sha256']=digest(source)
    data['application_source_tracked_files']=len(files)
    evidence.write_text(json.dumps(data,indent=2)+'\n')
    bundle=Path('OpenKJ-v9-Source-Built-App-'+os.environ['GITHUB_SHA']+'.zip')
    writezip(bundle,[('openkj.exe',app),('OpenKJ-Remote.exe',launcher),('source-build-evidence.json',evidence),
        ('OpenKJ-v9-Application-Source.zip',source),('optional-codec-test-evidence.json',Path('optional-codec-test-evidence.json'))])
    print('Draft-only source-built bundle:',bundle.name,'SHA256:',digest(bundle))
if __name__=='__main__':main()
