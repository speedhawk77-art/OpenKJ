# OpenKJ v9 — Six Windows DLL origin review

**Status: primary-source licence identification complete, exact installed binary provenance still needs local verification. Public Windows release remains DRAFT.**

The cleaned install SHA-256 matches have been established for all six below. This table separates **what upstream documentation actually supports** from assumptions still requiring proof that the binary came from the expected software distribution.

| Installed DLL (SHA-256 prefix) | Supported identification and primary source | Outstanding evidence |
| --- | --- | --- |
| `iconengines/qsvgicon.dll` (`42dde60b`) | Qt SVG icon plugin, installed version **5.15.2.0**, company **The Qt Company Ltd.**. [Qt SVG 5.15](https://qthub.com/static/doc/qt5/qtsvg/qtsvg-index.html) confirms Qt SVG offers LGPLv3 (among other options). | Match binary SHA-256 with original `C:\Qt\5.15.2\msvc2019_64\plugins\iconengines\qsvgicon.dll`. Preserve SVG-specific third-party attributions. |
| `libEGL.dll` (`32092de0`) | Qt graphics runtime, file version **5.15.2.0**. [Qt Windows Deployment](https://qthub.com/static/doc/qt5/qtdoc/windows-deployment.html) identifies this as part of the **ANGLE** OpenGL-to-Direct3D stack. ANGLE uses [BSD-style licensing](https://github.com/google/angle/blob/main/LICENSE). | Match actual Qt 5.15.2 SDK binary and identify correct vendored ANGLE source/third-party notices for that particular release. |
| `libGLESv2.dll` (`50fad560`) | Same Qt **ANGLE** graphics runtime; **5.15.2.0** in installed file metadata. | Same provenance and third-party-notice check as libEGL. |
| `opengl32sw.dll` (`963641a7`) | [Qt Windows requirements](https://qthub.com/static/doc/qt5/qtdoc/windows-requirements.html) describes this as Qt's **software OpenGL fallback**, commonly a **Mesa llvmpipe** build. Qt also [documents Mesa llvmpipe redistribution](https://wiki.qt.io/MesaLlvmpipe). | Compare with Qt SDK binary; obtain the actual Mesa build version/source/patched code and appropriate full copyright/licence texts. Filename alone does not prove that this specific copy is Mesa or make a determination about its licence. |
| `d3dcompiler_47.dll` (`1da540a0`) | Windows file version **10.0.26100.9457**, company **Microsoft Corporation**. [Microsoft officially permits redistributing the Windows SDK D3DCompiler_47.dll **side-by-side** with an app](https://learn.microsoft.com/en-us/windows/win32/directx-sdk--august-2009-). | Check Authenticode signature and exact file provenance: compare hash with Windows SDK distribution and/or official Qt deployment. Confirm the actual file copy was distributed under the applicable SDK terms; maintain Microsoft's licence notices. |
| `libintl-8.dll` (`f1da20c9`) | Likely the gettext / MinGW internationalization runtime. [GNU gettext licensing](https://www.gnu.org/software/gettext/manual/html_node/Licenses.html) establishes **libintl LGPL**. The installed GStreamer SDK has a `share/licenses/proxy-libintl/README-LICENSE-INFO.txt` citing LGPL-2+, but this does not itself associate the binary with `proxy-libintl`. | Confirm byte-identical to `C:\Program Files\gstreamer\1.0\mingw_x86_64\bin\libintl-8.dll`; identify exact upstream package, source version and any Cerbero patches. Add corresponding sources and full LGPL terms as required. |

## Additional Microsoft installer payload (not shown in installed DLL inventory)

The verified `BUILD_CLEAN_OPENKJ_INSTALLER.py` embeds a separate **`vc_redist.x64.exe`**, runs it quietly during installation if a relevant runtime registry key is absent, then deletes that temporary file. It is **not** in the 340-binary installed-app inventory. The builder selects the newest sufficiently large local candidate found in the distribution test folder, OpenKJ portable folders or a Visual Studio `VC\Redist\MSVC\<version>` directory. The prior build **did not record the selected redist's SHA-256, Authenticode status or path**.

Therefore:
- The absence of `vcruntime140.dll` and `msvcp140.dll` from the installed OpenKJ folder does **not** imply the installer bundles no Microsoft runtime. It bundles the redist EXE within the installer.
- Check the embedded `vc_redist.x64.exe` against an authentic Microsoft-signed authorized redistributable from the licensed Visual Studio/Windows SDK environment. Verify rights according to [Microsoft Visual C++ redistribution rules](https://learn.microsoft.com/en-us/cpp/windows/redistributing-visual-cpp-files).
- If we cannot prove which redistributable was embedded, rebuild from an explicitly identified, Microsoft-signed redist and record its SHA-256 and source with the build manifest, then retest the resulting installer.

## Focused local verification

The read-only `OpenKJ_v9_Origin_Check_Kit.zip` contains `RUN_OPENKJ_ORIGIN_CHECK.bat` / `CHECK_OPENKJ_DLL_ORIGINS.ps1`. On the **development Windows PC** it compares installed SHA-256 hashes with original Qt 5.15.2, GStreamer 1.26.9 and Windows SDK library candidates, checks Authenticode information, and inventories Microsoft redistributable candidates without copying binary files or making changes. Its output is technical evidence and does **not** by itself demonstrate complete source/licence compliance.

## Release decision

The six are **identified as likely known components**, not six established infringing DLLs. Microsoft explicitly allows an SDK D3DCompiler_47 to ship side-by-side. Their exact origin, source availability, and some third-party copyright notices are still unverified. Keep the Windows installer **draft-only** until this and the wider 79-DLL provenance, complete corresponding LGPL sources/patches, and installed notices are resolved.


**Further source progress:** the pinned GStreamer Cerbero tag 1.26.9, with build recipes and patch descriptions, is now SHA-256 verified in the GitHub draft. This improves the evidence for `libintl-8.dll` and other GStreamer-side binaries once their hashes are matched, but is not the complete corresponding source itself. See [source-distribution status](SOURCE_DISTRIBUTION_STATUS_v9.md).


## Development SDK byte-match results — 2026-10-22

The developer ran the read-only comparison against the cleaned **installed** binaries, supplying `SIX-DLL-SDK-MATCHES.csv`, `VC-REDIST-CANDIDATES.csv`, and the corresponding origin report. All six requested DLLs matched byte-for-byte with copies installed in their respective supplier SDKs; the original scanner also recorded several non-matching alternative candidates for the Microsoft compiler, which is normal.

| DLL | Confirmed byte-identical SDK location | Installed SHA-256 | Signature |
| --- | --- | --- | --- |
| `d3dcompiler_47.dll` | `Windows Kits\10\Redist\D3D\x64\d3dcompiler_47.dll` | `1da540a0ddbc54de4142570c8ec4a4566ef610fce72c181efb55a9332c480790` | Valid Microsoft |
| `iconengines/qsvgicon.dll` | Qt 5.15.2 `plugins\iconengines\qsvgicon.dll` | `42dde60befcf1d9f96b8366a9988626b97d7d0d829ebea32f756d6ecd9ea99a8` | Valid Qt Company |
| `libEGL.dll` | Qt 5.15.2 `bin\libEGL.dll` | `32092de077fd57b6ef355705ec46c6d21f6d72fbe3d3a5dd628f2a29185a96fa` | Valid Qt Company |
| `libGLESv2.dll` | Qt 5.15.2 `bin\libGLESv2.dll` | `50fad5605b3d57627848b3b84a744dfb6a045609b8236b04124f2234676758d8` | Valid Qt Company |
| `libintl-8.dll` | GStreamer 1.26.9 MinGW `bin\libintl-8.dll` | `f1da20c9a6fc686d582396f67d6c9a8aa8e12bb74aa2b35b0d009bfaff581bbf` | Not signed (not unexpected for MinGW) |
| `opengl32sw.dll` | Qt 5.15.2 `bin\opengl32sw.dll` | `963641a718f9cae2705d5299eae9b7444e84e72ab3bef96a691510dd05fa1da4` | Not signed (not unexpected for Mesa) |

The Microsoft D3D DLL is specifically from the **Windows SDK Redist D3D x64 folder**, rather than merely a system32 alternative, significantly strengthening side-by-side redistribution evidence. Qt’s [Windows deployment reference](https://qthub.com/static/doc/qt5/qtdoc/windows-deployment.html) identifies the graphics dependencies, and [Mesa attribution](https://qthub.com/static/doc/qt5/qtdoc/qt-attribution-llvmpipe.html) lists its MIT licence. Missing exact third-party licence text and source version/build provenance still need attention, however.

The user supplied two **byte-identical**, **Valid Microsoft-signed** Visual Studio 2026 Community `vc_redist.x64.exe` candidates in `VC\Redist\MSVC\14.51.36231` and `VC\Redist\MSVC\v145`. Both have SHA-256 `843068991daaa1f73ad9f6239bce4d0f6a07a51f18c37ea2a867e9beca71295c`, size **18,731,856 bytes**, version **14.51.36247.0**. Microsoft’s [2026 distributable list](https://learn.microsoft.com/en-us/visualstudio/releases/2026/redistribution) explicitly includes `[VisualStudioFolder]\VC\redist` for validly licensed Visual Studio Community users distributing unmodified files with an application. **Important:** these candidates do not prove which EXE was actually *embedded* in the Inno installer; verify that separately before checking off the redist package subcheck. No private licence/account data was supplied.

**New conclusion:** the **six installed DLLs' direct SDK byte-origin check is complete**. The six DLLs are not simply “unidentified unknown binaries.” **Neither exact upstream third-party source code nor all component notice obligations are proven by this hash comparison.** The broader remaining DLL/source audit and release hold still apply.


## Additional upstream notices and embedded-redist inspection — 2026-10-22

The exact original **Qt 5.15.2 ANGLE** copyright and licence files have now been copied, **unmodified**, from `qt/qtbase` tag `v5.15.2` into:

- `community/licenses/qt-5.15.2/angle/LICENSE` (ANGLE project BSD-style notice);
- `community/licenses/qt-5.15.2/angle/KHRONOS_LICENSE` (Khronos MIT permission);
- `community/licenses/qt-5.15.2/angle/SYSTEMINFO_LICENSE` (Apple BSD);
- `community/licenses/qt-5.15.2/angle/TRACEEVENT_LICENSE` (Chromium BSD);
- `community/licenses/qt-5.15.2/angle/qt_attribution.json` (Qt's original 5.15.2 attribution metadata).

Qt documents `opengl32sw.dll` as a Mesa llvmpipe software renderer, with an [MIT third-party attribution](https://qthub.com/static/doc/qt5/qtdoc/qt-attribution-llvmpipe.html). The original Mesa/LLVM binary's exact build revisions remain to be identified. Adding notices to the GitHub source branch **does not automatically add them to the already-built Windows installer**. A future revised installer must ship the applicable notices, source and modifications if required.

The **draft installer EXE itself** (SHA-256 `bc35c24ef25529b86b90bd3f5a215603de3b283d590620ec58820e02f83421ef`) was downloaded and independently hash-verified in GitHub Actions. The first extraction attempt used Linux `innoextract` v1.9 (workflow `39880549278`), which could not parse its newer **Inno Setup 7** format. A second attempt with 7-Zip (workflow `39880728389`) also could not read this archive. **Neither failure is evidence that the installer is corrupt:** it successfully installed on Windows and matched all installed runtime hashes. Crucially, **neither tool retrieved the embedded temporary `vc_redist.x64.exe`**; therefore the embedded redistributable's hash/actual source **is still unverified**. The two VS 2026 Community candidates had matching hash `843068991daaa1f73ad9f6239bce4d0f6a07a51f18c37ea2a867e9beca71295c`, but may not be the actual payload selected when the installer was compiled. This requires build provenance or a rebuild that deliberately pins and records the exact Microsoft-signed redist.

Microsoft's official [VS 2026 redistribution list](https://learn.microsoft.com/en-us/visualstudio/releases/2026/redistribution) allows licensed VS 2026 users to distribute **unmodified** files within their `VC\redist` tree subject to licence terms. Microsoft's [Windows SDK D3DCompiler guidance](https://learn.microsoft.com/en-us/windows/win32/directx-sdk--august-2009-) allows SDK `D3DCompiler_47.dll` application-local distribution; the installed file hash exactly matches the developer's Windows Kit `Redist\D3D\x64` copy.


## Remote verification follow-up — 2026-10-24

See [the consolidated remote result](REMOTE_VERIFICATION_RESULT_2026-10-24.md) for the executed Windows installer round trip, fresh source build, HTTP probes, Qt renderer and GNU runtime byte matches, expanded source/notice mirrors, original failure causes and rerun steps. It supersedes earlier pending-test statements only for checks explicitly evidenced there. The original draft installer remains unchanged and unpublished. Physical karaoke tests, final codec distribution integration and complete corresponding-source/notice review remain open.


## Final remote engineering follow-up — 2026-10-24

The [completion report](REMOTE_COMPLETION_FOLLOWUP_2026-10-24.md) supersedes earlier pending engineering statements for the checks explicitly proved there: optional end-user codec controls, both application executables built from recorded source, direct official SDK byte matches, expanded Qt/Rust notices and source mirrors, and the source-built installer test. It records the remaining evidentiary/licence-use limits separately. The Windows release remains draft-only for physical karaoke testing.
