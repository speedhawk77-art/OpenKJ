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


## Verified source mirror result — 2026-10-22

The GitHub Actions job [`39873916515`](https://github.com/speedhawk77-art/OpenKJ/actions/runs/39873916515) completed **successfully**. GitHub API independently confirms these six checksum-verified archives are attached to release **421204266**, which remains **draft=true**, along with `OpenKJ-v9-Upstream-Source-Mirror-Manifest.json`:

| Source archive in draft | Bytes | SHA-256 |
| --- | ---: | --- |
| `qt-everywhere-src-5.15.2.tar.xz` | 586690220 | `3a530d1b243b5dec00bc54937455471aaa3e56849d2593edb8ded07228202240` |
| `gstreamer-1.26.9.tar.xz` | 1894152 | `8ee8772e468102ebe86d414d8c4c7440dd91d9f3e9396952ff23aecde80670c8` |
| `gst-plugins-base-1.26.9.tar.xz` | 2499200 | `11db4ec9ae5be2194ff8ccf2b5fecdc069252fe4416921b083a921ef71dd000a` |
| `gst-plugins-good-1.26.9.tar.xz` | 3007176 | `f38566fb53ae56b7019a039475311756a28eb978a781154d67813dcb551ff5fe` |
| `gst-plugins-bad-1.26.9.tar.xz` | 7787684 | `a96bfb68c53a7c9ab1ae279eac7c101f4204fb5c0e4729683b2be64261707446` |
| `gst-rtsp-server-1.26.9.tar.xz` | 291996 | `67e92e9a5b0f2b51eb571cc893b80da2ae2374f19b05d381d1e364393fe12654` |

**Not a complete LGPL corresponding-source bundle.** The six archives are upstream snapshots. Exact Windows SDK build patches and upstream sources for other LGPL-covered DLLs are still unverified; see checklist above. Do not make this draft public on the basis of these six archives alone.


## GStreamer/Cerbero source recipes and patches mirrored — 2026-10-22

The official `GStreamer/cerbero` **annotated tag `1.26.9`** (tag SHA `477c21ce40a3d304fa2cf43f39bc01d832f1ce20`, resolved **commit `c8981c6e66a7c877a9a9a9cdc7a14668296f3f64`**) was retrieved by GitHub Actions and staged to the same **draft** release as the cleaned installer. Success confirmed in [workflow run `39878936859`](https://github.com/speedhawk77-art/OpenKJ/actions/runs/39878936859).

- `GStreamer-cerbero-1.26.9-source-and-recipes.tar.gz`: **3,455,696 bytes**, SHA-256 `1a5732762e29b3db189ddc0ac68f6efba7bc31d7e8f73d4f0fd61ca4a991d86a`.
- `OpenKJ-v9-Cerbero-Source-Manifest.json`: **588 bytes**, SHA-256 `3f172257bf93981cabb8c138f47af0de0bdd1966a037c7e54522d431ead93f23`.

**Limit:** Cerbero includes build recipes and patches but **not necessarily the full third-party source tarballs** used to build the Windows MinGW SDK, and it does not establish the exact build recipe was used for the user-installed DLL hashes. Corresponding library source and legal compliance work remain open. This is documented and checksum-verified source **build metadata**, not a public release clearance.


## proxy-libintl corresponding upstream source mirrored — 2026-10-22

The **official GStreamer Cerbero tag `1.26.9`** recipe `recipes/proxy-libintl.recipe` identifies `proxy-libintl` **v0.4** as LGPLv2-or-later, producing `libintl`, with upstream source `https://github.com/frida/proxy-libintl/archive/refs/tags/0.4.tar.gz`, SHA-256 `13ef3eea0a3bc0df55293be368dfbcff5a8dd5f4759280f28e030d1494a5dffb`, and Cerbero patch `recipes/proxy-libintl/0001-Rename-symbols-for-compatibility-with-gettext.patch`. This is a strong recipe-to-module match for `libintl-8.dll` sourced byte-identically from the GStreamer MinGW SDK.

[GitHub Actions run `39881106017`](https://github.com/speedhawk77-art/OpenKJ/actions/runs/39881106017) successfully downloaded the checksum-pinned upstream source, verified it, and attached it to draft release 421204266:

- `proxy-libintl-0.4-source.tar.gz`: **13,033 bytes**, SHA-256 `13ef3eea0a3bc0df55293be368dfbcff5a8dd5f4759280f28e030d1494a5dffb`;
- `OpenKJ-v9-Proxy-Libintl-Source-Manifest.json`: **493 bytes**, SHA-256 `d5c64f826f6177d164bfdb60406e5e1ee4b742ea41fdb12b16f4ad093e2b01d3`.

The separately mirrored Cerbero 1.26.9 archive also contains the corresponding patch, but the exact GStreamer Windows SDK build commit and whether its `libintl-8.dll` used this particular recipe remain to be proven; do **not** call that DLL's complete corresponding-source obligations cleared based on these two archives alone.


## Eight transitive source archives preserved — 2026-10-24

[Workflow run 40066229494](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40066229494) passed. The existing draft release **421204266** now holds a source-only bundle and manifest. Each upstream archive was verified against the SHA-256 declared by its recipe in checksum-pinned Cerbero commit `c8981c6e66a7c877a9a9a9cdc7a14668296f3f64`. No recipe code was executed. The bundle includes the complete pinned Cerbero source/recipes archive, preserving conditional patches and build instructions alongside the eight upstream archives.

| Package | Version | Recipe-pinned source SHA-256 |
| --- | --- | --- |
| fribidi | 1.0.16 | `1b1cde5b235d40479e91be2f0e88a309e3214c8ab470ec8a2744d82a5a9ea05c` |
| gdk-pixbuf | 2.42.12 | `b9505b3445b9a7e48ced34760c3bcb73e966df3ac94c95a148cb669ab748e3c7` |
| glib | 2.80.5 | `9f23a9de803c695bbfde7e37d6626b18b9a83869689dd79019bf3ae66c3e6771` |
| glib-networking | 2.80.1 | `b80e2874157cd55071f1b6710fa0b911d5ac5de106a9ee2a4c9c7bee61782f8e` |
| json-glib | 1.10.6 | `77f4bcbf9339528f166b8073458693f0a20b77b7059dbc2db61746a1928b0293` |
| libiconv | 1.15 | `ccf536620a45458d26ba83887a983b96827001e92a13847b45e4925cc8913178` |
| libsoup | 3.6.0 | `62959f791e8e8442f8c13cedac8c4919d78f9120d5bb5301be67a5e53318b4a3` |
| pango | 1.56.2 | `03b7afd7ed730bef651155cbfb5320556b8ef92b0dc04abbb9784dcd4057afe7` |

- `OpenKJ-v9-Transitive-Source-Batch1.zip`: 25,859,997 bytes; SHA-256 `00795c4fe6d887dc620cc039934d512d26433c76616fea2ead51547fc10e9aa3`; draft asset 671367048.
- `OpenKJ-v9-Transitive-Source-Batch1-Manifest.json`: 5,867 bytes; SHA-256 `7f0cffef41d21cf529f3c7839f8a5eea73d555bbfb856f48f99b43cb7aa5474b`; draft asset 671367133.

These eight recipe families map to **15 of the original 74 GStreamer-side transitive DLLs**. Source mirroring for proxy-libintl was completed earlier and is separate. Other libraries, toolchain sources, exact shipped build provenance and application build attestation remain open. This bundle is partial; it does not establish complete corresponding source, and draft assets are not publicly accessible source delivery. No installer asset was changed or published.

### Narrowed toolchain and Git-source gaps

The literal recipe version `0.1` is a packaging placeholder, not a compiler version. The pinned [Windows bootstrap code](https://github.com/GStreamer/cerbero/blob/c8981c6e66a7c877a9a9a9cdc7a14668296f3f64/cerbero/bootstrap/windows.py) specifies `mingw-12.0.0-gcc-14.2.0-windows-multilib.tar.xz`, SHA-256 `2e9039c6404f68bfce1eecd34172b16715565a15555c76c1652ec26265d5ae89`. The [mingw-runtime recipe](https://github.com/GStreamer/cerbero/blob/c8981c6e66a7c877a9a9a9cdc7a14668296f3f64/recipes/mingw-runtime.recipe) references GCC 14.2.0 runtime paths, and [winpthreads-runtime](https://github.com/GStreamer/cerbero/blob/c8981c6e66a7c877a9a9a9cdc7a14668296f3f64/recipes/winpthreads-runtime.recipe) copies its DLL from the selected toolchain. These identify a concrete build-metadata lead; the bootstrap archive is a binary toolchain and is not a substitute for its corresponding upstream sources or evidence that this toolchain built the shipped DLLs.

[libffi.recipe](https://github.com/GStreamer/cerbero/blob/c8981c6e66a7c877a9a9a9cdc7a14668296f3f64/recipes/libffi.recipe) uses Git source `https://gitlab.freedesktop.org/gstreamer/meson-ports/libffi.git` at `meson-3.2.9999.3`, rather than a missing tarball checksum. Next evidence needed is a resolved immutable Git object and archived source, followed by actual SDK build matching.
