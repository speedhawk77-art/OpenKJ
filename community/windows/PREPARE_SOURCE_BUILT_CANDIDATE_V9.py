#!/usr/bin/env python3
"""Replace only the historical application with a verified source-built app.

The optional GStreamer adapter is packaged outside the scanned plugin directory.
Cisco's binary is NEVER bundled; it is separately fetched only by the end user.
"""
import argparse,hashlib,json,re,shutil,zipfile
from pathlib import Path
from STAGE_OPENKJ_COMPLIANCE_CANDIDATE_V9 import bins,VC_SHA256

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    ap=argparse.ArgumentParser(description=__doc__)
    for key in ('candidate','original','bundle','source-commit','report'):ap.add_argument('--'+key,required=True)
    a=ap.parse_args();root=Path(a.candidate);original=Path(a.original)
    if not re.fullmatch('[0-9a-f]{40}',a.source_commit):raise RuntimeError('Invalid source commit')
    before=bins(root)
    if len(before)!=338:raise RuntimeError('Unexpected input runtime inventory')
    with zipfile.ZipFile(a.bundle) as z:
        expected={'openkj.exe','OpenKJ-Remote.exe','source-build-evidence.json','OpenKJ-v9-Application-Source.zip','optional-codec-test-evidence.json'}
        if len(z.namelist())!=5 or set(z.namelist())!=expected:raise RuntimeError('Unexpected source app bundle inventory')
        data=json.loads(z.read('source-build-evidence.json'));tests=json.loads(z.read('optional-codec-test-evidence.json'))
        exe=z.read('openkj.exe');launcher=z.read('OpenKJ-Remote.exe');source=z.read('OpenKJ-v9-Application-Source.zip')
        if (data['source_commit']!=a.source_commit or data['current_build_source_attested'] is not True
            or hashlib.sha256(exe).hexdigest()!=data['executable_sha256']
            or hashlib.sha256(launcher).hexdigest()!=data['launcher_sha256']
            or hashlib.sha256(source).hexdigest()!=data['application_source_archive_sha256'] or tests['passed'] is not True):
            raise RuntimeError('Source app build evidence mismatch')
        (root/'openkj.exe').write_bytes(exe)
        (root/'OpenKJ-Remote.exe').write_bytes(launcher)
    plugin=Path('optional-codecs/openh264/libgstopenh264.dll');(root/plugin).parent.mkdir(parents=True,exist_ok=True)
    shutil.copy2(original/'gst-plugins/libgstopenh264.dll',root/plugin)
    # The optional plugin has only the original GStreamer bytes, no Cisco binary.
    if (root/'libopenh264-7.dll').exists() or (root/'gst-plugins/libgstopenh264.dll').exists():
        raise RuntimeError('Cisco dependency or active adapter unexpectedly bundled')
    after=bins(root)
    if len(after)!=339 or set(after)-set(before)!={plugin.as_posix()} or set(before)-set(after):
        raise RuntimeError('Unexpected source candidate binary inventory')
    if any(after[k]!=v for k,v in before.items() if k not in ('openkj.exe','OpenKJ-Remote.exe')):raise RuntimeError('Unexpected retained binary modification')
    if sha(root/'vc_redist.x64.exe')!=VC_SHA256:raise RuntimeError('Runtime pin changed')
    staged=json.loads((root/'CANDIDATE-BUILD-EVIDENCE-v9.json').read_text())
    data.update(candidate_runtime_sha256=after,candidate_runtime_binary_count=len(after),
        candidate_notice_sha256=staged['candidate_notice_sha256'],
        purpose='DRAFT ONLY: source-built application candidate with optional adapter and no Cisco binary',
        changes_from_historical_candidate=['openkj.exe and OpenKJ-Remote.exe rebuilt from the same recorded public source',plugin.as_posix()+' added outside scanned plugin directory'],
        optional_plugin_sha256=sha(root/plugin),cisco_binary_bundled=False,optional_codec_default_enabled=False,
        physical_playback_tested=False,release_approved=False)
    (root/'CANDIDATE-BUILD-EVIDENCE-v9.json').write_text(json.dumps(data,indent=2)+'\n')
    Path(a.report).parent.mkdir(parents=True,exist_ok=True)
    Path(a.report).write_text(json.dumps(data,indent=2)+'\n')
    print('Source-built candidate:',a.source_commit,'runtime files:',len(after),'Cisco binary bundled: false')
if __name__=='__main__':main()
