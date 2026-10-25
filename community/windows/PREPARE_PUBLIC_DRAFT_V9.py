#!/usr/bin/env python3
"""Copy the verified installer and selected sources to a separate unpublished draft.

Requires GitHub CLI with GH_TOKEN. Does not delete assets or publish releases.
"""
import argparse,hashlib,json,subprocess
from pathlib import Path

REPO='speedhawk77-art/OpenKJ'
def gh(*args):
    return subprocess.check_output(['gh',*args],text=True)
def api(path):return json.loads(gh('api',f'repos/{REPO}/'+path))
def sha(path):
    h=hashlib.sha256()
    with path.open('rb') as f:
        for block in iter(lambda:f.read(1024*1024),b''):h.update(block)
    return h.hexdigest()
def private(release):
    if release['draft'] is not True or release['published_at'] is not None:
        raise RuntimeError('Release is not an unpublished draft')
def one(release,name):
    matches=[a for a in release['assets'] if a['name']==name]
    if len(matches)!=1:raise RuntimeError('Expected exactly one asset: '+name)
    return matches[0]

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--candidate-run',type=int,required=True)
    ap.add_argument('--work',type=Path,required=True)
    a=ap.parse_args()
    if a.candidate_run<=0:raise RuntimeError('Invalid run ID')
    if a.work.exists():raise RuntimeError('Work directory already exists')
    a.work.mkdir(parents=True)
    run=api(f'actions/runs/{a.candidate_run}')
    if (run['conclusion']!='success' or run['head_branch']!='offline-remote-v9'
        or run['head_repository']['full_name']!=REPO
        or run['path']!='.github/workflows/verify-openkj-v9-source-candidate.yml'):
        raise RuntimeError('Expected a successful source candidate run from this branch/repository')
    evidence=a.work/'evidence'
    gh('run','download',str(a.candidate_run),'--repo',REPO,'--name','OpenKJ-v9-Source-Candidate-Audit','--dir',str(evidence))
    def read(name):return json.loads((evidence/name).read_text())
    baseline=json.loads(Path(__file__).with_name('data').joinpath('tested-runtime-v9.json').read_text())
    candidate=read('OpenKJ-v9-Source-Candidate.json')
    roundtrip=read('OpenKJ-v9-Installer-Roundtrip.json')
    http=read('OpenKJ-v9-Installed-App-HTTP-Probe.json')
    if (candidate['candidate_runtime_sha256']!=baseline['runtime_sha256']
        or candidate['source_commit']!=baseline['source_commit']
        or candidate['cisco_binary_bundled'] is not False
        or roundtrip['passed'] is not True or roundtrip['installed_runtime_hash_matches']!=339
        or roundtrip['installed_notice_hash_matches']<9500
        or http['passed'] is not True or len(http['checks'])!=11):
        raise RuntimeError('Final installer/notice/runtime evidence does not pass the release selection gate')
    plan=json.loads(Path(__file__).with_name('data').joinpath('public-release-source-assets-v9.json').read_text())
    working=api('releases/'+str(plan['working_draft_id']));private(working)
    installer_name=f"OpenKJ-v9-Source-Candidate-{baseline['source_commit']}-{a.candidate_run}-{run['run_attempt']}.exe"
    installer=one(working,installer_name)
    if (installer['digest']!='sha256:'+roundtrip['installer_sha256'] or installer['size']!=roundtrip['installer_bytes']):
        raise RuntimeError('Installer asset differs from successful installed-file evidence')
    selected=[installer]
    for pin in plan['assets']:
        item=one(working,pin['name'])
        if item['id']!=pin['id'] or item['size']!=pin['bytes'] or item['digest']!='sha256:'+pin['sha256']:
            raise RuntimeError('Pinned source asset changed: '+pin['name'])
        selected.append(item)
    for name in plan['additional_required_assets']:selected.append(one(working,name))
    # Download and verify everything before creating or modifying the public-facing draft.
    for item in selected:
        if '/' in item['name'] or '\\' in item['name']:raise RuntimeError('Unsafe asset name')
        path=a.work/item['name']
        with path.open('wb') as f:
            subprocess.run(['gh','api',f"repos/{REPO}/releases/assets/{item['id']}",'-H','Accept: application/octet-stream'],stdout=f,check=True)
        if sha(path)!=item['digest'].removeprefix('sha256:') or path.stat().st_size!=item['size']:
            raise RuntimeError('Downloaded source/installer checksum mismatch')
    notes=Path('community/PUBLIC_RELEASE_NOTES_v9.md').read_text()
    notes+='\nInstaller: `'+installer_name+'`\n\nSHA-256: `'+roundtrip['installer_sha256']+'`\n'
    notes_path=a.work/'release-notes.md';notes_path.write_text(notes)
    tag=plan['public_draft_tag']
    releases=api('releases?per_page=100');matches=[r for r in releases if r['tag_name']==tag]
    if len(matches)>1:raise RuntimeError('Duplicate target release tags')
    if matches:
        target=matches[0];private(target)
        if target['body']!=notes:raise RuntimeError('Existing target draft has different notes; review before changing it')
    else:
        gh('release','create',tag,'--repo',REPO,'--draft','--target','offline-remote-v9','--title','OpenKJ Offline Remote v9','--notes-file',str(notes_path))
        target=api('releases/tags/'+tag);private(target)
    expected={x['name']:x['digest'] for x in selected}
    if any(x['name'] not in set(expected)|{'SHA256SUMS.txt'} for x in target['assets']):raise RuntimeError('Unexpected asset in target draft; no assets deleted')
    for item in selected:
        target=api('releases/'+str(target['id']));private(target)
        present=[x for x in target['assets'] if x['name']==item['name']]
        if present:
            if len(present)!=1 or present[0]['digest']!=item['digest']:raise RuntimeError('Conflicting target draft asset')
        else:gh('release','upload',tag,str(a.work/item['name']),'--repo',REPO)
    checksum=a.work/'SHA256SUMS.txt'
    checksum.write_text(''.join(x['digest'].removeprefix('sha256:')+'  '+x['name']+'\n' for x in sorted(selected,key=lambda x:x['name'])))
    target=api('releases/'+str(target['id']));private(target)
    sums=[x for x in target['assets'] if x['name']==checksum.name]
    if sums:
        if len(sums)!=1 or sums[0]['digest']!='sha256:'+sha(checksum):raise RuntimeError('Conflicting checksum asset')
    else:gh('release','upload',tag,str(checksum),'--repo',REPO)
    target=api('releases/'+str(target['id']));private(target)
    expected[checksum.name]='sha256:'+sha(checksum)
    if {x['name']:x['digest'] for x in target['assets']}!=expected:raise RuntimeError('Final target draft inventory mismatch')
    print('Prepared unpublished clean draft:',target['html_url'])
if __name__=='__main__':main()
