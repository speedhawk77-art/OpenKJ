#!/usr/bin/env python3
"""Create a one-file OpenKJ installer from a working customized portable folder.

Place this script and its .bat next to openkj.exe + OpenKJ-Remote.exe,
then double-click the .bat. Requires Inno Setup 6 or 7 on the BUILD PC.
Does not change any OpenKJ sources, binaries, local databases, or settings.
"""
from __future__ import annotations

import argparse
import hashlib
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

APP_DISPLAY = 'OpenKJ (Custom v9)'
OUTPUT_FILE = 'OpenKJ-v9-Setup.exe'
GUID = '6C8A20EB-B83B-4DB9-96D5-C4747451E8B6'  # Consistent upgrades/uninstall identity
VC_DOWNLOAD_URL = 'https://aka.ms/vc14/vc_redist.x64.exe'

# Exclude non-portable, potentially sensitive material from a working portable bundle.
# The Qt and GStreamer DLL/plugin trees, including subfolders, remain intact.
SKIP_FOLDERS = {
    '.git', '.vs', '__pycache__', 'build', 'source', 'sources', 'backups',
    'backup', 'karaoke', 'music', 'songs', 'videos', 'media', 'recordings',
    'installeroutput', 'openkj-installer-output', 'installer-work', 'temp', 'tmp',
    'logs', 'log', '.cache',
}
SKIP_EXTENSIONS = {
    '.bat', '.cmd', '.ps1', '.py', '.pyc', '.iss', '.zip', '.7z', '.rar',
    '.bak', '.obj', '.pdb', '.ilk', '.lib', '.exp', '.log',
    '.sqlite', '.sqlite3', '.db', '.db3', '.db-journal', '.sqlite-wal', '.sqlite-shm',
    '.mp3', '.mp4', '.m4a', '.cdg', '.mkv', '.avi', '.wav', '.flac', '.mpg', '.wmv', '.mov',
    '.csv', '.xlsx', '.xls', '.doc', '.docx', '.pdf',
}
SKIP_FILE_NAMES = {
    'vc_redist.x64.exe', 'vc_redist.x86.exe',
    'openkj-remote.previous.exe',
    'openkj-remote-launcher.previous.exe',
}


def checked_directory(source: Path) -> None:
    required = [
        'openkj.exe',
        'OpenKJ-Remote.exe',
        'Qt5Core.dll',
        'libgstreamer-1.0-0.dll',
        'platforms/qwindows.dll',
    ]
    for file_name in required:
        if not (source / file_name).is_file():
            raise RuntimeError(
                'Missing ' + file_name + ' in ' + str(source) + '\n'
                'Put both builder files in the WORKING OpenKJ-Portable folder, '
                'next to OpenKJ-Remote.exe, not in OpenKJ-Source.'
            )
    plugins = source / 'gst-plugins'
    if not plugins.is_dir() or not any(plugins.glob('*.dll')):
        raise RuntimeError('Missing GStreamer plugins in ' + str(plugins))
    if not (source / 'sqldrivers' / 'qsqlite.dll').is_file():
        raise RuntimeError('Missing Qt SQLite plugin: sqldrivers/qsqlite.dll')


def find_compiler() -> Path | None:
    override = os.environ.get('INNO_SETUP_COMPILER')
    if override and Path(override).is_file():
        return Path(override)
    where = shutil.which('ISCC.exe') or shutil.which('ISCC')
    if where:
        return Path(where)
    roots = []
    for envname in ['ProgramFiles', 'ProgramFiles(x86)']:
        val = os.environ.get(envname)
        if val:
            roots.append(Path(val))
    for root in roots:
        for folder in ['Inno Setup 7', 'Inno Setup 6', 'Inno Setup 7 (x64)']:
            test = root / folder / 'ISCC.exe'
            if test.is_file():
                return test
    return None


def _version_sort(path: Path) -> tuple[int, ...]:
    versions = [int(v) for v in re.findall(r'\d+', str(path.parent.name))]
    return tuple(versions)


def find_vc_redist(source: Path) -> Path | None:
    # Prefer an explicitly supplied Microsoft installer in the portable directory.
    supplied = source / 'vc_redist.x64.exe'
    if supplied.is_file() and supplied.stat().st_size > 1_000_000:
        return supplied
    candidates: list[Path] = []
    roots = []
    for name in ('ProgramFiles', 'ProgramFiles(x86)'):
        val = os.environ.get(name)
        if val:
            roots.append(Path(val) / 'Microsoft Visual Studio')
    # Modern Visual Studio Community/Pro/Enterprise/BuildTools locations.
    for base in roots:
        if not base.is_dir():
            continue
        for vs_version in base.iterdir():
            if not vs_version.is_dir():
                continue
            for edition in vs_version.iterdir():
                vc_redist_dir = edition / 'VC' / 'Redist' / 'MSVC'
                if not vc_redist_dir.is_dir():
                    continue
                candidates.extend(vc_redist_dir.glob('*/vc_redist.x64.exe'))
    candidates = [p for p in candidates if p.is_file() and p.stat().st_size > 1_000_000]
    return sorted(candidates, key=_version_sort, reverse=True)[0] if candidates else None


