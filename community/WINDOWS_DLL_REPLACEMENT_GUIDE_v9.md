# Replacing LGPL libraries in OpenKJ Offline Remote v9 — Windows x64

**Work-in-progress installation and relinking guidance.** This document is for users of the Windows **cleaned candidate**. It does not establish that the candidate has been fully license-cleared or that replacement libraries are ABI compatible.

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

- Application source is public: [OpenKJ branch `offline-remote-v9`](https://github.com/speedhawk77-art/OpenKJ/tree/offline-remote-v9). The current candidate manifest is **not** an exact binary build attestation.
- Qt 5.15.2, GStreamer 1.26.9 and five related upstream source archives have been checksum-verified and mirrored as **draft assets** under release 421204266. They become publicly accessible only if and when the release is published.
- For missing exact-library sources and build instructions, review `community/SOURCE_DISTRIBUTION_STATUS_v9.md`; these obligations **remain outstanding** for the candidate.

**Do not redistribute the binary publicly yet:** the additional LGPL transitive-library source, patch/build provenance, and other component licences must be addressed first. This guide is not a substitute for complying with actual LGPLv2.1/v3 and GPLv3 requirements.

Relevant original terms: `community/licenses/qt-5.15.2/`, `community/licenses/gstreamer-1.26.9/`, `LICENSE`, `THIRD_PARTY_NOTICES.md`.
