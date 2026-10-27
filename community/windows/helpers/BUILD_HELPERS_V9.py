#!/usr/bin/env python3
"""Build an isolated x64 YouTube helper bundle; never modify OpenKJ runtime files."""
import argparse,base64,hashlib,io,json,os,re,shutil,subprocess,tarfile,time,tomllib,urllib.request,zipfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path,PurePosixPath

HERE=Path(__file__).resolve().parent
LEGAL=re.compile(r'^(licen[cs]e[^/]*|copying[^/]*|notice[^/]*|copyright[^/]*)$',re.I)
BINS={'.exe','.dll','.pyd','.ocx'}

def sha(data):return hashlib.sha256(data).hexdigest()
def digest(path):return sha(path.read_bytes())
def download(url,path,pin,encoded=False):
    if path.exists() and digest(path)==pin:return path
    path.parent.mkdir(parents=True,exist_ok=True)
    for attempt in range(4):
        try:
            req=urllib.request.Request(url,headers={'User-Agent':'OpenKJ-YouTube-Helper-Build'})
            with urllib.request.urlopen(req,timeout=90) as response:raw=response.read()
            if encoded:raw=base64.b64decode(raw)
            if sha(raw)!=pin:raise RuntimeError('Checksum mismatch: '+path.name)
            path.write_bytes(raw);return path
        except Exception:
            if attempt==3:raise
            time.sleep(2)

def safe_name(name):
    p=PurePosixPath(name)
    if p.is_absolute() or '..' in p.parts or '\\' in name or ':' in name:raise RuntimeError('Unsafe archive member')
    return p

def unzip(archive,out):
    with zipfile.ZipFile(archive) as z:
        for entry in z.infolist():
            rel=safe_name(entry.filename)
            if entry.is_dir():continue
            if entry.file_size>300*1024*1024:raise RuntimeError('Unexpectedly large helper member')
            target=out/str(rel);target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(z.read(entry))

def notice_member(name,raw):
    return (LEGAL.match(PurePosixPath(name).name) and len(raw)<4*1024*1024 and b'\0' not in raw
            and PurePosixPath(name).suffix.lower() not in BINS|{'.rs','.c','.h','.cpp','.cc','.py','.bat','.ps1','.cmd','.sh','.js','.ts'})

