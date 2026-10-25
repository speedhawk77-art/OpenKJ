#!/usr/bin/env python3
"""Reject any program change in the documentation-only installer refresh."""
import argparse,json
from pathlib import Path
from STAGE_OPENKJ_COMPLIANCE_CANDIDATE_V9 import bins

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--candidate',type=Path,required=True)
    ap.add_argument('--baseline',type=Path,default=Path(__file__).with_name('data')/'tested-runtime-v9.json')
    a=ap.parse_args()
    baseline=json.loads(a.baseline.read_text())
    expected=baseline['runtime_sha256'];actual=bins(a.candidate)
    if len(expected)!=339 or actual!=expected:
        changed=sorted(k for k in set(actual)|set(expected) if actual.get(k)!=expected.get(k))
        raise RuntimeError('Runtime differs from the user-tested candidate: '+', '.join(changed))
    print('All 339 program files match the user-tested candidate; documentation-only refresh.')
if __name__=='__main__':main()
