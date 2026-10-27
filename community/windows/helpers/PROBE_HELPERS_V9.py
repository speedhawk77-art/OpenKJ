#!/usr/bin/env python3
"""Exercise installed helpers offline with synthetic local media, never a live YouTube video."""
import argparse,json,shutil,subprocess,tempfile,zipfile
from pathlib import Path
from VERIFY_HELPERS_V9 import verify

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    for n in ['root','manifest','fixture','report']:ap.add_argument('--'+n,type=Path,required=True)
    a=ap.parse_args();manifest=json.loads(a.manifest.read_text());verify(a.root,manifest);tools=a.root.resolve()/'helpers/youtube';checks=[]
    def check(name,result):
        if not result:raise RuntimeError('Helper probe failed: '+name)
        checks.append({'name':name,'passed':True});print('PASS',name,flush=True)
    def run(args):return subprocess.run([str(x) for x in args],capture_output=True,text=True,encoding='utf-8',errors='replace',check=True,timeout=60).stdout
    check('bundled_downloader_version',run([tools/'yt-dlp.exe','--ignore-config','--no-plugin-dirs','--version']).strip()==manifest['versions']['yt-dlp'])
    check('bundled_deno_version',run([tools/'deno.exe','--version']).startswith('deno '+manifest['versions']['deno']))
    check('bundled_deno_javascript',run([tools/'deno.exe','eval','--no-config',"console.log('OPENKJ_JS_OK')"]).strip()=='OPENKJ_JS_OK')
    version=run([tools/'ffmpeg.exe','-version']);check('bundled_ffmpeg_version',version.startswith('ffmpeg version '+manifest['versions']['ffmpeg']))
    check('bundled_ffprobe_version',run([tools/'ffprobe.exe','-version']).startswith('ffprobe version '+manifest['versions']['ffmpeg']))
    check('ffmpeg_has_no_gpl_or_nonfree', '--enable-gpl' not in version and '--enable-nonfree' not in version)
    protocols=run([tools/'ffmpeg.exe','-hide_banner','-protocols']);check('merger_has_no_network_protocols',not any(x in protocols.split() for x in ['http','https','tcp','tls','udp']))
    with tempfile.TemporaryDirectory(prefix='OpenKJ helper probe with spaces ') as d:
        d=Path(d);video=d/'video with spaces.mp4';audio=d/'audio with spaces.m4a';merged=d/'merged with spaces.mp4'
        for target,mapping in [(video,'0:v:0'),(audio,'0:a:0')]:run([tools/'ffmpeg.exe','-y','-i',a.fixture.resolve(),'-map',mapping,'-c','copy',target])
        run([tools/'ffmpeg.exe','-y','-i',video,'-i',audio,'-map','0:v:0','-map','1:a:0','-c','copy','-movflags','+faststart',merged])
        probe=json.loads(run([tools/'ffprobe.exe','-v','error','-show_streams','-show_format','-of','json',merged]));streams=probe['streams']
        check('merger_preserves_h264_and_aac',len(streams)==2 and {s['codec_name'] for s in streams}=={'h264','aac'})
        check('merged_media_has_duration',float(probe['format']['duration'])>0)
        # Replace only the temporary zip-app fixture to test the real launcher's
        # Windows argument quoting and pipe forwarding without any network access.
        shutil.copytree(tools/'python',d/'python');shutil.copy2(tools/'yt-dlp.exe',d/'yt-dlp.exe')
        with zipfile.ZipFile(d/'yt-dlp.pyz','w') as z:z.writestr('__main__.py','import json,sys\nprint(json.dumps(sys.argv[1:]))\n')
        args=['two words','quote"inside','trailing\\','ampersand & percent %','$(nothing) `literal`','雪']
        check('launcher_preserves_arguments_in_spaced_path',json.loads(run([d/'yt-dlp.exe',*args]))==args)
    check('bundled_ejs_imports_from_zipapp',run([tools/'python/python.exe','-I','-c',"import sys;sys.path.insert(0,sys.argv[1]);import yt_dlp_ejs;from yt_dlp_ejs._version import __version__;print(__version__)",tools/'yt-dlp.pyz']).strip()=='0.8.0')
    a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps({'passed':True,'checks':checks,'helper_source_commit':manifest['source_commit'],'live_youtube_tested':False,'physical_playback_tested':False},indent=2)+'\n')
if __name__=='__main__':main()
