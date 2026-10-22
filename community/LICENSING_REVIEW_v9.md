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


## Collected dependency-licence evidence — 2026-10-22

The developer supplied `OPENKJ-LICENSE-EVIDENCE-REPORT.md` and `OPENKJ-DEPENDENCY-LICENSE-DOCUMENTS.zip` for the installed Qt 5.15.2 MSVC2019 x64 and GStreamer 1.26.9 MinGW x64 libraries. The evidence archive contains **109 documents, all under GStreamer paths**, across 66 component directories: **38 non-README licence/copyright documents** and **71 README-LICENSE-INFO.txt** reference files. The archive contains **no Qt documents**, even though Qt was installed.

Several GStreamer README reference files cite umbrella licence texts (for example LGPL-2+.txt, LGPL-2.1+.txt and Apache-2.0.txt), which **were not present in the collected archive**. A README indicating the applicable licence does not substitute for its full terms. The collected documents are useful evidence but not a comprehensive bill of materials for all 79 transitive libraries. In particular, the evidence does not demonstrate which precise Windows DLL build is covered by which licence/source package.

The following **upstream source licence files** have now been copied verbatim to this community fork, with version/commit provenance from their repositories:

- `community/licenses/qt-5.15.2/LICENSE.LGPLv3` — `qt/qtbase` tag `v5.15.2`.
- `community/licenses/qt-5.15.2/LICENSE.LGPL3` — same Qt source tag.
- `community/licenses/qt-5.15.2/LICENSE.GPL3` — same Qt source tag.
- `community/licenses/gstreamer-1.26.9/COPYING-LGPL-2.1` — `GStreamer/gstreamer` tag `1.26.9`.

These GitHub files are *documentation*, not proof the currently drafted installer includes them. The collected GStreamer component notices also need to accompany the final binary installer and be mapped to its contents. Other dependencies and codec patent/distribution conditions remain to be audited. LGPL-compliant corresponding source availability and end-user ability to replace the dynamically linked libraries are still required. **Do not publish the existing v9 draft installer.**

Collected evidence is not reproduced in full on GitHub yet; the review was conducted against the files provided by the community tester. No personal karaoke data was collected.

## Cleaned single-file Windows installer candidate built — 2026-10-22

The community tester reports that `BUILD_CLEAN_OPENKJ_INSTALLER.bat` completed with **SUCCESS** after using the working reduced-plugin distribution folder with staged licence notices. This confirms the installer builder ran successfully on the development laptop; **the built installer has not yet been independently inspected or installation-tested**. The existing GitHub draft release still contains the earlier `OpenKJ-v9-Setup.exe` and checksum rather than a verified cleaned replacement.

Next checks: install the cleaned EXE on another machine (or in a controlled clean test environment), validate that the installed `licenses` directory and executable/plugin inventory match the cleaned staging folder, confirm playback and offline controls, determine precise third-party dependency licences and corresponding source delivery arrangements, and only then replace the draft asset / consider public release.

## Cleaned Windows installer tested on second computer — 2026-10-22

The community tester confirms that the rebuilt `OpenKJ-v9-Cleaned-Setup.exe` **works on a second Windows computer** after successful creation with the cleaned runtime and staged licence-notice folder. This is a positive installation/functional smoke test. A separately documented file-by-file check of the installed licence directory and complete binary inventory has not yet been provided, so the statement does not certify package contents or legal compliance.

**Distribution remains on hold:** the remaining transitive DLL copyright/licence conditions and corresponding LGPL library source availability/relinking requirements must be resolved. The existing GitHub draft still contains the original (uncleaned) `OpenKJ-v9-Setup.exe`, not the new installer. Do not publish that draft as-is.

## GitHub draft assets successfully replaced — 2026-10-22

GitHub API confirms release **421204266** is still **draft=true** and has three cleaned candidate assets: `OpenKJ-v9-Cleaned-Setup.exe`, `OpenKJ-v9-Cleaned-Setup.sha256.txt`, and `OpenKJ-v9-Cleaned-Manifest.json`. The cleaned EXE has **205,876,054 bytes** and GitHub-recorded SHA-256 **`bc35c24ef25529b86b90bd3f5a215603de3b283d590620ec58820e02f83421ef`**. The old uncleaned draft assets were removed. GitHub assigned a temporary `untagged-*` draft identifier during the release metadata PATCH, causing the upload helper's excessively strict final tag check to print an error *after* successful replacement. This does not mean the upload failed. **The Windows binary is still not licensed/cleared for public publication.** See `community/DRAFT_CLEANED_INSTALLER_STAGING_v9.md` for details.
