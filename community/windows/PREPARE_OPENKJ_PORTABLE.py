"""Prepare a Windows portable OpenKJ bundle from an already configured Release build.

Requires installed Qt 5.15.2 MSVC2019_64, GStreamer MinGW x64, Python 3.
Only changes src/main.cpp (with a timestamped backup) when --prepare is used.
"""
from __future__ import annotations

import argparse
import datetime as dt
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
MAIN = ROOT / 'src' / 'main.cpp'
QT = Path(r'C:\Qt\5.15.2\msvc2019_64')
GST = Path(r'C:\Program Files\gstreamer\1.0\mingw_x86_64')
OUTPUT_PARENT = ROOT.parent

MARKER = '// OPENKJ_PORTABLE_GSTREAMER_SETUP_V1'
INJECT = '''
#ifdef Q_OS_WIN
    // OPENKJ_PORTABLE_GSTREAMER_SETUP_V1
    // Load GStreamer plug-ins from our self-contained Windows package.
    // The Qt/GStreamer DLLs themselves are installed beside openkj.exe.
    const QString localGstPlugins = QCoreApplication::applicationDirPath() + "/gst-plugins";
    if (QDir(localGstPlugins).exists()) {
        qputenv("GST_PLUGIN_SYSTEM_PATH_1_0", QDir::toNativeSeparators(localGstPlugins).toLocal8Bit());
        const QString localScanner = QCoreApplication::applicationDirPath() + "/gst-plugin-scanner.exe";
        if (QFileInfo::exists(localScanner)) {
            qputenv("GST_PLUGIN_SCANNER", QDir::toNativeSeparators(localScanner).toLocal8Bit());
            qputenv("GST_PLUGIN_SCANNER_1_0", QDir::toNativeSeparators(localScanner).toLocal8Bit());
        }
        const QString localGioModules = QCoreApplication::applicationDirPath() + "/gio-modules";
        if (QDir(localGioModules).exists())
            qputenv("GIO_EXTRA_MODULES", QDir::toNativeSeparators(localGioModules).toLocal8Bit());
    }
#endif
'''


def prepare() -> None:
    if not MAIN.is_file():
        raise RuntimeError('Not in OpenKJ source directory: src\\main.cpp not found.')
    text = MAIN.read_text(encoding='utf-8-sig')
    if MARKER in text:
        print('Portable GStreamer source configuration is already present.')
        return
    anchor = 'QApplication a(argc, argv);'
    if text.count(anchor) != 1:
        raise RuntimeError('Could not safely locate the unique QApplication initialization. No source changed.')
    backup = MAIN.with_name(f'main.cpp.before-portable-{dt.datetime.now():%Y%m%d-%H%M%S}.bak')
    shutil.copy2(MAIN, backup)
    if '#include <QFileInfo>' not in text:
        text = text.replace('#include <QApplication>', '#include <QApplication>\n#include <QFileInfo>', 1)
    text = text.replace(anchor, anchor + '\n' + INJECT, 1)
    MAIN.write_text(text, encoding='utf-8-sig')
    print(f'Configured bundled GStreamer plugin discovery. Source backup: {backup}')


def copy_pattern(directory: Path, pattern: str, target: Path, label: str) -> int:
    files = list(directory.glob(pattern)) if directory.is_dir() else []
    if not files:
        raise RuntimeError(f'Missing {label}: {directory / pattern}')
    target.mkdir(parents=True, exist_ok=True)
    for p in files:
        if p.is_file():
            shutil.copy2(p, target / p.name)
    print(f'Copied {len(files)} {label}.')
    return len(files)