def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--work',type=Path,required=True);ap.add_argument('--cache',type=Path,required=True);ap.add_argument('--prepare-only',action='store_true');a=ap.parse_args()
    work=a.work.resolve();cache=a.cache.resolve();work.mkdir(parents=True,exist_ok=True);cache.mkdir(parents=True,exist_ok=True)
    payload=work/'payload';tools=payload/'helpers/youtube';notices=payload/'licenses/youtube-helpers';sources=work/'sources'
    if payload.exists():raise RuntimeError('Refusing to reuse helper output')
    tools.mkdir(parents=True);notices.mkdir(parents=True);sources.mkdir(parents=True)
    lock=json.loads((HERE/'inputs.json').read_text());notice_lock=json.loads((HERE/'notices.json').read_text())
    def fetch(item):return download(item['url'],cache/item['name'],item['sha256'])
    with ThreadPoolExecutor(max_workers=6) as pool:list(pool.map(fetch,lock['downloads']))
    for item in lock['downloads']:
        if item['name'] not in ['deno-windows.zip','python-3.14.9-embed-amd64.zip','yt-dlp_win.zip','yt-dlp.pyz']:
            shutil.copy2(cache/item['name'],sources/item['name'])
    unzip(cache/'python-3.14.9-embed-amd64.zip',tools/'python')
    unzip(cache/'deno-windows.zip',tools)
    if set(p.name for p in tools.glob('*.exe'))!={'deno.exe'}:raise RuntimeError('Unexpected Deno archive inventory')
    shutil.copy2(cache/'yt-dlp.pyz',tools/'yt-dlp.pyz')
    shutil.copy2(tools/'python/LICENSE.txt',notices/'CPython-LICENSE.txt')
    with zipfile.ZipFile(cache/'yt-dlp_win.zip') as z:
        (notices/'Upstream-yt-dlp-Third-Party-Notices.txt').write_bytes(z.read('_internal/THIRD_PARTY_LICENSES.txt'))
    # This superset is labelled explicitly: the official zip app does not bundle
    # the curl-cffi, mutagen or PyInstaller binaries from the Windows one-file build.
    (notices/'BUNDLE-README.txt').write_text('OpenKJ bundles the unmodified official yt-dlp zip application, unmodified CPython embeddable runtime, unmodified Deno executable and a source-built LGPL FFmpeg stream-copy merger. yt-dlp.exe is OpenKJ\'s small source-available launcher. The upstream yt-dlp notice file is retained as a superset; curl-cffi, mutagen and PyInstaller are not bundled. Exact input hashes, sources, build configuration and installed file hashes are recorded in the accompanying manifests. Manual helper paths remain supported.\n',encoding='utf-8')
    docs=[]
    def add_notice(group,member,raw):
        # Flatten names to keep Windows paths short; retain original source paths in the manifest.
        filename=sha(raw)[:16]+'-'+re.sub(r'[^a-zA-Z0-9_.-]','_',PurePosixPath(member).name)[:45]+'.txt'
        target=notices/group/filename;target.parent.mkdir(parents=True,exist_ok=True)
        if not target.exists():target.write_bytes(raw)
        docs.append({'file':target.relative_to(payload).as_posix(),'sha256':sha(raw),'source_member':member})
    for item in lock['downloads']:
        if not item['name'].endswith(('.tar.gz','.tar.xz','.tgz')):continue
        with tarfile.open(cache/item['name']) as t:
            for m in t:
                if not m.isfile() or m.size>4*1024*1024:continue
                if LEGAL.match(PurePosixPath(m.name).name):
                    raw=t.extractfile(m).read()
                    if notice_member(m.name,raw):add_notice(item['name'].split('.')[0],m.name,raw)
    # Deno's Cargo.lock pins all registry inputs. Preserve original notices and
    # source archives without executing any crate/build script. This is a superset
    # including target-specific/build dependencies, not a claim they all link into Deno.
    with tarfile.open(cache/'deno_src.tar.gz') as t:raw_lock=t.extractfile('deno/Cargo.lock').read()
    (sources/'deno-Cargo.lock').write_bytes(raw_lock)
    packages=tomllib.loads(raw_lock.decode())['package'];registry=[]
    for p in packages:
        if 'source' not in p:continue
        if p['source']!='registry+https://github.com/rust-lang/crates.io-index':raise RuntimeError('Unreviewed Git dependency')
        if not re.fullmatch(r'[\w.+-]+',p['name']) or not re.fullmatch(r'[\w.+-]+',p['version']) or not re.fullmatch('[0-9a-f]{64}',p.get('checksum','')):raise RuntimeError('Invalid crate lock')
        registry.append(p)
    def crate(p):
        name=p['name']+'-'+p['version']+'.crate';return download('https://static.crates.io/crates/'+p['name']+'/'+name,cache/'crates'/name,p['checksum'])
    with ThreadPoolExecutor(max_workers=12) as pool:archives=list(pool.map(crate,registry))
    for package,archive in zip(registry,archives):
        group='deno-crates/'+(package['name']+'-'+package['version'])[:65];found=False
        shutil.copy2(archive,sources/archive.name)
        with tarfile.open(archive) as t:
            for m in t:
                if not m.isfile() or m.size>4*1024*1024:continue
                if not LEGAL.match(PurePosixPath(m.name).name):continue
                raw=t.extractfile(m).read()
                if notice_member(m.name,raw):add_notice(group,m.name,raw);found=True
            if not found:
                # Some crates omit licence files from their published archive.
                # Preserve original metadata and leading source attribution blocks.
                for m in t.getmembers():
                    if not m.isfile() or m.size>4*1024*1024:continue
                    if PurePosixPath(m.name).name in ['Cargo.toml','Cargo.toml.orig']:
                        add_notice(group,m.name,t.extractfile(m).read());continue
                    if PurePosixPath(m.name).suffix not in ['.rs','.c','.h','.cc','.cpp']:continue
                    raw=t.extractfile(m).read();prefix=[];block=False
                    for line in raw.splitlines(keepends=True):
                        text=line.lstrip()
                        if block:prefix.append(line);block=b'*/' not in line
                        elif text.startswith(b'/*'):prefix.append(line);block=b'*/' not in line
                        elif text.startswith(b'//') or not text.strip():prefix.append(line)
                        else:break
                    prefix=b''.join(prefix)
                    if re.search(rb'copyright|licen[cs]e|SPDX',prefix,re.I):add_notice(group,m.name+'-header',prefix)
    def fetch_notice(item):
        target=cache/'notices'/sha(item['name'].encode());download(item['url'],target,item['sha256'],item.get('base64',False));return item,target
    with ThreadPoolExecutor(max_workers=8) as pool:
        for item,path in pool.map(fetch_notice,notice_lock['notices']):add_notice('deno-v8-rust',item['name'],path.read_bytes())
    print('Preserved Deno source/notice inputs:',len(registry),'registry crates;',len(docs),'notice references',flush=True)
    if a.prepare_only:return
    # Compile the launcher with the same MSVC toolset selected by the build workflow.
    subprocess.run(['cl.exe','/nologo','/std:c++17','/O2','/MT','/EHsc','/Fe:'+str(tools/'yt-dlp.exe'),str(HERE/'YouTubeLauncher.cpp'),'/link','shell32.lib'],check=True,cwd=work)
    ffsource=work/'ffmpeg-src';ffsource.mkdir()
    with tarfile.open(cache/'ffmpeg-8.1.3.tar.xz') as t:
        for m in t:
            if not m.isfile():continue
            rel=safe_name(m.name);target=ffsource/str(PurePosixPath(*rel.parts[1:]));target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(t.extractfile(m).read())
    config=['--toolchain=msvc','--arch=x86_64','--disable-everything','--disable-autodetect','--disable-network','--disable-doc','--disable-debug','--disable-shared','--enable-static','--disable-x86asm','--disable-avdevice','--disable-swresample','--disable-swscale','--enable-ffmpeg','--enable-ffprobe','--enable-protocol=file,pipe','--enable-demuxer=mov','--enable-muxer=mp4,mov,ipod','--enable-parser=h264,aac','--enable-bsf=aac_adtstoasc,extract_extradata,h264_mp4toannexb','--enable-filter=null,anull','--extra-cflags=-MT']
    # Native Windows make plus Git's sh, with MSVC link.exe ahead of coreutils link.
    env=dict(os.environ);vc=Path(os.environ['VCToolsInstallDir'])/'bin/Hostx64/x64';env['PATH']=str(vc)+os.pathsep+env['PATH']
    subprocess.run([str(Path(os.environ['ProgramFiles'])/'Git/bin/bash.exe'),'configure',*config],cwd=ffsource,env=env,check=True)
    subprocess.run(['make','-j4'],cwd=ffsource,env=env,check=True)
    for name in ['ffmpeg.exe','ffprobe.exe']:shutil.copy2(ffsource/name,tools/name)
    cfg=(ffsource/'ffbuild/config.mak').read_text()
    if re.search(r'^CONFIG_(GPL|NONFREE|VERSION3)=yes$',cfg,re.M):raise RuntimeError('Unapproved FFmpeg licence configuration')
    for name in ['COPYING.LGPLv2.1','COPYING.LGPLv3','LICENSE.md']:
        if (ffsource/name).is_file():shutil.copy2(ffsource/name,notices/('FFmpeg-'+name+'.txt'))
    (sources/'ffmpeg-configure-arguments.json').write_text(json.dumps(config,indent=2)+'\n')
    shutil.copy2(ffsource/'ffbuild/config.mak',sources/'ffmpeg-config.mak.txt');shutil.copy2(HERE/'YouTubeLauncher.cpp',sources/'YouTubeLauncher.cpp')
    shutil.copy2(HERE/'BUILD_HELPERS_V9.py',sources/'BUILD_HELPERS_V9.py')
    for name in ['inputs.json','notices.json']:shutil.copy2(HERE/name,sources/name)
    (notices/'SOURCE-AND-BUILD.txt').write_text('Matching source package: OpenKJ-v9-YouTube-Helper-Sources-'+os.environ['GITHUB_SHA']+'.zip\nAvailable beside the installer in the same GitHub release. Source includes unmodified FFmpeg, exact configure options/config.mak, yt-dlp and EJS sources, CPython, Deno/Cargo source archives and the launcher/build scripts. The FFmpeg executable is statically linked LGPL-2.1-or-later code without GPL/nonfree parts; the source and configuration support rebuilding/relinking it. OpenKJ calls it as a separate process. Deno is MIT with third-party notices retained. No helper replaces a playback library.\n',encoding='utf-8')
    manifest={'source_commit':os.environ['GITHUB_SHA'],'versions':lock['versions'],'inputs':lock,'notice_inputs':notice_lock,'notice_references':docs,'deno_registry_source_packages':len(registry),'ffmpeg_configure':config,'ffmpeg_gpl':False,'ffmpeg_nonfree':False,'cisco_binary_bundled':False,'files':{p.relative_to(payload).as_posix():digest(p) for p in payload.rglob('*') if p.is_file()}}
    manifest['runtime_files']={n:h for n,h in manifest['files'].items() if Path(n).suffix.lower() in BINS}
    (work/'helper-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf-8')
    with zipfile.ZipFile(work/'helper-bundle.zip','w',zipfile.ZIP_DEFLATED) as z:
        for p in payload.rglob('*'):
            if p.is_file():z.write(p,p.relative_to(payload).as_posix())
        z.write(work/'helper-manifest.json','helper-manifest.json')
    with zipfile.ZipFile(work/'helper-sources.zip','w',zipfile.ZIP_STORED) as z:
        for p in sorted(sources.iterdir()):
            info=zipfile.ZipInfo(p.name,(1980,1,1,0,0,0));info.compress_type=zipfile.ZIP_STORED;z.writestr(info,p.read_bytes())
    print('Helper bundle complete:',len(manifest['runtime_files']),'program files;',len(manifest['files']),'total files',flush=True)
if __name__=='__main__':main()
