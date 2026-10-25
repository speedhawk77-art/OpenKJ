# OpenKJ v9 source access and library changes

The application and launcher in the user-tested installer were built from
[`016d5c3135bbc8073b96881e49e19feb97359101`](https://github.com/speedhawk77-art/OpenKJ/tree/016d5c3135bbc8073b96881e49e19feb97359101).
The application is GPL-3.0; each bundled library retains its own terms.

## Source archives to accompany the installer

The public release is
[`v9.0-offline-remote.2`](https://github.com/speedhawk77-art/OpenKJ/releases/tag/v9.0-offline-remote.2).
The matching source assets accompany the installer and are publicly accessible
without signing in. Keep these source downloads available with the binary.
No written source offer is substituted for those files.

| Component | Source assets |
| --- | --- |
| OpenKJ and launcher | `OpenKJ-v9-Application-Source-016d5c3135bbc8073b96881e49e19feb97359101.zip`; includes tracked submodule source |
| Qt 5.15.2 | `qt-everywhere-src-5.15.2.tar.xz` |
| GStreamer 1.26.9 | Core, base, good, bad and RTSP server `1.26.9.tar.xz` archives |
| SDK recipes, patches and build instructions | `GStreamer-cerbero-1.26.9-source-and-recipes.tar.gz`, commit `c8981c6e66a7c877a9a9a9cdc7a14668296f3f64` |
| Native libraries and compiler runtime sources | `OpenKJ-v9-Transitive-Source-Batch1.zip` through `Batch6.zip`, their manifests, and `proxy-libintl-0.4-source.tar.gz` |
| librsvg Rust dependencies | `OpenKJ-v9-librsvg-2.59.2-Cargo-Sources.zip` and manifest |
| Retained Rust plugins and static/header dependencies | `OpenKJ-v9-Retained-Plugin-Sources.zip` and manifest: plugin commit `901616eec08a80516b4787dcf52e4889ee6cd1ba`, Cargo.lock, 734 registry archives, five Git snapshots, seven native/header dependencies, five C module sources and Cerbero recipes |
| Rust standard library | `OpenKJ-v9-Rust-Standard-Sources.zip` and manifest, Rust 1.85.0 |
| Qt software renderer | `OpenKJ-v9-Software-OpenGL-Sources.zip` and manifest, Mesa/LLVM sources |

The exact selected existing asset IDs, names, sizes and SHA-256 values are in
`community/windows/data/public-release-source-assets-v9.json`. New plugin
archive hashes are recorded in `community/RELEASE_COMPLETION_2026-10-25.md`.
Keep source manifests, build recipes and patches with the corresponding archives.

## Building and modifying

The source commit's `.github/workflows/build-openkj-v9-source-audit.yml`
records the application build commands and dependency selection. It uses Qt
5.15.2 MSVC x64 and MSVC 14.44; newer compiler versions are not an equivalent
drop-in build environment. The source archive preserves the application build
files and bundled TagLib source used by that build. Inspect its own licence
and build files when modifying that component.

For SDK libraries use the preserved Cerbero recipes and their conditional
Windows patches with the matching sources. The source archive manifests
record hashes and Git revisions. Rust archives preserve the complete lockfile
and original inputs; an offline Cargo vendor tree is not pre-generated.
Supplier byte matching establishes the distributed Qt/GStreamer package origin;
it does not claim bit-for-bit rebuilds of every upstream library.

Compatible LGPL libraries may be replaced. See
`WINDOWS_DLL_REPLACEMENT_GUIDE_v9.md` in this directory. OpenKJ does not impose
an additional restriction on debugging library modifications. Full original
component notices are installed under `licenses/`.

Microsoft terms apply to Microsoft components only. The SDK-built OpenH264
runtime is excluded. The optional Cisco binary is downloaded separately on
the end user's computer after the application's codec prompt; the original
Cisco terms and attribution are supplied with that feature.
