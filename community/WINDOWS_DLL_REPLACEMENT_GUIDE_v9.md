# Replacing LGPL libraries in OpenKJ Offline Remote v9 — Windows x64

This guide applies to the source-built Windows x64 candidate from application commit `016d5c3135bbc8073b96881e49e19feb97359101` and documentation-only refreshes with the same 339 program hashes. Library replacement still requires compatible interfaces.

OpenKJ itself is distributed under GPL-3.0. The cleaned Windows build uses dynamically linked Qt 5.15.2 DLLs (various module licences, often LGPLv3) and GStreamer 1.26.9 DLLs (core LGPLv2.1 and plugin-specific terms). This project does not impose a restriction on modifying, reverse engineering for debugging such modifications, or replacing LGPL-covered libraries. Official terms control if this document is inconsistent.

## Locating dynamically linked libraries

1. Exit OpenKJ completely, including the offline phone-remote launcher.
2. Locate your chosen install directory. For the cleaned candidate's Inno Setup defaults this is normally `C:\Program Files\OpenKJ Custom\` (or the folder selected during setup).
3. Qt main DLLs such as `Qt5Core.dll` and Qt plugins under `platforms\`, `sqldrivers\` and other subdirectories are in the application folder.
4. GStreamer DLLs such as `libgstreamer-1.0-0.dll` are alongside the application; GStreamer plugins are under `gst-plugins\`.
5. The `licenses\` subdirectory contains notices for the candidate as initially packaged. The **GitHub draft source archives do not replace notices installed on an end-user machine**.

## Replacing a library

1. Make a backup copy of the installation or at least the libraries and plugins being replaced.
2. Obtain or build a 64-bit DLL compatible with the relevant ABI, module name and other dependencies. Qt 5 and GStreamer 1.26 components have interdependent versions; a newer or modified DLL is not necessarily drop-in compatible.
3. With OpenKJ stopped, replace the target DLL and any required matching dependencies. Administrator privileges may be required under `Program Files`.
4. Restart OpenKJ and verify playback and the offline remote. If the application fails, restore the backup.

OpenKJ does not intentionally lock the runtime to specific library hashes or prevent compatible DLL substitution. However, Windows execution policies, antivirus, operating-system protections and ABI incompatibilities can prevent modified DLLs from running; this is **not** a guarantee that any altered DLL will work. No DRM or EULA of this community project restricts your LGPL rights.

## Building and obtaining source

The exact application and launcher source is [commit 016d5c3](https://github.com/speedhawk77-art/OpenKJ/tree/016d5c3135bbc8073b96881e49e19feb97359101). The standalone application source archive includes tracked submodule source and build files. Its source-build workflow records the Qt 5.15.2/MSVC 14.44 environment and commands. Bundled TagLib source is part of that archive; rebuilding the application builds that source as specified by its build files.

See `SOURCE_ACCESS_v9.md` beside this guide for library source archives, Cargo lockfiles, Cerbero recipes/patches and build instructions. These sources must be supplied alongside a public installer. A private draft does not provide public source access. Original component terms are installed under `licenses/`; Microsoft terms apply only to Microsoft components.

Relevant original terms: `community/licenses/qt-5.15.2/`, `community/licenses/gstreamer-1.26.9/`, `LICENSE`, `THIRD_PARTY_NOTICES.md`.
