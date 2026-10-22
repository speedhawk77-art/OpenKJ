# Windows v9 transitive DLL licence audit — evidence assessment

**Status: incomplete, NOT approval to publish a binary.** Reviewed 2026-10-22.

The v9 Windows runtime inventory includes **79 transitive/third-party DLLs** beyond the OpenKJ, Qt and GStreamer core classifications. We compared their **filenames** against the 109 license-related documents uploaded from the installed GStreamer 1.26.9 SDK. Component associations are **heuristics**, not verified build provenance.

| Evidence in the supplied archive | DLL count |
| --- | ---: |
| Component-specific licence/copyright file found | 40 |
| Only a GStreamer/Cerbero license-reference README found | 33 |
| No matching component document or uncertain supplier | 6 |
| **Total** | **79** |

A notice file in the archive does **not** prove the exact compiled DLL came from that source package, meets its licence, or that complete corresponding LGPL source code is available.

### Unresolved distribution requirements

1. Verify the actual package/build provenance, copyrights and redistribution licences of **every shipped DLL**, including the six without matching documents. A license-reference README citing an umbrella licence is not itself the complete license.
2. Make complete corresponding source code, including patches, for Qt 5.15.2 and GStreamer 1.26.9 libraries and other LGPL-covered components available **under distributor control** or via a legally sufficient written offer. An upstream link alone may not satisfy Qt's stated requirements. See https://www.qt.io/development/open-source-lgpl-obligations.
3. Preserve the ability to replace and relink Qt/GStreamer LGPL DLLs and provide installation/relink instructions. No licence-restrictive EULA or DRM.
4. Include in the **installed** distribution all matching component notices, Qt/GStreamer/OpenKJ GPL/LGPL terms, QR-code MIT and Apache notices, and exact corresponding source/patch availability statements.
5. Check Microsoft Visual C++ redistributable distribution entitlement from the actual licensed Visual Studio installation (https://learn.microsoft.com/en-us/cpp/windows/redistributing-visual-cpp-files), plus relevant codec patent/redistribution terms. Do not presume that libraries are cleared solely because they are open source.
6. Verify the cleaned single-file installer really excludes the 36 removed GStreamer plugins, includes the staged notices, has a documented checksum and source revision, and works in an installed environment.

### Functional status, distinct from licence clearance

The community tester reported that the **separately built cleaned v9 installer works on a second Windows computer**. The older original v9 binary attached to the GitHub **draft release** is not the cleaned file; it should not be publicly published.

### Source evidence

- Local user-supplied `OpenKJ-LICENSE-INVENTORY(1).csv` (file checksums, filenames), `OPENKJ-LICENSE-EVIDENCE-REPORT.md` and `OPENKJ-DEPENDENCY-LICENSE-DOCUMENTS.zip` (109 notice documents).
- Complete application source: `offline-remote-v9` branch.
- Existing record: `community/LICENSING_REVIEW_v9.md`; `THIRD_PARTY_NOTICES.md`.
- GStreamer official explanation: https://gstreamer.freedesktop.org/documentation/frequently-asked-questions/licensing.html

**Publication decision: HOLD.** A draft-only upload of the separately tested cleaned installer is allowed for review; do not switch the release to public before the source and licence requirements above are resolved.