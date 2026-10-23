# OpenKJ v9 — Public Windows release readiness checklist

**Release status: ON HOLD — GitHub draft ID 421204266.** The cleaned installer works on a second Windows computer, but evidence for some redistributed DLLs and source obligations is incomplete. This is a practical compliance tracker, **not legal certification**.

Mark each box `[x]` only once a concrete file, hash, source archive/build recipe, rights document or installation test proves the requirement. Keep review evidence alongside this file or linked to it.

- [ ] **1. Audit the actual cleaned *installed* DLLs.** Re-inventory the installed version, not the earlier portable bundle; verify 196 GStreamer plugins and removed plug-ins absent. For each bundled third-party DLL, record version, supplier, actual licence, third-party notices and package/source provenance. Resolve unknowns by verified evidence or removal and retesting. The prior report counted 79 candidate transitive DLLs before the installed version was rechecked.
- [ ] **2. Complete LGPL and GPL corresponding-source delivery.** Six upstream Qt 5.15.2 and GStreamer 1.26.9 source archives are mirrored with checksums in the GitHub draft; these do NOT yet cover exact GStreamer/Cerbero build recipes, patches, all transitive LGPL libraries (e.g. GLib, libass where applicable), or prove the shipped binaries match. Link the OpenKJ source to the actual executable build commit. Supply the missing corresponding source or valid offer under distributor control and user-replacement / relinking instructions.
- [ ] **3. Check Microsoft and codec redistribution permission.** Identify Microsoft Visual C++ runtime/Microsoft DLLs actually inside the installed build and match them to allowed redistributables under the user's Visual Studio license; keep licence evidence but never publish serials or account information. Check codecs (e.g. if OpenH264 is bundled) for any separate distributor/patent conditions; replace unsupported binaries and retest.
- [ ] **4. Confirm the installed notices.** Check the actual installer installs OpenKJ GPLv3, Qt LGPLv3, GStreamer LGPLv2.1 (where applicable), QR MIT/Apache texts and each installed third-party copyright/licence. Resolve missing umbrella licence texts referenced by SDK README documents. Include visible source availability and library replacement instructions.
- [ ] **5. Rebuild, hash, retest and review release.** If DLLs or notices change, rebuild the installer, install and test it on a second machine, compute the new SHA-256 and update the GitHub *draft* asset + manifest. Verify the release remains draft, body links exact sources and checksum, and do not make it public until the foregoing requirements are supported by the evidence.

## Current known evidence

- [Licensing review](LICENSING_REVIEW_v9.md) — early inventory, cleaned installer test, upstream source mirror.
- [Transitive DLL audit](TRANSITIVE_DLL_AUDIT_v9.md) — 79 initial candidate DLLs need provenance.
- [Source availability](SOURCE_DISTRIBUTION_STATUS_v9.md) — mirrored official source archives, remaining source gaps.
- [DLL replacement guide](WINDOWS_DLL_REPLACEMENT_GUIDE_v9.md) — replacement instructions, provisional until exact installed layout is verified.
- [Qt LGPL requirements](https://www.qt.io/development/open-source-lgpl-obligations)
- [Microsoft Visual C++ redistribution requirements](https://learn.microsoft.com/en-us/cpp/windows/redistributing-visual-cpp-files)
- [GStreamer Cerbero build details](https://gstreamer.freedesktop.org/documentation/installing/building-from-source-using-cerbero.html)

**No public release should be published merely because boxes in this checklist are checked.** Actual licence obligations must be met, and potentially a qualified legal review if uncertainty remains.
