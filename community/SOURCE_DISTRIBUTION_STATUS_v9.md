# OpenKJ Offline Remote v9 — corresponding source distribution status

**Current status: source mirroring is preparatory work, NOT release clearance.** Draft release ID: `421204266`; candidate filename: `OpenKJ-v9-Cleaned-Setup.exe`; SHA-256 `bc35c24ef25529b86b90bd3f5a215603de3b283d590620ec58820e02f83421ef`.

## Verified upstream source locations

| Component family | Source archive/version | Provenance |
| --- | --- | --- |
| Qt 5.15.2 | `qt-everywhere-src-5.15.2.tar.xz` | [Qt project archive](https://download.qt.io/archive/qt/5.15/5.15.2/single/), published SHA-256 `3a530d1b243b5dec00bc54937455471aaa3e56849d2593edb8ded07228202240` |
| GStreamer core 1.26.9 | `gstreamer-1.26.9.tar.xz` | [GStreamer official sources](https://gstreamer.freedesktop.org/src/gstreamer/) |
| GStreamer base 1.26.9 | `gst-plugins-base-1.26.9.tar.xz` | [Official plugin sources](https://gstreamer.freedesktop.org/src/gst-plugins-base/) |
| GStreamer good 1.26.9 | `gst-plugins-good-1.26.9.tar.xz` | [Official plugin sources](https://gstreamer.freedesktop.org/src/gst-plugins-good/) |
| GStreamer bad 1.26.9 | `gst-plugins-bad-1.26.9.tar.xz` | [Official plugin sources](https://gstreamer.freedesktop.org/src/gst-plugins-bad/) |
| GStreamer RTSP server 1.26.9 | `gst-rtsp-server-1.26.9.tar.xz` | [Official RTSP server sources](https://gstreamer.freedesktop.org/src/gst-rtsp-server/) |

The workflow `.github/workflows/stage-openkj-v9-upstream-sources.yml` is restricted to **draft-only** uploads to release 421204266. It downloads official archives, checks the published Qt SHA-256 and GStreamer-provided SHA-256 sums, checks GitHub release asset SHA-256 digests, and never sets `draft=false`. This is source **mirroring**, not attestation of exact binary correspondence.

## Unresolved release blockers

1. **Exact binary/source matching:** confirm Qt DLLs really correspond to unmodified upstream 5.15.2, and identify the precise official GStreamer SDK/Cerbero build commit, all patches, build options, and the upstream versions of every included binary library.
2. **Transitive LGPL components and their source:** the bundled Windows SDK includes GLib, Pango, libass, MinGW runtimes, and other LGPL-covered libraries. A mirror of GStreamer main modules does not provide their complete corresponding source. See `TRANSITIVE_DLL_AUDIT_v9.md`.
3. **Unresolved third-party binaries:** `d3dcompiler_47.dll` (Microsoft distribution terms), `libintl-8.dll` (source and licence), and Qt graphics modules (`libEGL.dll`, `libGLESv2.dll`, `opengl32sw.dll`, `iconengines/qsvgicon.dll`) require package-origin and licencing verification. Cisco OpenH264 licence is in the evidence but any H.264 patent or Cisco-binary specific conditions also need checking.
4. **Corresponding-source delivery and user modifications:** make all LGPL-covered library sources, modifications and installation/relink information available via a sufficiently durable distribution mechanism or compliant written offer. GPLv3 application source must correspond to the shipped build; the candidate manifest currently records a review-branch commit **not** an exact build commit attestation.
5. **Installer verification:** inspect actual installed `licenses/` contents and DLL imports; confirm the 36 removed plugins are absent, and document any remaining missing umbrella terms in 71 reference README files.
6. **Microsoft Visual C++ runtime:** confirm licensed VS redistribution rights and that the correct unmodified redistributable was used. Microsoft's [Visual Studio 2026 redistributable list](https://learn.microsoft.com/en-us/visualstudio/releases/2026/redistribution) and [Visual C++ redistribution requirements](https://learn.microsoft.com/en-us/cpp/windows/redistributing-visual-cpp-files) are the applicable starting points.

For the proprietary Windows installer, **do not publish before these items are verified or remediated**. The OpenKJ application source is already publicly available under the upstream GPL licence on branch `offline-remote-v9`.

For GPL/LGPL guidance, see [Qt's official LGPL obligations](https://www.qt.io/development/open-source-lgpl-obligations) and [GStreamer's licensing advisory](https://gstreamer.freedesktop.org/documentation/plugin-development/appendix/licensing-advisory.html). This document is a technical compliance tracker, not formal legal advice.
