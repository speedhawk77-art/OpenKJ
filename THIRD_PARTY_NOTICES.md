# Licensing and third-party components

This document identifies known components. **It is not a claim that the runtime bundle's complete licensing inventory has been audited.** Review the exact DLLs/plugins included in any public Windows installer and include their applicable licenses and source availability as required before publishing its binary.

- **OpenKJ:** GNU General Public License version 3. Full license text in repository root `LICENSE`. Upstream: https://github.com/OpenKJ/OpenKJ
- **Application-bundled TagLib, spdlog and fmt:** The repository contains TagLib 1.11.1 sources and a pinned spdlog 1.9.2 submodule (with fmt 8.0.1); the alternative CPM path names spdlog 1.8.2/fmt 7.1.3. Exact MIT notices and original vendored TagLib source headers are preserved under `community/licenses/`; see `Application-Dependency-Provenance.md`. The original draft executable's build choices remain unattested. The separate GStreamer-side TagLib DLL maps to 1.13.1.
- **QR encoder embedded in the remote UI:** `qrcode-terminal` / Kazuhiko Arase QRCode code, with Apache-2.0 and MIT licences. Apache-2.0 and original attribution: `community/licenses/QR-Encoder-Notices.txt`; full MIT licence text: `community/licenses/QRCode-MIT.txt`. These notices must also accompany the distributed binary.
- **Qt 5:** Toolkit available under LGPL/GPL/commercial terms, depending on components and acquisition. Review the dynamically linked Qt DLLs and Qt plugins shipped and include relevant licence notices and compliance information. https://www.qt.io/licensing
- **GStreamer and plugins:** Different components/plugins can have different licenses, including LGPL or GPL. Review the particular DLLs selected for the Windows installer, not just GStreamer core. https://gstreamer.freedesktop.org/documentation/
- **Microsoft Visual C++ redistributable:** subject to Microsoft's redistributable licensing terms. https://learn.microsoft.com/en-us/cpp/windows/redistributing-visual-cpp-files
- **Other transitive dependencies:** inspect the actual portable bundle and the upstream OpenKJ license/readme files for further third-party components and notices.

Modified OpenKJ source code is in this repository on the `offline-remote-v9` branch; builds distributed from it should point to the corresponding tag/commit, with their exact changes retained.


### Verified source licence texts added during the v9 audit

To accompany the public **source** branch, the original Qt 5.15.2 terms are copied from `qt/qtbase` tag `v5.15.2` under `community/licenses/qt-5.15.2/` (`LICENSE.LGPLv3`, `LICENSE.LGPL3`, `LICENSE.GPL3`). The original LGPL-2.1 terms for GStreamer 1.26.9 are copied from `GStreamer/gstreamer` tag `1.26.9` under `community/licenses/gstreamer-1.26.9/COPYING-LGPL-2.1`. These texts do **not** certify that each Windows library is LGPL-licensed or that the current installer contains these files.

The GStreamer SDK's component-specific `share/licenses/` documents have also been inventoried in the local licence evidence archive. The review found 109 documents from 66 component directories, but some referenced umbrella texts were not collected and no Qt installation documents appeared in that evidence archive. See `community/LICENSING_REVIEW_v9.md`. The Windows installer must remain **draft** pending the binary-specific dependency review, corresponding-source arrangements and final installed-bundle verification.


### Windows v9: recovered Cerbero generic licence documents (not yet in existing installer)

Review of the tested Windows **cleaned** binary inventory identified 33 dependency DLLs with only GStreamer SDK package README licence references in the installed notice collection. The source of the missing generic texts was found in official GStreamer/Cerbero tag `1.26.9` under `data/licenses/`. Eight original GNU GPL/LGPL, Apache and MPL full texts have been preserved unchanged in this source fork at `community/licenses/cerbero-1.26.9/`.

The original copyright statement and LGPL2.1 notice for ZBar 0.23.93 are under `community/licenses/zbar-0.23.93/`; original Qt 5.15.2 ANGLE, Khronos, Apple and Chromium third-party notices are under `community/licenses/qt-5.15.2/angle/`. [Full source evidence and outstanding packager actions](community/LICENCE_TEXT_COVERAGE_v9.md).

**The already-built draft EXE has not been updated with these additional notices.** This source-repository change does not alone fulfill the installed distribution's corresponding source, attributions, Microsoft/runtime or codec obligations. The release remains DRAFT until remaining gaps are resolved.
