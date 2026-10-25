> Current status, 2026-10-25: [release completion and remaining reruns](RELEASE_COMPLETION_2026-10-25.md) supersedes the historical pending statements below. The hardware test passed by user report. Application source and installed-file verification are recorded there. Retained-plugin source/notice coverage has been extended locally; final Windows execution awaits GitHub Actions re-enablement. The old working draft must not be published wholesale. [Source access index](SOURCE_ACCESS_v9.md).

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


## Remaining archive sources and libffi preserved — 2026-10-24

The additional mirrors succeeded: [batch 2 run 40066585303](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40066585303), [batch 3 run 40066740913](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40066740913), and [final four-batch verification 40066904767](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40066904767). GitHub API confirms the assets below in the same unpublished draft 421204266.

- Batch 2 preserves 12 more recipe-pinned source archives, covering 14 original DLL mappings.
- Batch 3 preserves the remaining 32 archive-pinned recipe families, covering 40 original DLL mappings. Separate package directories prevent collisions between same-named archives, including the two different `v2.6.0.tar.gz` files.
- Batch 4 preserves libffi Git source. Its recipe tag `meson-3.2.9999.3` resolves to annotated tag object `7b21e70fa237c4ee5ac465bbb6bfd8caae9219c2` and commit `258a091198308063770643cf74a0deb6e121196c`. Both are verified against the upstream Git server before downloading. The immutable commit archive SHA-256 is `184d16c0ebcfc7c4bb1e2dff5d0c16b51ea9892eadf6b0fedf0ae34a39f3e355`; this is an independently recorded archive pin, **not** a checksum declared by the Git-based recipe. Meson build files, native source and LICENSE were confirmed in the archive.

The SpanDSP upstream URL returned SHA-256 `3423504100a75834033aaf6d20caa63121da3897034bc7ed41e9fe70f8b5306e`, which does not match the recipe. Those bytes were rejected. The official GStreamer fallback mirror supplied the archive matching the unchanged recipe pin `cc053ac67e8ac4bb992f258fd94f275a7872df959f6a87763965feabfdcc9465`. Downloads from every location must still pass the same expected hash.

| Draft asset | Bytes | SHA-256 | Asset ID |
| --- | ---: | --- | ---: |
| `OpenKJ-v9-Transitive-Source-Batch2-Manifest.json` | 8,093 | `8ef40eb1b95177fde3b82a225c629bc9a8e5fbd663b0ee9fce606ec20aed7746` | 671377558 |
| `OpenKJ-v9-Transitive-Source-Batch2.zip` | 54,866,947 | `cfaecddfcd239b21677b15e6e2fb856c0eac493ed8c27cd3b882d520762f9f61` | 671377500 |
| `OpenKJ-v9-Transitive-Source-Batch3-Manifest.json` | 20,851 | `0859c7a5f4a1624562f7140f4ec875c6968a5432ca2892bcab883dacfbb390ab` | 671382428 |
| `OpenKJ-v9-Transitive-Source-Batch3.zip` | 199,105,015 | `5082a5247b09761eb4d4bc7fe03a6b6220196535e40118ca3af540cb0ef0424b` | 671382258 |
| `OpenKJ-v9-Transitive-Source-Batch4-Manifest.json` | 1,911 | `09ae2d7d4c8453e9bf51116f0ef4a1fde9a6652cf7acc383ce6a02b8f8307180` | 671385914 |
| `OpenKJ-v9-Transitive-Source-Batch4.zip` | 3,951,617 | `c9812f67a540096f2c512f7dfda7ae4769f02d385c1ac793d9b7b842a5307525` | 671385888 |

Including the earlier eight-package batch and proxy-libintl, upstream archives now exist for **54 of the 56 mapped recipe families**, associated with **71 of the original 74 GStreamer-side transitive DLLs**. This is source-family/archive coverage for the original inventory, not evidence that these exact sources produced the shipped binaries; the original OpenH264 mapping is included even though that component is omitted from the newer test candidate.

The two recipe families still lacking corresponding toolchain source preservation are `mingw-runtime` (two original DLLs) and `winpthreads-runtime` (one). Additional dependency sources (including Rust crates or build-generated/subproject inputs), exact compiler/build configuration, binary-to-source provenance, application build attestation and a durable public source-delivery arrangement remain to be verified. A recipe-pinned SQLite amalgamation archive or a library's top-level source tarball alone must not be assumed to contain every build input. The manifests intentionally retain `complete_corresponding_source=false` and `binary_build_match_proven=false`. Historical manifest gap lists describe their generation-time state and are superseded by this coverage update.

## Static imports after omitting OpenH264 — 2026-10-24

[Windows run 40066585261](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40066585261) passed the extended candidate audit. The checksum-pinned pefile 2024.8.26 parser examined normal and delay PE imports in all 340 original and 338 retained program binaries without executing them. **Zero retained normal/delay imports name an OpenH264 component.** The full per-file hashes/import inventory is available as [text-only audit artifact 12549770611](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40066585261/artifacts/12549770611) (30-day artifact retention; reproduce with the committed workflow).

This narrowly excludes static import edges to the omitted codec. It does not check dynamic library loading, forwarded exports, all Windows/system dependencies, available decoding paths, media playback or guest/phone controls. The local regression test and rebuilt installer verification remain required. No candidate program binaries were uploaded as workflow artifacts or release assets.


## Remote verification follow-up — 2026-10-24

See [the consolidated remote result](REMOTE_VERIFICATION_RESULT_2026-10-24.md) for the executed Windows installer round trip, fresh source build, HTTP probes, Qt renderer and GNU runtime byte matches, expanded source/notice mirrors, original failure causes and rerun steps. It supersedes earlier pending-test statements only for checks explicitly evidenced there. The original draft installer remains unchanged and unpublished. Physical karaoke tests, final codec distribution integration and complete corresponding-source/notice review remain open.


## Final remote engineering follow-up — 2026-10-24

The [completion report](REMOTE_COMPLETION_FOLLOWUP_2026-10-24.md) supersedes earlier pending engineering statements for the checks explicitly proved there: optional end-user codec controls, both application executables built from recorded source, direct official SDK byte matches, expanded Qt/Rust notices and source mirrors, and the source-built installer test. It records the remaining evidentiary/licence-use limits separately. The Windows release remains draft-only for physical karaoke testing.
