#!/usr/bin/env python3
"""Verify/stage only the separately attested YouTube helper files."""
import argparse,hashlib,json,os,struct,zipfile
from pathlib import Path,PurePosixPath
BINS={'.exe','.dll','.pyd','.ocx'}
PREFIXES=('helpers/youtube/','licenses/youtube-helpers/')
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def validate_manifest(m):
    files=m['files']
    if len({n.casefold() for n in files})!=len(files):raise RuntimeError('Case-colliding helper paths')
    for n,h in files.items():
        p=PurePosixPath(n)
        if not n.startswith(PREFIXES) or p.is_absolute() or '..' in p.parts or '\\' in n or ':' in n or len(h)!=64 or any(c not in '0123456789abcdef' for c in h):raise RuntimeError('Unsafe helper manifest')
    runtime={n:h for n,h in files.items() if Path(n).suffix.lower() in BINS}
    if runtime!=m['runtime_files'] or not runtime:raise RuntimeError('Helper binary inventory differs')
    required={'helpers/youtube/'+n for n in ['yt-dlp.exe','yt-dlp.pyz','deno.exe','ffmpeg.exe','ffprobe.exe','python/python.exe','python/python314.zip']}
    if not required<=set(files):raise RuntimeError('Missing helper component')
    if m['ffmpeg_gpl'] is not False or m['ffmpeg_nonfree'] is not False or m['cisco_binary_bundled'] is not False:raise RuntimeError('Unapproved helper licence flags')
    return runtime

def verify(root,manifest):
    root=Path(root);runtime=validate_manifest(manifest)
    actual={}
    for prefix in PREFIXES:
        for p in (root/prefix).rglob('*'):
            if p.is_symlink():raise RuntimeError('Helper symlink not allowed')
            if p.is_file():actual[p.relative_to(root).as_posix()]=digest(p)
    if actual!=manifest['files']:raise RuntimeError('Helper file inventory/hash mismatch')
    for n in runtime:
        raw=(root/n).read_bytes()
        if raw[:2]!=b'MZ':raise RuntimeError('Helper is not a PE binary: '+n)
        offset=struct.unpack_from('<I',raw,0x3c)[0]
        if raw[offset:offset+4]!=b'PE\0\0' or struct.unpack_from('<H',raw,offset+4)[0]!=0x8664:raise RuntimeError('Helper is not Windows x64: '+n)
    return runtime

def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--candidate',type=Path,required=True);ap.add_argument('--bundle',type=Path);ap.add_argument('--manifest',type=Path);ap.add_argument('--report',type=Path);a=ap.parse_args()
    if a.bundle:
        with zipfile.ZipFile(a.bundle) as z:
            m=json.loads(z.read('helper-manifest.json'));validate_manifest(m)
            if m['source_commit']!=os.environ['GITHUB_SHA']:raise RuntimeError('Helper source commit differs')
            for name,expected in [('inputs', 'inputs.json'),('notice_inputs','notices.json')]:
                if m[name]!=json.loads(Path(__file__).with_name(expected).read_text()):raise RuntimeError('Helper input pins differ from current source')
            if set(z.namelist())!=set(m['files'])|{'helper-manifest.json'}:raise RuntimeError('Unattested bundle entries')
            if len(z.namelist())!=len(set(z.namelist())):raise RuntimeError('Duplicate archive entries')
            for n,pin in m['files'].items():
                target=a.candidate/n
                if target.exists():raise RuntimeError('Refusing to replace candidate file: '+n)
                raw=z.read(n)
                if hashlib.sha256(raw).hexdigest()!=pin:raise RuntimeError('Bundle checksum mismatch: '+n)
                target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(raw)
    else:m=json.loads(a.manifest.read_text())
    runtime=verify(a.candidate,m)
    if a.report:a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(m,indent=2)+'\n',encoding='utf-8')
    print('YouTube helpers verified:',len(runtime),'Windows x64 programs;',len(m['files']),'file hashes')
if __name__=='__main__':main()
