#!/usr/bin/env python3
"""Compare the packaged software renderer with Qt's exact Mesa preview build."""
import argparse,json,subprocess
from pathlib import Path
from MIRROR_TRANSITIVE_SOURCES_V9 import download,sha256


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    for name in ('candidate','work','report'):ap.add_argument('--'+name,type=Path,required=True)
    a=ap.parse_args();a.work.mkdir(parents=True,exist_ok=True)
    archive=a.work/'qt-opengl32sw-mesa12rc2.7z'
    url='https://download.qt.io/development_releases/prebuilt/llvmpipe/windows/opengl32sw-64-mesa_12_0_rc2.7z'
    pin='2a0d2f92c60e0962ef5f6039d3793424c6f39e49ba27ac04a5b21ca4ae012e15'
    download([url],archive,pin)
    dest=a.work/'extracted'
    subprocess.run(['7z','x','-y','-o'+str(dest.resolve()),str(archive.resolve())],check=True)
    target=sha256(a.candidate/'opengl32sw.dll')
    matches=[p.relative_to(dest).as_posix() for p in dest.rglob('*.dll') if sha256(p)==target]
    result={'check':'Exact Qt-published Mesa 12 RC2 binary byte identity',
            'reference_url':url,'reference_archive_sha256':pin,'candidate_sha256':target,
            'matching_reference_paths':matches,'passed':bool(matches),
            'exact_upstream_source_build_attested':False,'release_approved':False}
    a.report.parent.mkdir(parents=True,exist_ok=True)
    a.report.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
    print('Qt-published Mesa 12 RC2 byte match:',bool(matches))
    if not matches:raise RuntimeError('Packaged software OpenGL did not match Qt preview reference')


if __name__=='__main__':main()
