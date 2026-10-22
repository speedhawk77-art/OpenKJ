# Licensing and third-party components

This document identifies known components. **It is not a claim that the runtime bundle's complete licensing inventory has been audited.** Review the exact DLLs/plugins included in any public Windows installer and include their applicable licenses and source availability as required before publishing its binary.

- **OpenKJ:** GNU General Public License version 3. Full license text in repository root `LICENSE`. Upstream: https://github.com/OpenKJ/OpenKJ
- **QR encoder embedded in the remote UI:** `qrcode-terminal` / Kazuhiko Arase QRCode code, with Apache-2.0 and MIT licences. Apache-2.0 and original attribution: `community/licenses/QR-Encoder-Notices.txt`; full MIT licence text: `community/licenses/QRCode-MIT.txt`. These notices must also accompany the distributed binary.
- **Qt 5:** Toolkit available under LGPL/GPL/commercial terms, depending on components and acquisition. Review the dynamically linked Qt DLLs and Qt plugins shipped and include relevant licence notices and compliance information. https://www.qt.io/licensing
- **GStreamer and plugins:** Different components/plugins can have different licenses, including LGPL or GPL. Review the particular DLLs selected for the Windows installer, not just GStreamer core. https://gstreamer.freedesktop.org/documentation/
- **Microsoft Visual C++ redistributable:** subject to Microsoft's redistributable licensing terms. https://learn.microsoft.com/en-us/cpp/windows/redistributing-visual-cpp-files
- **Other transitive dependencies:** inspect the actual portable bundle and the upstream OpenKJ license/readme files for further third-party components and notices.

Modified OpenKJ source code is in this repository on the `offline-remote-v9` branch; builds distributed from it should point to the corresponding tag/commit, with their exact changes retained.
