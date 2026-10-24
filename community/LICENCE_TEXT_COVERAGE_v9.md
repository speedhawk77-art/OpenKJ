# OpenKJ v9 — resolving missing generic licence texts

**Verification date:** 2026-10-22. **Status:** licence-text source documents gathered; final installed binary package is not yet cleared.

The cleaned Windows installed audit found **118 licence/notice documents**: nine named core documents plus 109 files from the GStreamer Windows SDK. Of the 79 third-party/transitive DLLs, the supplied earlier evidence matrix found **40** with project-specific licence/copyright documents, **33** with only Cerbero `README-LICENSE-INFO.txt` reference files, and **six** without matching GStreamer SDK licence-directory documentation. The six were later shown to be byte-identical to copies from the developer's Qt, GStreamer or Windows SDK and are not mysterious unknown libraries.

## Where Cerbero's missing texts really live

The official, pinned [GStreamer/Cerbero **1.26.9** source tree](https://github.com/GStreamer/cerbero/tree/1.26.9/data/licenses) contains the full standard licences that many of the SDK's `README-LICENSE-INFO.txt` files refer to by filename. The initial evidence collector only copied `share/licenses/**` from the **installed** SDK, so it missed the source tree's `data/licenses/` directory. This was a **collection-scope omission**, not proof the SDK binaries lacked all licence information.

The following **eight verbatim source documents** have now been copied from the official pinned Cerbero tag into this fork:

| Generic legal text | GitHub-stored original from Cerbero 1.26.9 |
| --- | --- |
| Apache 2.0 | [Apache-2.0.txt](licenses/cerbero-1.26.9/Apache-2.0.txt) |
| GNU GPL v2+ | [GPL-2+.txt](licenses/cerbero-1.26.9/GPL-2+.txt) |
| GNU GPL v3+ | [GPL-3+.txt](licenses/cerbero-1.26.9/GPL-3+.txt) |
| GNU Library GPL v2+ | [LGPL-2+.txt](licenses/cerbero-1.26.9/LGPL-2+.txt) |
| GNU Lesser GPL v2.1+ | [LGPL-2.1+.txt](licenses/cerbero-1.26.9/LGPL-2.1+.txt) |
| GNU Lesser GPL v3+ | [LGPL-3+.txt](licenses/cerbero-1.26.9/LGPL-3+.txt) |
| Mozilla Public License 1.1 | [MPL-1.1.txt](licenses/cerbero-1.26.9/MPL-1.1.txt) |
| Mozilla Public License 2.0 | [MPL-2.txt](licenses/cerbero-1.26.9/MPL-2.txt) |

**Important distinction:** LGPL-2+.txt in Cerbero actually contains the older **GNU Library General Public License version 2**; it should not simply be replaced with an LGPLv2.1 text. These original files ensure notices referencing both terms can be fulfilled correctly.

## Other individually identified originals

- Exact [ZBar 0.23.93 COPYING](licenses/zbar-0.23.93/COPYING) and [LGPLv2.1 text](licenses/zbar-0.23.93/LICENSE.md) copied directly from upstream `mchehab/zbar` tag `0.23.93`. This confirms ZBar's **LGPL-2.1-or-later** library notice, even though the Cerbero recipe omitted an explicit `licenses =` declaration.
- Qt 5.15.2 ANGLE third-party notices and original attribution metadata are preserved verbatim under [`licenses/qt-5.15.2/angle/`](licenses/qt-5.15.2/angle/): ANGLE, Khronos, Chromium and Apple notices.
- Original Qt 5.15.2 GPL/LGPL texts and GStreamer 1.26.9 LGPLv2.1 are preserved under [`licenses/qt-5.15.2/`](licenses/qt-5.15.2/) and [`licenses/gstreamer-1.26.9/`](licenses/gstreamer-1.26.9/).

## What still blocks publishing

1. These eight newly collected texts and Qt/ZBar documents are in the **GitHub source branch**, **not** automatically embedded in the already-built `OpenKJ-v9-Cleaned-Setup.exe`. The installed candidate has 118 documents; it must be updated/rebuilt and tested to include all necessary full licence texts and specific notices.
2. Generic licence texts do not replace the actual **copyright/third-party notices** for individual components or demonstrate that all LGPL-covered library corresponding sources and patches have been offered.
3. The official Cerbero 1.26.9 recipe metadata now documents **56 source families for 74 shipped GStreamer/MinGW DLLs**, but the exact binary-build provenance and complete sources for several LGPL libraries (including GLib and the MinGW runtime components) remain unproven.
4. The embedded `vc_redist.x64.exe` couldn't be extracted by the tested Linux Inno 7 utilities. The two Visual Studio 2026 development-machine candidates are byte-identical and Microsoft-signed, but they do not establish the embedded file's SHA-256.
5. Evaluate specific codec distribution/patent conditions as applicable (especially OpenH264), and preserve applicable source/copyright notices.

**Publication decision: HOLD.** This record completes sourcing of the **standard licence texts** for the SDK README references, but not installed-notice completeness or broader redistributor compliance. See [readiness checklist](RELEASE_READINESS_CHECKLIST_v9.md).

## Final remote engineering follow-up — 2026-10-24

The [completion report](REMOTE_COMPLETION_FOLLOWUP_2026-10-24.md) supersedes earlier pending engineering statements for the checks explicitly proved there: optional end-user codec controls, both application executables built from recorded source, direct official SDK byte matches, expanded Qt/Rust notices and source mirrors, and the source-built installer test. It records the remaining evidentiary/licence-use limits separately. The Windows release remains draft-only for physical karaoke testing.
