# OpenKJ v9 — Public Windows release readiness checklist

**Release status: ON HOLD — GitHub draft ID 421204266.** The cleaned installer works on a second Windows computer, but evidence for some redistributed DLLs and source obligations is incomplete. This is a practical compliance tracker, **not legal certification**.

Mark each box `[x]` only once a concrete file, hash, source archive/build recipe, rights document or installation test proves the requirement. Keep review evidence alongside this file or linked to it.

- [x] **1A. Verify the actual cleaned *installed* binary inventory.** Completed: 340/340 original application-file SHA-256 matches; 196 GStreamer plugins; the 36 removed plugins absent; no missing/unexpected runtime files; one expected generated Inno uninstaller. [Evidence and independent comparison](INSTALLED_BINARY_AUDIT_RESULT_v9.md).
- [x] **1B-i. Identify the six DLL families from primary-source licensing documentation.** Qt SVG, Qt ANGLE, Qt software OpenGL, Windows SDK D3D compiler and likely GNU gettext/proxy-libintl have credible official source/license leads. This is a **component-family research subcheck**, NOT a determination that these exact DLL bytes came from those packages. [Review](SIX_DLL_ORIGIN_REVIEW_v9.md).
- [ ] **1B. Verify exact DLL licensing and provenance.** The cleaned installed bundle still has **79** transitive dependencies whose build-source/redistribution terms need documented review: 40 have a component notice in the SDK evidence, 33 only have package-reference README files, and six have no matched SDK component notice. Identify real supplier, exact library source/build and licence for each; resolve missing/conflicting components or remove and retest. [Installed audit result](INSTALLED_BINARY_AUDIT_RESULT_v9.md).
- [ ] **2. Complete LGPL and GPL corresponding-source delivery.** Six upstream Qt 5.15.2 and GStreamer 1.26.9 source archives are mirrored with checksums in the GitHub draft; these do NOT yet cover exact GStreamer/Cerbero build recipes, patches, all transitive LGPL libraries (e.g. GLib, libass where applicable), or prove the shipped binaries match. Link the OpenKJ source to the actual executable build commit. Supply the missing corresponding source or valid offer under distributor control and user-replacement / relinking instructions.
- [x] **3A. Identify embedded Microsoft redist and its installer behavior.** The builder embeds a separate `vc_redist.x64.exe` and executes it during setup; the installed-file audit omits this temporary installer by design. The exact chosen redist binary and rights must still be verified. [Source/rights review](SIX_DLL_ORIGIN_REVIEW_v9.md).
- [ ] **3B. Verify Microsoft and codec redistribution permission.** Identify Microsoft Visual C++ runtime/Microsoft DLLs actually inside the installed build and match them to allowed redistributables under the user's Visual Studio license; keep licence evidence but never publish serials or account information. Check codecs (e.g. if OpenH264 is bundled) for any separate distributor/patent conditions; replace unsupported binaries and retest.
- [x] **4A. Check that staged core notices are present in the installed program.** Verified 118 documents, including all nine expected core notice files; this is a presence-only check. [Evidence](INSTALLED_BINARY_AUDIT_RESULT_v9.md).
- [ ] **4B. Confirm that all installed notices are legally complete.** Check the actual installer installs OpenKJ GPLv3, Qt LGPLv3, GStreamer LGPLv2.1 (where applicable), QR MIT/Apache texts and each installed third-party copyright/licence. Resolve missing umbrella licence texts referenced by SDK README documents. Include visible source availability and library replacement instructions.
- [x] **5A. Build and test the current cleaned installer.** Already built, SHA-256 verified on GitHub and installed/tested on a second computer; the installed runtime matches its expected cleaned reference. [Installed audit](INSTALLED_BINARY_AUDIT_RESULT_v9.md).
- [ ] **5B. Final release verification after any compliance fixes.** If DLLs or notices change, rebuild the installer, install and test it on a second machine, compute the new SHA-256 and update the GitHub *draft* asset + manifest. Verify the release remains draft, body links exact sources and checksum, and do not make it public until the foregoing requirements are supported by the evidence.

## Current known evidence

- [Installed cleaned-binary hash audit](INSTALLED_BINARY_AUDIT_RESULT_v9.md) — 340/340 hashes verified, 196 plugins, installed licence presence.
- [Licensing review](LICENSING_REVIEW_v9.md) — early inventory, cleaned installer test, upstream source mirror.
- [Transitive DLL audit](TRANSITIVE_DLL_AUDIT_v9.md) — 79 initial candidate DLLs need provenance.
- [Source availability](SOURCE_DISTRIBUTION_STATUS_v9.md) — mirrored official source archives, remaining source gaps.
- [DLL replacement guide](WINDOWS_DLL_REPLACEMENT_GUIDE_v9.md) — replacement instructions, provisional until exact installed layout is verified.
- [Qt LGPL requirements](https://www.qt.io/development/open-source-lgpl-obligations)
- [Microsoft Visual C++ redistribution requirements](https://learn.microsoft.com/en-us/cpp/windows/redistributing-visual-cpp-files)
- [GStreamer Cerbero build details](https://gstreamer.freedesktop.org/documentation/installing/building-from-source-using-cerbero.html)

**No public release should be published merely because boxes in this checklist are checked.** Actual licence obligations must be met, and potentially a qualified legal review if uncertainty remains.
