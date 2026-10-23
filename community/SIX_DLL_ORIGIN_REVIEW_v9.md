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
