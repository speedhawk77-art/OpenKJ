#!/usr/bin/env python3
"""Publish only the pinned, owner-tested maintenance installer and matching sources."""
import hashlib
import json
import subprocess
import zipfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

from PREPARE_PUBLIC_DRAFT_V9 import REPO, api, gh, one, private, sha, upload

TAG = 'v9.0-offline-remote.3'
SOURCE = '39da17fed3644b8ae93d5e114fb77892b8da6655'
RUN = 40239122777


def main():
    plan = json.loads(Path('community/windows/data/maintenance-publication-v9.json').read_text())
    if (plan['tag'] != TAG or plan['source_commit'] != SOURCE
            or plan['verification_run'] != RUN
            or plan['owner_reported_playback_passed'] is not True
            or plan['private_working_release_id'] != 421204266):
        raise RuntimeError('Unexpected publication plan')
    run = api(f'actions/runs/{RUN}')
    if (run['conclusion'] != 'success' or run['head_sha'] != SOURCE
            or run['head_repository']['full_name'] != REPO
            or run['path'] != '.github/workflows/verify-openkj-v9-maintenance.yml'):
        raise RuntimeError('Pinned maintenance verification has not passed')
    working = api('releases/421204266')
    private(working)
    previous = api('releases/423591307')
    if previous['draft'] or previous['tag_name'] != 'v9.0-offline-remote.2':
        raise RuntimeError('Previous public release changed unexpectedly')
    releases = {working['id']: working, previous['id']: previous}
    pins = plan['assets']
    if len(pins) != 41 or len({p['name'] for p in pins}) != 41:
        raise RuntimeError('Unexpected asset inventory')
    work = Path('maintenance-publication')
    work.mkdir(exist_ok=True)

    def download(pin):
        item = one(releases[pin['release_id']], pin['name'])
        if (item['id'] != pin['id'] or item['size'] != pin['bytes']
                or item['digest'] != 'sha256:' + pin['sha256']
                or '/' in pin['name'] or '\\' in pin['name']):
            raise RuntimeError('Pinned asset changed: ' + pin['name'])
        path = work / pin['name']
        with path.open('wb') as f:
            subprocess.run(['gh', 'api', f"repos/{REPO}/releases/assets/{pin['id']}",
                            '-H', 'Accept: application/octet-stream'], stdout=f, check=True)
        if sha(path) != pin['sha256'] or path.stat().st_size != pin['bytes']:
            raise RuntimeError('Downloaded asset checksum mismatch: ' + pin['name'])
        print('Verified:', pin['name'], flush=True)

    with ThreadPoolExecutor(max_workers=4) as pool:
        list(pool.map(download, pins))
    audit = next(p for p in pins if p['id'] == 676294368)
    installer = next(p for p in pins if p['id'] == 676294470)
    source = next(p for p in pins if p['id'] == 676294806)
    if source['name'] != f'OpenKJ-v9-Application-Source-{SOURCE}.zip':
        raise RuntimeError('Application source does not match')
    with zipfile.ZipFile(work / audit['name']) as z:
        def read(name):
            return json.loads(z.read(name))
        roundtrip = read('OpenKJ-v9-Installer-Roundtrip.json')
        native = read('OpenKJ-v9-Maintenance-Regressions.json')
        http = read('OpenKJ-v9-Installed-App-HTTP-Probe.json')
        candidate = read('OpenKJ-v9-Source-Candidate.json')
    if (roundtrip['passed'] is not True or roundtrip['installer_sha256'] != installer['sha256']
            or roundtrip['installer_bytes'] != installer['bytes']
            or roundtrip['application_source_commit'] != SOURCE
            or roundtrip['installed_runtime_hash_matches'] != 339
            or roundtrip['installed_notice_hash_matches'] != 9597
            or roundtrip['install_exit_code'] != 0
            or len(native['checks']) != 52 or not all(c['passed'] for c in native['checks'])
            or http['passed'] is not True or len(http['checks']) != 11
            or not all(c['passed'] for c in http['checks'])
            or candidate['source_commit'] != SOURCE
            or candidate['cisco_binary_bundled'] is not False):
        raise RuntimeError('Maintenance evidence does not match publication pins')
    notes = Path('community/MAINTENANCE_RELEASE_NOTES_v9.md').read_text()
    notes += '\nInstaller: `' + installer['name'] + '`\n\nSHA-256: `' + installer['sha256'] + '`\n'
    checksum = work / 'SHA256SUMS.txt'
    checksum.write_text(''.join(p['sha256'] + '  ' + p['name'] + '\n'
                                for p in sorted(pins, key=lambda p: p['name'])))
    expected = {p['name']: (p['bytes'], 'sha256:' + p['sha256']) for p in pins}
    expected[checksum.name] = (checksum.stat().st_size, 'sha256:' + sha(checksum))
    matches = [r for r in api('releases?per_page=100') if r['tag_name'] == TAG]
    if len(matches) > 1:
        raise RuntimeError('Duplicate maintenance release tag')
    if matches:
        target = matches[0]
    else:
        request = work / 'create.json'
        request.write_text(json.dumps({'tag_name': TAG, 'target_commitish': SOURCE,
                                      'name': 'OpenKJ Offline Remote v9 — Maintenance Update',
                                      'body': notes, 'draft': True, 'prerelease': False}))
        target = json.loads(gh('api', f'repos/{REPO}/releases', '--method', 'POST', '--input', str(request)))
    if target['body'] != notes or target['prerelease']:
        raise RuntimeError('Unexpected target release notes/state')
    actual = {p['name']: (p['size'], p['digest']) for p in target['assets']}
    if any(name not in expected or value != expected[name] for name, value in actual.items()):
        raise RuntimeError('Unexpected/conflicting asset in target release; no files deleted')
    if target['draft']:
        private(target)
        for pin in pins:
            if pin['name'] not in actual:
                private(api('releases/' + str(target['id'])))
                upload(target['id'], work / pin['name'])
        if checksum.name not in actual:
            private(api('releases/' + str(target['id'])))
            upload(target['id'], checksum)
    target = api('releases/' + str(target['id']))
    if {p['name']: (p['size'], p['digest']) for p in target['assets']} != expected:
        raise RuntimeError('Final release inventory mismatch')
    private(api('releases/421204266'))
    if target['draft']:
        request = work / 'publish.json'
        request.write_text(json.dumps({'draft': False, 'make_latest': 'true'}))
        gh('api', f"repos/{REPO}/releases/{target['id']}", '--method', 'PATCH', '--input', str(request))
    published = api('releases/' + str(target['id']))
    if published['draft'] or not published['published_at']:
        raise RuntimeError('Release was not published')
    latest = api('releases/latest')
    if latest['id'] != published['id']:
        raise RuntimeError('Maintenance release is not the latest public release')
    print('Published:', published['html_url'], flush=True)


if __name__ == '__main__':
    main()
