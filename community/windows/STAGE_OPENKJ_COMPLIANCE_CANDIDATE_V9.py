#!/usr/bin/env python3
"""Create a SEPARATE OpenKJ v9 distribution candidate with additional notices.

No downloads, no execution-policy changes, no binary patching, no GitHub publish.
Only copies existing files from a local OpenKJ folder, a checked-out source tree,
and a SHA-256-pinned MSVC redist. The existing OpenKJ installation is never changed.

This is a candidate preparation tool, not a redistribution legal certificate.
Usage (Windows, Python 3.10+):
    py -3 STAGE_OPENKJ_COMPLIANCE_CANDIDATE_V9.py --cleaned "C:/OpenKJ-Development/OpenKJ-Distribution-Test-v9" --repo "C:/OpenKJ-Development/OpenKJ-Source" --vc-redist "C:/path/to/vc_redist.x64.exe" --omit-openh264
"""
from __future__ import annotations
import argparse, hashlib, json, shutil, sys, zipfile
from pathlib import Path, PurePosixPath

VC_SHA256 = '843068991daaa1f73ad9f6239bce4d0f6a07a51f18c37ea2a867e9beca71295c'
OMIT_CODEC = ('gst-plugins/libgstopenh264.dll','libopenh264-7.dll')
CERBERO = ('Apache-2.0.txt','GPL-2+.txt','GPL-3+.txt','LGPL-2+.txt',
           'LGPL-2.1+.txt','LGPL-3+.txt','MPL-1.1.txt','MPL-2.txt')

def digest(p:Path)->str:
    h=hashlib.sha256()
    with p.open('rb') as f:
        for block in iter(lambda:f.read(1024*1024),b''): h.update(block)
    return h.hexdigest()

def bins(root:Path)->dict[str,str]:
    return {p.relative_to(root).as_posix():digest(p) for p in root.rglob('*')
            if p.is_file() and p.suffix.lower() in {'.dll','.exe','.pyd','.ocx'}
            and p.name.lower() != 'vc_redist.x64.exe'}

def install_notice_supplement(bundle:Path,target:Path)->int:
    if target.exists():raise RuntimeError('Supplement destination already exists')
    with zipfile.ZipFile(bundle) as z:
        names=z.namelist()
        if len({n.casefold() for n in names})!=len(names):
            raise RuntimeError('Duplicate/colliding supplement paths')
        evidence_name='OpenKJ-v9-Upstream-Notice-Evidence-v2.json'
        data=json.loads(z.read(evidence_name))
        expected={d['installed_supplement_path']:d['sha256'] for d in data['documents']}
        if len(expected)!=data['document_count'] or set(names)!=set(expected)|{evidence_name}:
            raise RuntimeError('Supplement inventory does not match its evidence manifest')
        for name in names:
            p=PurePosixPath(name)
            if p.is_absolute() or '..' in p.parts or '\\' in name or ':' in name:
                raise RuntimeError('Unsafe supplement path')
            if p.suffix.lower() in {'.exe','.dll','.pyd','.ocx','.py','.ps1','.bat','.cmd'}:
                raise RuntimeError('Executable/script found in notice supplement')
            raw=z.read(name)
            if name in expected and hashlib.sha256(raw).hexdigest()!=expected[name]:
                raise RuntimeError('Supplement legal-document checksum mismatch')
        for name in names:
            path=target/name;path.parent.mkdir(parents=True,exist_ok=True)
            path.write_bytes(z.read(name))
    return len(expected)

