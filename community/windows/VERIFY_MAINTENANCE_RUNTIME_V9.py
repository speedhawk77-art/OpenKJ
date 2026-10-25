#!/usr/bin/env python3
"""Allow attested app changes while retaining every released dependency hash."""
import argparse,hashlib,json
from pathlib import Path
from STAGE_OPENKJ_COMPLIANCE_CANDIDATE_V9 import bins

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--candidate',type=Path,required=True)
    ap.add_argument('--attestation',type=Path,required=True)
    a=ap.parse_args()
    baseline=json.loads(Path(__file__).with_name('data').joinpath('tested-runtime-v9.json').read_text())['runtime_sha256']
    current=bins(a.candidate)
    evidence=json.loads(a.attestation.read_text())
    permitted={'openkj.exe':evidence['executable_sha256'],'OpenKJ-Remote.exe':evidence['launcher_sha256']}
    if len(current)!=339 or set(current)!=set(baseline):raise RuntimeError('Maintenance runtime inventory changed')
    if evidence['current_build_source_attested'] is not True or evidence['cisco_binary_bundled'] is not False:
        raise RuntimeError('Missing source/codec attestation')
    for name,pin in current.items():
        if pin!=permitted.get(name,baseline[name]):raise RuntimeError('Unapproved runtime change: '+name)
    print('337 dependency files match released v9; both executables match the new source attestation.')
if __name__=='__main__':main()
