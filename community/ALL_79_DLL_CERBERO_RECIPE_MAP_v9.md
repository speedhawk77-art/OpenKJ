# OpenKJ v9 — 79 installed third-party DLLs: source-recipe family map

**Review stage: likely source-family mapping only. NOT complete corresponding-source delivery or clearance to publish the Windows installer.** This is based on the previously submitted 340-file installed Windows inventory, which independently matched SHA-256 against the tested clean portable build, and the [official GStreamer/Cerbero tag 1.26.9](https://github.com/GStreamer/cerbero/tree/1.26.9/recipes).

The **79 transitive DLLs** group into **58 likely source families** (56 corresponding Cerbero recipes and the separate Qt and Microsoft SDK sources). The mapping is by DLL naming / package conventions and the recipe's declared build outputs; **it is not a proof that these particular bytes came from the exact recipe version**. The six separately investigated DLLs have direct, byte-for-byte supplier SDK proof. No antivirus-bypassing or local script execution was needed for this review.

| Likely component / upstream tagged recipe | Shipped DLL count | Installed DLL filenames |
| --- | ---: | --- |
| Supplier SDK (see origin proof) | 1 | `d3dcompiler_47.dll` |
| Supplier SDK (see origin proof) | 4 | `iconengines/qsvgicon.dll`, `libEGL.dll`, `libGLESv2.dll`, `opengl32sw.dll` |
| [`glib-networking`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/glib-networking.recipe) | 1 | `gio-modules/libgioopenssl.dll` |
| [`libass`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/libass.recipe) | 1 | `libass-9.dll` |
| [`bzip2`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/bzip2.recipe) | 1 | `libbz2.dll` |
| [`cairo`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/cairo.recipe) | 3 | `libcairo-2.dll`, `libcairo-gobject-2.dll`, `libcairo-script-interpreter-2.dll` |
| [`libiconv`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/libiconv.recipe) | 2 | `libcharset-1.dll`, `libiconv-2.dll` |
| [`openssl`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/openssl.recipe) | 2 | `libcrypto-3-x64.dll`, `libssl-3-x64.dll` |
| [`dav1d`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/dav1d.recipe) | 1 | `libdav1d.dll` |
| [`libdv`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/libdv.recipe) | 1 | `libdv-4.dll` |
| [`expat`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/expat.recipe) | 1 | `libexpat-1.dll` |
| [`libffi`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/libffi.recipe) | 1 | `libffi-7.dll` |
| [`flac`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/flac.recipe) | 1 | `libFLAC-8.dll` |
| [`fontconfig`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/fontconfig.recipe) | 1 | `libfontconfig-1.dll` |
| [`freetype`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/freetype.recipe) | 1 | `libfreetype-6.dll` |
| [`fribidi`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/fribidi.recipe) | 1 | `libfribidi-0.dll` |
| [`mingw-runtime`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/mingw-runtime.recipe) | 2 | `libgcc_s_seh-1.dll`, `libstdc++-6.dll` |
| [`gdk-pixbuf`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/gdk-pixbuf.recipe) | 1 | `libgdk_pixbuf-2.0-0.dll` |
| [`glib`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/glib.recipe) | 5 | `libgio-2.0-0.dll`, `libglib-2.0-0.dll`, `libgmodule-2.0-0.dll`, `libgobject-2.0-0.dll`, `libgthread-2.0-0.dll` |
| [`graphene`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/graphene.recipe) | 1 | `libgraphene-1.0-0.dll` |
| [`harfbuzz`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/harfbuzz.recipe) | 2 | `libharfbuzz-0.dll`, `libharfbuzz-gobject-0.dll` |
| [`proxy-libintl`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/proxy-libintl.recipe) | 1 | `libintl-8.dll` |
| [`libjpeg-turbo`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/libjpeg-turbo.recipe) | 2 | `libjpeg-8.dll`, `libturbojpeg.dll` |
| [`json-glib`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/json-glib.recipe) | 1 | `libjson-glib-1.0-0.dll` |
| [`libltc`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/libltc.recipe) | 1 | `libltc-11.dll` |
| [`lame`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/lame.recipe) | 1 | `libmp3lame-0.dll` |
| [`mpg123`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/mpg123.recipe) | 1 | `libmpg123-1.dll` |
| [`nghttp2`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/nghttp2.recipe) | 1 | `libnghttp2.dll` |
| [`libnice`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/libnice.recipe) | 1 | `libnice-10.dll` |
| [`libogg`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/libogg.recipe) | 1 | `libogg-0.dll` |
| [`openh264`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/openh264.recipe) | 1 | `libopenh264-7.dll` |
| [`openjpeg`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/openjpeg.recipe) | 1 | `libopenjp2.dll` |
| [`opus`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/opus.recipe) | 1 | `libopus-0.dll` |
| [`orc`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/orc.recipe) | 2 | `liborc-0.4-0.dll`, `liborc-test-0.4-0.dll` |
| [`pango`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/pango.recipe) | 3 | `libpango-1.0-0.dll`, `libpangocairo-1.0-0.dll`, `libpangowin32-1.0-0.dll` |
| [`pcre2`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/pcre2.recipe) | 1 | `libpcre2-8-0.dll` |
| [`pixman`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/pixman.recipe) | 1 | `libpixman-1-0.dll` |
| [`libpng`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/libpng.recipe) | 1 | `libpng16.dll` |
| [`libpsl`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/libpsl.recipe) | 1 | `libpsl-5.dll` |
| [`librsvg`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/librsvg.recipe) | 1 | `librsvg-2-2.dll` |
| [`sbc`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/sbc.recipe) | 1 | `libsbc-1.dll` |
| [`libsoup`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/libsoup.recipe) | 1 | `libsoup-3.0-0.dll` |
| [`spandsp`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/spandsp.recipe) | 1 | `libspandsp-2.dll` |
| [`speex`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/speex.recipe) | 1 | `libspeex-1.dll` |
| [`sqlite3`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/sqlite3.recipe) | 1 | `libsqlite3-0.dll` |
| [`srt`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/srt.recipe) | 1 | `libsrt.dll` |
| [`libsrtp`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/libsrtp.recipe) | 1 | `libsrtp2-1.dll` |
| [`svt-av1`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/svt-av1.recipe) | 1 | `libSvtAv1Enc.dll` |
| [`svt-jpeg-xs`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/svt-jpeg-xs.recipe) | 1 | `libSvtJpegxs.dll` |
| [`taglib`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/taglib.recipe) | 1 | `libtag.dll` |
| [`libtheora`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/libtheora.recipe) | 3 | `libtheora-0.dll`, `libtheoradec-1.dll`, `libtheoraenc-1.dll` |
| [`tiff`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/tiff.recipe) | 1 | `libtiff-6.dll` |
| [`libvorbis`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/libvorbis.recipe) | 3 | `libvorbis-0.dll`, `libvorbisenc-2.dll`, `libvorbisfile-3.dll` |
| [`wavpack`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/wavpack.recipe) | 1 | `libwavpack-1.dll` |
| [`winpthreads-runtime`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/winpthreads-runtime.recipe) | 1 | `libwinpthread-1.dll` |
| [`libxml2`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/libxml2.recipe) | 1 | `libxml2-2.dll` |
| [`zlib`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/zlib.recipe) | 1 | `libz-1.dll` |
| [`zbar`](https://github.com/GStreamer/cerbero/blob/1.26.9/recipes/zbar.recipe) | 1 | `libzbar-0.dll` |

## What this mapping establishes, and what it does not

- Every identified *filename* now has a plausible source recipe/project. For the 74 GStreamer/MinGW-origin candidates, the tagged upstream recipe is a precise place to check declared licence, source archive checksum, build patches and dependencies. The four Qt graphics/SVG DLLs and one Windows SDK D3D compiler have their own previously documented SDK byte-origin matches.
- The recipe may describe `License.LGPLv2Plus`, `License.LGPLv2_1Plus`, `License.LGPLv3`, `License.BSD`, `License.MIT`, `License.Apachev2`, `License.MPLv2` or several licences. **We must preserve actual copyright notices, check each file's exact built origin, and meet complete corresponding-source requirements for dynamically linked LGPL code.**
- OpenSSL 3 (`libssl`, `libcrypto`), openh264 and other audio/video codecs also require project-specific redistribution, notice and potentially independent codec-patent assessment. Merely finding a valid open-source licence does not settle all of those issues.
- The exact source and patch package of `libintl-8.dll` has a particularly strong mapping to proxy-libintl 0.4 in Cerbero 1.26.9, with its source mirrored and checksummed in the private GitHub draft. The same work remains for other LGPL dependencies.
- `mingw-runtime` and `winpthreads-runtime` label packaging recipes, not the true GCC or MinGW-w64 upstream compiler/runtime source revision. The printed recipe placeholder version cannot be used as a source correspondence proof.
- We still need the exact **original Windows SDK binary/build manifest**, source archives/patches for components covered by LGPL, and complete installed licences. The Visual C++ redistributable candidate is Microsoft-signed in the development environment, but its byte identity with the temporary copy embedded in the Inno 7 installer was verified by [run 40065659931](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40065659931).

## Prior evidence and next checks

1. [Exact installed runtime integrity](INSTALLED_BINARY_AUDIT_RESULT_v9.md) — 340/340 identical application binary hashes; 196 plug-ins and all 118 staged notice documents.
2. [Six SDK byte-origin matches](SIX_DLL_ORIGIN_REVIEW_v9.md) — matched Qt 5.15.2 SDK, GStreamer MinGW 1.26.9 SDK, Windows D3D SDK.
3. [Source distribution](SOURCE_DISTRIBUTION_STATUS_v9.md) — six upstream sources, pinned Cerbero 1.26.9 recipes and proxy-libintl 0.4 source in draft.
4. [Open tasks](RELEASE_READINESS_CHECKLIST_v9.md) — license/source verification and runtime rights must be finished before public binary release.

**Publication decision: HOLD.**