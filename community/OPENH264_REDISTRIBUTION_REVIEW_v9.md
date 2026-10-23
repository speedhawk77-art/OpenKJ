# OpenKJ Offline Remote v9 — OpenH264 and Microsoft redistribution review

**Reviewed 2026-10-23. Status: a specific H.264 patent-distribution condition remains unresolved. The Windows GitHub release is DRAFT.**

## Installed OpenH264 is a separate distribution question

The SHA-256-reviewed cleaned Windows runtime contains `libopenh264-7.dll` from the GStreamer 1.26.9 Windows SDK and `gst-plugins/libgstopenh264.dll` (the GStreamer plug-in reports a BSD licence). The official GStreamer/Cerbero 1.26.9 `openh264.recipe` names OpenH264 **2.6.0** and a BSD software licence, with source checksum **558544ad358283a7ab2930d69a9ceddf913f4a51ee9bf1bfb9e377322af81a69**. The user's installed library was byte-for-byte matched to the developer's GStreamer SDK, **not downloaded separately by each end user from Cisco**.

- Cisco's BSD source/binary copyright licence permits redistribution if copyright, conditions and disclaimers accompany binaries: https://github.com/cisco/openh264/blob/master/LICENSE
- **Separate patent coverage:** Cisco's own OpenH264 binary patent grant is conditional. Its official BINARY_LICENSE says the *Cisco-provided binary* must be **separately downloaded to the end user's device**, not first bundled or integrated with third-party software. It also requires end-user control, named codec notice and more conditions: https://github.com/cisco/openh264/blob/gh-pages/BINARY_LICENSE.txt
- An OpenH264 binary distributed inside a 206 MB third-party Inno installer **cannot assume** it benefits from that specific Cisco patent arrangement. The underlying BSD software licence is separate and may still permit redistribution of copyrighted code. This is **not a finding of patent infringement**, a determination that H.264 is currently patented in any specific country, or a substitute for jurisdiction-specific advice.

**Practical release options:** (A) remove **both** `libopenh264-7.dll` and `gst-plugins/libgstopenh264.dll` from a **new copy** of the cleaned runtime, rebuild and re-test all MP4/H.264 karaoke files using remaining Windows Media Foundation or other decoders; retain the unchanged working version; or (B) obtain and document an applicable patent/distribution right, which may be complex. Do **not** assume Cisco's separately downloaded binary patent arrangement covers the currently bundled GStreamer/Cerbero DLL.

Removing OpenH264 alone does not settle patent conditions potentially associated with other H.264 decoders or encoded media. Review the resulting codec chain and applicable local laws before deciding to publish.

## Microsoft Visual C++ redistributable

- The earlier installer builder embeds `vc_redist.x64.exe` temporarily during Inno Setup installation. The file is **not** in the 340 runtime-file inventory once installation completes.
- The development machine has two **Microsoft-signed**, identical Visual Studio 2026 Community redistributable candidates with SHA-256 **843068991daaa1f73ad9f6239bce4d0f6a07a51f18c37ea2a867e9beca71295c**, 18,731,856 bytes, version 14.51.36247.0. Both are located under VS 2026 `VC\Redist\MSVC\...`.
- The existing 205,876,054-byte **draft** Inno installer was not directly inspected to establish the **embedded** VC installer hash; Linux Inno Setup 7 extraction attempts could not extract it. The runtime-match audit doesn't test temporary setup payloads.
- Microsoft permits eligible **unmodified** VS 2026 distributable files for a person with a validly licensed VS edition, subject to all terms: https://learn.microsoft.com/en-us/visualstudio/releases/2026/redistribution
- **Needed for the next build:** explicitly select a known valid Microsoft-signed redist, pin its expected SHA-256, save its path/version/hash in the build manifest, and embed exactly those bytes. Do not rely on the old builder's implicit "newest installed VC redist" selection.

## Microsoft Direct3D compiler

The bundled `d3dcompiler_47.dll` SHA-256 matched a Windows SDK `Redist\D3D\x64` original byte-for-byte and has a valid Microsoft signature. Microsoft explicitly allows redistributing that SDK DLL **side-by-side** in the application directory, rather than copying to the Windows system directory: https://learn.microsoft.com/en-us/windows/win32/directx-sdk--august-2009-/ .

**Conclusion:** Direct3D app-local distribution is supported by primary Microsoft documentation (subject to SDK terms). Visual C++ conditions are known for the verified candidate, but the **actual embedded redist remains unproven**. The OpenH264/Cisco patent condition is **not satisfied by simply adding a licence file**.

This review updates an earlier vague "check OpenH264" item with exact source evidence and a concrete decision path. Do not publish the draft binary solely on this document.