def main()->int:
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--cleaned',required=True,type=Path,help='Previously tested OpenKJ-Distribution-Test-v9 folder')
    ap.add_argument('--repo',required=True,type=Path,help='Locally updated offline-remote-v9 GitHub source checkout')
    ap.add_argument('--vc-redist',required=True,type=Path,help='Microsoft-signed vc_redist.x64.exe, version 14.51.36247.0')
    ap.add_argument('--omit-openh264',action='store_true',help='Exclude both OpenH264 DLLs; video decoding MUST be retested')
    ap.add_argument('--out',type=Path,help='Output folder (default: adjacent OpenKJ-Compliance-Candidate-v9)')
    ap.add_argument('--notice-supplement',type=Path,help='Verified v2 upstream notice bundle (optional)')
    ap.add_argument('--rust-notice-supplement',type=Path,help='Verified librsvg locked Rust crate notice bundle (optional)')
    ap.add_argument('--rust-standard-notice-supplement',type=Path,help='Verified Rust 1.85.0 standard-library notice bundle (optional)')
    ap.add_argument('--qt-notice-supplement',type=Path,help='Verified original Qt module attribution and notice bundle (optional)')
    ap.add_argument('--retained-plugin-notice-supplement',type=Path,help='Checksum-verified retained-plugin notice bundle')
    x=ap.parse_args()
    cleaned=x.cleaned.expanduser().resolve(); repo=x.repo.expanduser().resolve()
    vc=x.vc_redist.expanduser().resolve()
    out=(x.out or cleaned.parent/'OpenKJ-Compliance-Candidate-v9').resolve()
    if not cleaned.is_dir() or not (cleaned/'openkj.exe').is_file() or not (cleaned/'OpenKJ-Remote.exe').is_file():
        ap.error('Cleaned source folder is not a working OpenKJ portable folder.')
    src=repo/'community'/'licenses'
    if not src.is_dir() or not all((src/'cerbero-1.26.9'/n).is_file() for n in CERBERO):
        ap.error('Source checkout lacks the 8 newly recovered Cerbero terms. Update the offline-remote-v9 branch first.')
    for needed in (repo/'THIRD_PARTY_NOTICES.md',repo/'community'/'WINDOWS_DLL_REPLACEMENT_GUIDE_v9.md'):
        if not needed.is_file(): ap.error('Missing source notice: '+str(needed))
    if not vc.is_file(): ap.error('VC++ redist file not found: '+str(vc))
    actual=digest(vc)
    if actual.lower()!=VC_SHA256:
        ap.error('VC++ redist SHA-256 mismatch. Refusing to bundle unverified file. Got '+actual)
    if out.exists(): ap.error('Output already exists. Refusing to overwrite: '+str(out))
    if out==cleaned or out.is_relative_to(cleaned) or cleaned.is_relative_to(out):
        ap.error('Output must be in a separate sibling folder, not inside the input.')
    expected=bins(cleaned)
    if len(expected)!=340:
        ap.error(f'Expected 340 cleaned app binaries, got {len(expected)}. Wrong input folder or modified build.')
    if len([k for k in expected if k.startswith('gst-plugins/') and k.endswith('.dll')]) !=196:
        ap.error('Expected exactly 196 cleaned GStreamer plug-in DLLs.')
    if x.omit_openh264 and not all(n in expected for n in OMIT_CODEC):
        ap.error('Expected both OpenH264 binaries in input but at least one is absent.')
    if not (cleaned/'licenses').is_dir():
        ap.error('Previously staged 118-file licence directory is not present in cleaned folder.')
    print('Staging a separate candidate. Existing binaries will not be modified.')
    try:
        shutil.copytree(cleaned,out)
        # Keep existing 118 SDK/core notices and ADD source-tagged original texts.
        supplemental=out/'licenses'/'upstream-verified-source-notices'
        shutil.copytree(src,supplemental)
        archive_notice_count=0
        if x.notice_supplement:
            archive_notice_count=install_notice_supplement(x.notice_supplement,
                out/'licenses'/'upstream-source-archive-notices')
        rust_notice_count=0
        if x.rust_notice_supplement:
            rust_notice_count=install_notice_supplement(x.rust_notice_supplement,
                out/'licenses'/'rust-crate-notices')
        standard_notice_count=0
        if x.rust_standard_notice_supplement:
            standard_notice_count=install_notice_supplement(x.rust_standard_notice_supplement,
                out/'licenses'/'rust-standard-library-notices')
        qt_notice_count=0
        if x.qt_notice_supplement:
            qt_notice_count=install_notice_supplement(x.qt_notice_supplement,out/'licenses'/'qt-module-notices')
        plugin_notice_count=0
        if x.retained_plugin_notice_supplement:
            plugin_notice_count=install_notice_supplement(x.retained_plugin_notice_supplement,out/'licenses'/'retained-plugin-notices')
        shutil.copy2(repo/'community'/'SOURCE_ACCESS_v9.md',out/'licenses'/'SOURCE_ACCESS_v9.md')
        shutil.copy2(repo/'THIRD_PARTY_NOTICES.md',out/'licenses'/'THIRD_PARTY_NOTICES-updated.md')
        shutil.copy2(repo/'community'/'WINDOWS_DLL_REPLACEMENT_GUIDE_v9.md',out/'licenses'/'WINDOWS_DLL_REPLACEMENT_GUIDE_v9.md')
        if x.omit_openh264:
            for path in OMIT_CODEC: (out/path).unlink()
        # The existing Inno builder prefers a vc_redist.x64.exe in the app folder.
        # Its normal stage_copy excludes the .exe from installed app files and
        # embeds it separately in the temporary Inno installer payload.
        shutil.copy2(vc,out/'vc_redist.x64.exe')
        (out/'licenses'/'SOURCE_AND_DISTRIBUTION_STATUS.txt').write_text(
            'OpenKJ Offline Remote Community Edition v9 - source, licences and LGPL rights\n\n'
            'Exact application source: https://github.com/speedhawk77-art/OpenKJ/tree/016d5c3135bbc8073b96881e49e19feb97359101\n'
            'Source access, archive versions and build instructions: licenses/SOURCE_ACCESS_v9.md\n'
            'Source mirror status: https://github.com/speedhawk77-art/OpenKJ/blob/offline-remote-v9/community/SOURCE_DISTRIBUTION_STATUS_v9.md\n'
            'Review archives are held in draft release ID 421204266; draft assets are not public source delivery.\n'
            'Original Qt 5.15.2 and GStreamer 1.26.9 licence texts, Cerbero package terms, '+
            'ZBar and ANGLE notices: licenses/upstream-verified-source-notices/.\n'
            'GStreamer component-specific SDK notices: licenses/gstreamer-1.26.9/.\n'
            'Replacing Qt/GStreamer LGPL libraries: licenses/WINDOWS_DLL_REPLACEMENT_GUIDE_v9.md.\n'
            'License freedoms, including reverse-engineering for modification and replacing '+
            'the LGPL-covered libraries, are not restricted by this community edition.\n\n'
            'Microsoft runtime and SDK component terms: licenses/upstream-verified-source-notices/microsoft/.\n'
            'The SDK-built OpenH264 binary is omitted; any optional Cisco binary is downloaded separately by the end user.\n'
            'Release packaging must make the source archives in SOURCE_ACCESS_v9.md available alongside this installer.\n',encoding='utf-8')
        after=bins(out)
        intended=set(OMIT_CODEC if x.omit_openh264 else ())
        if {k for k in expected if k not in after}!=intended or set(after)-set(expected) or any(expected[k]!=v for k,v in after.items()):
            raise RuntimeError('Unexpected binary inventory difference detected. Candidate not valid.')
        report={
            'purpose':'DRAFT ONLY: separately staged notice/codec candidate, not licence clearance',
            'source_runtime_binary_count':len(expected),
            'candidate_runtime_binary_count':len(after),
            'candidate_runtime_sha256':after,
            'candidate_notice_sha256':{p.relative_to(out).as_posix():digest(p)
                for p in sorted((out/'licenses').rglob('*')) if p.is_file()},
            'omitted_openh264':sorted(intended),
            'binary_hashes_unchanged_except_omissions':True,
            'original_license_directory_retained':True,
            'extra_source_notice_texts':len([p for p in supplemental.rglob('*') if p.is_file()]),
            'archive_derived_notice_files':archive_notice_count,
            'retained_plugin_notice_files':plugin_notice_count,
            'retained_plugin_notice_supplement_sha256':digest(x.retained_plugin_notice_supplement) if x.retained_plugin_notice_supplement else None,
            'qt_module_notice_files':qt_notice_count,
            'qt_notice_supplement_sha256':digest(x.qt_notice_supplement) if x.qt_notice_supplement else None,
            'rust_standard_notice_files':standard_notice_count,
            'rust_standard_notice_supplement_sha256':digest(x.rust_standard_notice_supplement) if x.rust_standard_notice_supplement else None,
            'rust_crate_notice_files':rust_notice_count,
            'rust_notice_supplement_sha256':digest(x.rust_notice_supplement) if x.rust_notice_supplement else None,
            'notice_supplement_sha256':digest(x.notice_supplement) if x.notice_supplement else None,
            'vc_redist_sha256':actual,
            'vc_redist_signed_status':'Previously validated Microsoft signed for this SHA-256; independently verify on build PC',
            'release_approved':False,
            'review_required':['Test the new candidate with all media formats',
                 'Confirm corresponding LGPL sources and exact build match for all covered binaries',
                 'Verify full installed notice completeness after compiling installer',
                 'Review patent rights for actual H264 codecs in shipped binary',
                 'Check embedded VC redist hash after pinning builder'],
        }
        (out/'CANDIDATE-BUILD-EVIDENCE-v9.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
        print(f'SUCCESS: {len(after)} runtime binary hashes verified; {report["extra_source_notice_texts"]} added source-notice files.')
        print('Output candidate:',out)
        print('Pinned VC++ redist SHA-256:',actual)
        if x.omit_openh264: print('OpenH264 binaries excluded; verify MP4/H.264 karaoke playback before any installer rebuild.')
        else: print('WARNING: OpenH264 still bundled; separate codec/patent redistribution conditions unresolved.')
        print('DRAFT TEST ONLY. NO INSTALLER PUBLISHED. The original folder is unchanged.')
        return 0
    except Exception:
        # Prevent a half-staged candidate from being mistaken for completed output.
        if out.exists(): shutil.rmtree(out)
        raise
if __name__=='__main__':
    try: raise SystemExit(main())
    except (OSError,RuntimeError) as exc:
        print('ERROR:',exc,file=sys.stderr);raise SystemExit(1)