def package() -> None:
    exe = ROOT / 'build' / 'Release' / 'openkj.exe'
    if not exe.is_file():
        raise RuntimeError(f'No Release EXE found: {exe}')
    if not (QT / 'bin' / 'windeployqt.exe').is_file():
        raise RuntimeError(f'Qt deployment tool missing: {QT / "bin" / "windeployqt.exe"}')
    if not (GST / 'bin' / 'libgstreamer-1.0-0.dll').is_file():
        raise RuntimeError(f'GStreamer runtime missing from {GST / "bin"}')
    if MARKER not in MAIN.read_text(encoding='utf-8-sig'):
        raise RuntimeError('Portable source adjustment not yet applied; run --prepare and rebuild first.')

    stamp = dt.datetime.now().strftime('%Y%m%d-%H%M%S')
    dest = OUTPUT_PARENT / f'OpenKJ-Portable-{stamp}'
    dest.mkdir(parents=True)
    shutil.copy2(exe, dest / 'openkj.exe')

    # Include Qt runtime libraries, platform driver, and image/database plugins.
    env = os.environ.copy()
    env['PATH'] = str(QT / 'bin') + os.pathsep + str(GST / 'bin') + os.pathsep + env.get('PATH', '')
    cmd = [str(QT / 'bin' / 'windeployqt.exe'), '--release', '--no-compiler-runtime', '--dir', str(dest), str(dest / 'openkj.exe')]
    print('Running Qt deployment tool ...')
    subprocess.run(cmd, check=True, env=env)

    qtplugins = QT / 'plugins'
    for subdir, name in [('platforms', 'qwindows.dll'), ('sqldrivers', 'qsqlite.dll')]:
        installed = qtplugins / subdir / name
        target = dest / subdir / name
        if not target.is_file():
            if not installed.is_file():
                raise RuntimeError(f'Required Qt plugin not found: {installed}')
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(installed, target)

    # DLLs in the *same directory* as openkj.exe allow double-clicking,
    # with no PATH changes or launcher script on the destination computer.
    copy_pattern(GST / 'bin', '*.dll', dest, 'GStreamer runtime DLLs')
    copy_pattern(GST / 'lib' / 'gstreamer-1.0', '*.dll', dest / 'gst-plugins', 'GStreamer playback plugins')

    scanner_candidates = [
        GST / 'libexec' / 'gstreamer-1.0' / 'gst-plugin-scanner.exe',
        GST / 'lib' / 'gstreamer-1.0' / 'gst-plugin-scanner.exe',
        GST / 'bin' / 'gst-plugin-scanner.exe',
    ]
    scanner = next((p for p in scanner_candidates if p.is_file()), None)
    if scanner is not None:
        shutil.copy2(scanner, dest / 'gst-plugin-scanner.exe')
        print(f'Copied GStreamer plugin scanner from {scanner}.')
    else:
        print('WARNING: gst-plugin-scanner.exe not found; test playback and all media formats carefully.')

    gio_source = GST / 'lib' / 'gio' / 'modules'
    gio_dlls = list(gio_source.glob('*.dll')) if gio_source.is_dir() else []
    if gio_dlls:
        (dest / 'gio-modules').mkdir(exist_ok=True)
        for p in gio_dlls:
            shutil.copy2(p, dest / 'gio-modules' / p.name)
        print(f'Copied {len(gio_dlls)} GIO modules.')

    original_license = ROOT / 'LICENSE'
    if original_license.is_file():
        shutil.copy2(original_license, dest / 'OpenKJ-LICENSE.txt')

    readme = '''OPENKJ PORTABLE WINDOWS BUILD - TEST BEFORE A LIVE SHOW
=======================================================

HOW TO RUN
1. On another 64-bit Windows 10/11 PC, extract the ENTIRE folder.
2. Install Microsoft's official Visual C++ Redistributable (x64) if missing:
   https://aka.ms/vs/17/release/vc_redist.x64.exe
3. Double-click openkj.exe in this folder. Do NOT copy the EXE alone.
4. Allow it through Windows Firewall on PRIVATE networks for phone control.
5. Both devices must be on the same local Wi-Fi network.

This folder contains Qt 5 runtime DLLs/plugins and GStreamer MinGW x64
runtime DLLs/plugins. Neither Visual Studio, Qt SDK nor GStreamer SDK
should be needed on a properly provisioned destination PC.

NOT INCLUDED: Karaoke songs/media, your user-profile OpenKJ database,
profile settings, and third-party Microsoft C++ redistributable installer.
Never hand out your operator remote PIN to singers.

PRODUCTION CHECKS
- Open the app without editing PATH or running a .bat launcher.
- Play CDG+MP3, MP4 and any other media formats you rely on.
- Check the SQLite karaoke library and adding singers/songs.
- Check break-music playback and speaker/audio device selection.
- Check remote phone site, search, request approval, and firewalls.
- Recheck on a DIFFERENT Windows PC without Qt/GStreamer development kits.

REDISTRIBUTION LICENSING
OpenKJ is GPL-3.0-or-later. If you share this modified executable,
you must comply with GPL source-availability and license requirements,
and with licenses/notices applicable to Qt, GStreamer and plugins.
Review included upstream licenses and provide complete corresponding
modified source code alongside a publicly shared binary. This script
does not by itself produce a legally complete redistribution bundle.

Qt deployment: https://doc.qt.io/qt-6/windows-deployment.html
GStreamer deployment: https://gstreamer.freedesktop.org/documentation/deploying/windows.html
'''
    (dest / 'README-FIRST.txt').write_text(readme, encoding='utf-8')
    print('\nPACKAGE CREATED: ' + str(dest))
    print('Test by double-clicking openkj.exe IN THIS NEW FOLDER, not in build\\Release.')
    print('Copy the ENTIRE new folder to the other Windows PC, not just the EXE.')
    print('Recipient may need official vc_redist.x64.exe installed once.')


def main() -> int:
    argp = argparse.ArgumentParser()
    argp.add_argument('--prepare', action='store_true')
    argp.add_argument('--package', action='store_true')
    args = argp.parse_args()
    if args.prepare == args.package:
        argp.error('Choose exactly one of --prepare or --package')
    try:
        if args.prepare:
            prepare()
        else:
            package()
    except (RuntimeError, OSError, subprocess.CalledProcessError) as e:
        print('\nERROR:', e, file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
