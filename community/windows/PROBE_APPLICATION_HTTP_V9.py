#!/usr/bin/env python3
"""Launch the app on an isolated CI profile and probe localhost guest/auth routes."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import time
import urllib.error
import urllib.request


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--app', type=Path, required=True)
    ap.add_argument('--report', type=Path, required=True)
    args = ap.parse_args()
    app = args.app.resolve()
    work = args.report.parent.resolve() / 'isolated-app-profile'
    work.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, LOCALAPPDATA=str(work / 'local'), APPDATA=str(work / 'roaming'),
               GST_REGISTRY=str(work / 'gst-registry.bin'))
    for name in ('local','roaming'): (work / name).mkdir(exist_ok=True)
    def request(path, method='GET', data=None):
        req=urllib.request.Request('http://127.0.0.1:8081'+path,data=data,method=method,
                                   headers={'Content-Type':'application/json'})
        try:
            with urllib.request.urlopen(req,timeout=2) as r: return r.status,r.read()
        except urllib.error.HTTPError as e: return e.code,e.read()
    checks=[]
    # Never archive raw app output or profile files; preserve test statuses only.
    with (work/'app-output.log').open('wb') as log:
        process=subprocess.Popen([str(app)],cwd=app.parent,env=env,stdout=log,stderr=log)
        try:
            ready=False
            for _ in range(30):
                if process.poll() is not None: break
                try:
                    code,body=request('/')
                    if code==200 and b'<html' in body.lower(): ready=True;break
                except (OSError,urllib.error.URLError): pass
                time.sleep(1)
            if ready:
                for path,method,data,expected in (
                    ('/','GET',None,200),('/request','GET',None,200),
                    ('/qr','GET',None,200),('/qr-library.js','GET',None,200),
                    ('/status','GET',None,403),('/songs?q=test','GET',None,403),
                    ('/public/songs?q=a','GET',None,400),
                    ('/public/songs?q=test','GET',None,200),
                    ('/public/request','GET',None,405),
                    ('/public/request','POST',b'not-json',400),
                    ('/nonexistent-audit-route','GET',None,404),
                    ('/remote','POST',b'{"cmd":"youtube_download","url":"https://youtu.be/00000000001"}',403),
                    ('/remote','POST',b'{"cmd":"youtube_cancel"}',403),
                    ('/remote','POST',b'{"cmd":"youtube_import"}',403),
                ):
                    code,body=request(path,method,data)
                    checks.append({'path':path,'method':method,'expected':expected,
                                   'actual':code,'passed':code==expected})
            result={'check':'Isolated app startup and localhost guest/auth HTTP routes',
                    'http_ready':ready,'process_exit_code_at_check':process.poll(),
                    'checks':checks,'passed':ready and all(x['passed'] for x in checks),
                    'physical_audio_video_tested':False,'real_phone_tested':False,
                    'authenticated_DJ_actions_tested':False,'release_approved':False}
            args.report.parent.mkdir(parents=True,exist_ok=True)
            args.report.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
            print('Isolated app HTTP probe:', 'PASS' if result['passed'] else 'FAIL/NOT READY',
                  f"({sum(x['passed'] for x in checks)}/{len(checks)} route checks)")
        finally:
            if os.name=='nt':
                subprocess.run(['taskkill','/PID',str(process.pid),'/T','/F'],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
            else:process.terminate()
            try:process.wait(timeout=10)
            except subprocess.TimeoutExpired:process.kill();process.wait(timeout=5)


if __name__=='__main__':main()