def keep_file(relative: Path) -> bool:
    parts = relative.parts
    # Licence trees contain text such as COPYING.LIB. Build-artifact suffix
    # filters must not discard these original legal documents.
    if parts and parts[0].lower() == 'licenses':
        return (not any(part.startswith('.') for part in parts)
                and relative.suffix.lower() not in {
                    '.exe', '.dll', '.pyd', '.ocx', '.bat', '.cmd', '.ps1',
                    '.py', '.pyc', '.zip', '.7z', '.rar'})
    if any(part.lower() in SKIP_FOLDERS or part.startswith('.') for part in parts[:-1]):
        return False
    name = relative.name
    lowname = name.lower()
    if lowname in SKIP_FILE_NAMES or lowname.endswith(('.previous.exe', '.old.exe')):
        return False
    if lowname.startswith(('openkj-remote-v9-setup', 'openkj-remote-setup', 'openkj-v9-setup', 'openkj-setup')):
        return False
    if name.startswith('.') or Path(name).suffix.lower() in SKIP_EXTENSIONS:
        return False
    return True


def stage_copy(source: Path, destination: Path) -> tuple[int, int, int]:
    count = 0
    skipped = 0
    bytes_total = 0
    for dirname, subdirs, filenames in os.walk(source, topdown=True, followlinks=False):
        directory = Path(dirname)
        rel_dir = directory.relative_to(source)
        in_licenses = bool(rel_dir.parts) and rel_dir.parts[0].lower() == 'licenses'
        subdirs[:] = [d for d in subdirs
                       if (in_licenses or d.lower() not in SKIP_FOLDERS) and not d.startswith('.')
                       and not (directory / d).is_symlink()]
        for filename in filenames:
            file = directory / filename
            relative = rel_dir / filename
            if file.is_symlink() or not file.is_file() or not keep_file(relative):
                skipped += 1
                continue
            target = destination / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(file, target)
            count += 1
            bytes_total += file.stat().st_size
    return count, skipped, bytes_total


def verify_staged_notices(source: Path, staging: Path) -> None:
    for original in (source / 'licenses').rglob('*'):
        if not original.is_file():
            continue
        relative = original.relative_to(source)
        copied = staging / relative
        if (not copied.is_file() or
                hashlib.sha256(original.read_bytes()).digest() != hashlib.sha256(copied.read_bytes()).digest()):
            raise RuntimeError('Installer staging omitted or changed a licence document: ' + str(relative))


def iss_path_literal(path: Path) -> str:
    return str(path.resolve()).replace('"', '""')


