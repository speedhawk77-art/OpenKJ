#!/usr/bin/env python3
"""Publish only Andrew's approved, checksum-verified clean v9 release."""
import hashlib,json,subprocess,tempfile
from pathlib import Path

REPO='speedhawk77-art/OpenKJ'
RELEASE=423591307
def gh(*args):return subprocess.check_output(['gh',*args],text=True)
def api(path):return json.loads(gh('api',f'repos/{REPO}/'+path))

def main():
    record=json.loads(Path(__file__).with_name('data').joinpath('verified-clean-draft-v9.json').read_text())
    if record['release_id']!=RELEASE or record['tag']!='v9.0-offline-remote.2':
        raise RuntimeError('Unexpected approved release')
    release=api(f'releases/{RELEASE}')
    expected={x['name']:(x['id'],x['bytes'],'sha256:'+x['sha256']) for x in record['verified_assets']}
    actual={x['name']:(x['id'],x['size'],x['digest']) for x in release['assets']}
    if len(expected)!=41 or len(release['assets'])!=41 or actual!=expected or release['tag_name']!=record['tag']:
        raise RuntimeError('Clean release asset inventory changed since owner review')
    if len([x for x in actual if x.endswith('.exe')])!=1:
        raise RuntimeError('Expected one approved installer')
    for run_id in (record['source_mirror_run'],record['microsoft_terms_run'],record['installer_verification_run'],record['clean_draft_preparation_run']):
        run=api(f'actions/runs/{run_id}')
        if run['conclusion']!='success' or run['head_repository']['full_name']!=REPO:
            raise RuntimeError('Required verification run is not successful')
    working=api('releases/421204266')
    if working['draft'] is not True or working['published_at'] is not None:
        raise RuntimeError('Historical working release must remain private')
    checksums=next(x for x in release['assets'] if x['name']=='SHA256SUMS.txt')
    with tempfile.TemporaryDirectory() as folder:
        folder=Path(folder)
        for item in (checksums,next(x for x in release['assets'] if x['name']==record['installer']['name'])):
            path=folder/item['name']
            with path.open('wb') as f:
                subprocess.run(['gh','api',f"repos/{REPO}/releases/assets/{item['id']}",'-H','Accept: application/octet-stream'],stdout=f,check=True)
            h=hashlib.sha256()
            with path.open('rb') as f:
                for block in iter(lambda:f.read(1048576),b''):h.update(block)
            if item['digest']!='sha256:'+h.hexdigest():raise RuntimeError('Approved download checksum mismatch')
        sums=''.join(expected[n][2].removeprefix('sha256:')+'  '+n+'\n' for n in sorted(expected) if n!='SHA256SUMS.txt')
        if (folder/'SHA256SUMS.txt').read_bytes()!=sums.encode():raise RuntimeError('Checksum file inventory differs')
        body=release['body'].replace('This draft is prepared for final owner review. It has not been published.\n','')
        if not body.startswith('**Unofficial community release.**'):
            body='**Unofficial community release.** This is a separate community edition, not an official release from the OpenKJ maintainers.\n\n'+body
        if release['draft']:
            payload=folder/'publish.json'
            payload.write_text(json.dumps({'draft':False,'prerelease':False,'make_latest':'true','body':body}))
            published=json.loads(gh('api',f'repos/{REPO}/releases/{RELEASE}','--method','PATCH','--input',str(payload)))
        else:published=release
    if published['draft'] or not published['published_at'] or published['tag_name']!=record['tag']:
        raise RuntimeError('Publication not confirmed')
    after=api(f'releases/{RELEASE}')
    if {x['name']:(x['id'],x['size'],x['digest']) for x in after['assets']}!=expected:
        raise RuntimeError('Published inventory mismatch')
    if api('releases/421204266')['draft'] is not True:
        raise RuntimeError('Historical draft state changed')
    print('Published verified release:',after['html_url'])
    print('Installer:',next(x['browser_download_url'] for x in after['assets'] if x['name']==record['installer']['name']))
if __name__=='__main__':main()
