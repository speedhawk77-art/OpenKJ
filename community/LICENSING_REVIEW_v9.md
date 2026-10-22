# Offline Remote v9 — Windows binary licensing review (2026-10-22)

**Status: Review in progress; Windows installer is held as a GitHub draft and is NOT yet cleared for public redistribution.** This is a technical check, not legal advice.

A local inventory of the tested 64-bit Windows portable runtime counted **376 executables/DLLs**, including **232 GStreamer plug-ins**. The corrected `gst-inspect-1.0` report showed:

| GStreamer plug-in reported licence | Count |
| --- | ---: |
| LGPL | 183 |
| MPL / MPL-2.0 | 31 |
| MIT/X11 | 10 |
| BSD | 3 |
| Proprietary | 1 |
| Unresolved or unknown | 4 |

The `libgstelevenlabs.dll` plug-in reports **Proprietary**; `libgstx265.dll` failed inspection and may have GPLv2/copyleft issues through x265; two AMR plug-ins also failed inspection, and `libgstrtponvif.dll` reported an unknown licence. These five require exclusion or specific verification.

An experimental **separate, reversible test candidate** excludes those five plus 31 optional MPL plug-ins to simplify review. Removal is not proof of full compatibility and may affect some media formats. **The user's known-good v9 portable folder and draft release are unchanged.**

The original portable folder was missing Qt/GStreamer LGPL licence texts, the QR-code MIT licence text, and complete redistribution notices. The candidate preparation package provides core texts, but **copyright notices, source code availability, actual licences of 79 transitive dependencies, and build/installer verification remain open**.

Before making the EXE public:
1. Test MP3+CDG, MP4/video, break music, offline controls, and song requests from the reduced plug-in folder.
2. Identify exact bundled Qt 5.15.2 and GStreamer 1.26.9 sources/build modifications, and make the corresponding LGPL-covered sources and relinking information available under the distributor's control or through a compliant written offer.
3. Audit the remaining Windows DLL dependencies/codecs, include component-specific licences/copyright notices, and check redistribution or patent conditions where relevant.
4. Confirm Microsoft runtime redistribution rights and publish a new installer that includes notices; re-test installed files.
5. Keep the published GPL-3.0 application source tied to the exact release build.

References: [Qt LGPL obligations](https://www.qt.io/development/open-source-lgpl-obligations), [GStreamer licensing](https://gstreamer.freedesktop.org/documentation/frequently-asked-questions/licensing.html), [existing third-party notices](../../THIRD_PARTY_NOTICES.md).

**Do not mark the existing v9 draft installer as a public release on the basis of this inventory alone.**

## Cleaned runtime test result — 2026-10-22

The community tester reports that the separately staged `OpenKJ-Distribution-Test-v9` build **works** after the 36 optional/review-required GStreamer plugins were removed. The cleaned candidate therefore retains **340 audited runtime binaries**, including **196 GStreamer plugins** (183 reported LGPL, 10 MIT/X11 and 3 BSD). This is a functional smoke test, not a comprehensive media-format certification or a licence clearance.

**Remaining blockers before publishing a binary:** The 79 bundled transitive DLL dependencies need their component-specific licence/copyright and redistribution/source obligations reviewed; corresponding Qt 5.15.2 and GStreamer 1.26.9 library and dependency sources or an adequate written offer must be arranged under the distributor's control; required licence/source notices must ship with the *installed* files; the cleaned installer must be rebuilt and tested separately. Do not replace or publish the existing v9 draft release yet.