def write_inno_script(stage: Path, vc_redist: Path, outdir: Path, iss_path: Path) -> None:
    files = f'''Source: "{iss_path_literal(stage)}\\*"; DestDir: "{{app}}"; Flags: ignoreversion recursesubdirs createallsubdirs\n'''
    files += f'''Source: "{iss_path_literal(vc_redist)}"; DestDir: "{{tmp}}"; DestName: "vc_redist.x64.exe"; Flags: deleteafterinstall\n'''
    script = f''' ; Auto-generated by MAKE_SINGLE_FILE_INSTALLER.py - do not edit by hand.
[Setup]
AppId=@APP_ID@
AppName=OpenKJ
AppVersion=9.0
AppVerName={APP_DISPLAY}
DefaultDirName={{autopf}}\\OpenKJ Custom
DefaultGroupName=OpenKJ
UninstallDisplayIcon={{app}}\\openkj.exe
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir={iss_path_literal(outdir)}
OutputBaseFilename={OUTPUT_FILE[:-4]}
Compression=lzma2/normal
SolidCompression=yes
WizardStyle=modern
CloseApplications=yes
RestartApplications=no
DisableProgramGroupPage=yes
UsePreviousAppDir=yes
SetupLogging=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional shortcuts:"; Flags: unchecked

[Files]
{files}
[Icons]
Name: "{{autoprograms}}\\OpenKJ"; Filename: "{{app}}\\OpenKJ-Remote.exe"; WorkingDir: "{{app}}"; IconFilename: "{{app}}\\openkj.exe"
Name: "{{autodesktop}}\\OpenKJ"; Filename: "{{app}}\\OpenKJ-Remote.exe"; WorkingDir: "{{app}}"; IconFilename: "{{app}}\\openkj.exe"; Tasks: desktopicon

[Run]
; Bundled official Microsoft C++ x64 Runtime. Runs only if not installed.
Filename: "{{tmp}}\\vc_redist.x64.exe"; Parameters: "/install /quiet /norestart"; StatusMsg: "Installing required Microsoft C++ runtime..."; Flags: waituntilterminated runhidden; Check: not VCRuntimePresent
Filename: "{{app}}\\OpenKJ-Remote.exe"; Description: "Launch OpenKJ"; Flags: nowait postinstall skipifsilent

[Code]
function VCRuntimePresent(): Boolean;
var
  Installed: Cardinal;
begin
  Result := RegQueryDWordValue(HKLM64,
    'SOFTWARE\\Microsoft\\VisualStudio\\14.0\\VC\\Runtimes\\x64',
    'Installed', Installed) and (Installed = 1);
end;
'''
    # Strip optional leading space from header before writing.
    iss_path.write_text(script.lstrip().replace('@APP_ID@', '{{' + GUID + '}'), encoding='utf-8-sig')


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, default=Path(__file__).resolve().parent,
                        help='Folder containing working OpenKJ-Remote.exe + bundled Qt/GStreamer runtime.')
    parser.add_argument('--preview', action='store_true',
                        help='Generate installer script/staged files in a temporary directory; do not compile.')
    args = parser.parse_args()
    source = args.source.resolve()
    output_dir = source.parent / 'OpenKJ-Installer-Output'
    try:
        checked_directory(source)
        vc_redist = find_vc_redist(source)
        if not vc_redist:
            raise RuntimeError(
                'Microsoft Visual C++ x64 runtime installer was not found.\n'
                'To keep this a TRUE one-file installer, download the official\n'
                'vc_redist.x64.exe from:\n  ' + VC_DOWNLOAD_URL + '\n'
                'Save it directly beside OpenKJ-Remote.exe in your portable folder,\n'
                'then run MAKE_SINGLE_FILE_INSTALLER.bat again.'
            )
        compiler = find_compiler() if not args.preview else None
        if not compiler and not args.preview:
            raise RuntimeError(
                'Inno Setup compiler (ISCC.exe) is not installed.\n'
                'Install Inno Setup 7 (64-bit) ONE TIME from:\n'
                '  https://jrsoftware.org/isdl.php\n'
                'Then double-click MAKE_SINGLE_FILE_INSTALLER.bat again.'
            )
        if output_dir == source or output_dir.is_relative_to(source):
            raise RuntimeError('Unsafe installer output location (inside the portable folder).')
        with tempfile.TemporaryDirectory(prefix='openkj-setup-staging-') as staging_root:
            staging_root = Path(staging_root)
            staging = staging_root / 'app'
            staging.mkdir()
            count, skipped, size = stage_copy(source, staging)
            verify_staged_notices(source, staging)
            checked_directory(staging)
            iss_path = staging_root / 'OpenKJ-v9.iss'
            output_dir.mkdir(parents=True, exist_ok=True)
            write_inno_script(staging, vc_redist, output_dir, iss_path)
            print('Portable directory: ', source)
            print(f'Collected runtime:   {count:,} files, {size / 1048576:,.1f} MiB')
            print('Skipped irrelevant/personal files:', skipped)
            print('Microsoft VC runtime:', vc_redist)
            print('Installer destination:', output_dir / OUTPUT_FILE)
            if args.preview:
                # Store only the generated text for inspection, not a misleading empty EXE.
                preview_file = output_dir / 'OpenKJ-v9-PREVIEW.iss'
                preview_file.write_bytes(iss_path.read_bytes())
                print('PREVIEW ONLY: ', preview_file)
                return 0
            print('\nBuilding a single installer file (large bundles can take a while)...\n', flush=True)
            proc = subprocess.run([str(compiler), str(iss_path)], cwd=source)
            if proc.returncode != 0:
                raise RuntimeError('Inno Setup compilation failed with exit code ' + str(proc.returncode))
            installer = output_dir / OUTPUT_FILE
            if not installer.is_file() or installer.stat().st_size < 100_000:
                raise RuntimeError('Compiler finished but expected installer EXE was not found: ' + str(installer))
            print('\n' + '=' * 65)
            print('SUCCESS! TRANSFER JUST THIS ONE FILE TO ANOTHER COMPUTER:')
            print(installer)
            print(f'Size: {installer.stat().st_size / 1048576:,.1f} MiB')
            print('=' * 65)
            print('Run that installer as administrator on Windows 10/11 x64.')
            print('Your karaoke song files and personal OpenKJ database are NOT included.')
        return 0
    except (RuntimeError, OSError, subprocess.SubprocessError) as error:
        print('\nERROR:', error, file=sys.stderr)
        return 1


if __name__ == '__main__':
    raise SystemExit(main())
