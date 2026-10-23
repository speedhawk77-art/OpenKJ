# OpenKJ Offline Remote v9 — installed Windows integrity audit

**Result: PASS for binary inventory and installed notice presence. NOT complete legal redistribution clearance.**

The community tester ran the audit **against OpenKJ installed from the cleaned single-file Windows installer on a second Windows computer**, then supplied three generated evidence files. The data was reviewed against the original 376-binary portable inventory. The underlying evidence files are available from the tester; this report records their counts, checksums and independent cross-checks without posting the user's computer path.

## Binary integrity

| Test | Verified outcome |
| --- | ---: |
| Original reference runtime binaries | 376 |
| Intentionally omitted GStreamer plugins | **36** |
| Expected cleaned application binaries | **340** |
| Installed application binaries | **340** |
| Installed Inno Setup uninstaller (separate generated executable) | **1** |
| Installed application binaries with identical SHA-256 to original reference | **340/340** |
| Missing reference binaries (other than 36 deliberately removed plugins) | **0** |
| Unexpected application binaries | **0** |
| Reference binaries present with changed hashes | **0** |
| GStreamer plugin DLLs installed | **196**, as expected |
| Removed GStreamer plugin DLLs still present | **0** |

The full file-by-file independent comparison of `INSTALLED-RUNTIME-INVENTORY.csv` against the earlier `OpenKJ-LICENSE-INVENTORY.csv` found **exactly 36 differences, all removed GStreamer plugin DLLs**. Every one of the 340 remaining application-binary SHA-256 hashes matches. The installed uninstaller `unins000.exe` is an expected Inno-generated executable, not an unwanted library.

## Installed binary composition

| Category | Count |
| --- | ---: |
| GStreamer plugins | 196 |
| GStreamer libraries | 40 |
| GStreamer library/tool | 2 |
| Qt library/plugins | 21 |
| Other / transitive DLLs still needing component-specific licence provenance | **79** |
| OpenKJ executable/launcher | 2 |
| Generated Inno uninstaller | 1 |

All 21 identified Qt binaries report Windows file version `5.15.2.0`. The 196 remaining GStreamer plugin licences recorded from the **earlier** gst-inspect scan were **183 LGPL, 10 MIT/X11, and 3 BSD**. These reference plugin metadata fields were carried forward by the installed-inventory tool; the installed DLL hashes match the earlier binaries. This does not verify the licences or dependencies of the full 340-file package.

## Installed documentation

The audit found **118 licence/notice documents**:

- **109** component documents collected from the GStreamer SDK (including **71** `README-LICENSE-INFO.txt` package reference files);
- **9** top-level documents for OpenKJ GPL, third-party notices, QR licence/encoder notice, Qt LGPL/GPL terms, GStreamer LGPL terms, and a licensing-status README.

The required nine named core documents were present; this verifies **presence, not completeness or sufficient corresponding source**. Some package-reference README files point to full umbrella licence texts not included in the previously collected SDK archive.

## Transitive dependency evidence — still unresolved

The separate, earlier component-document evidence matrix contains 79 transitive DLLs. Every binary hash in that matrix was independently matched to the installed inventory, so its subject files are confirmed to be in this exact installed release:

- **40**: a component-specific notice/copyright document exists in the supplied SDK licence archive;
- **33**: only a short package licence-reference README exists (not sufficient by itself);
- **6**: no matching SDK licence document was found.

The six with **no package evidence** are:

1. `d3dcompiler_47.dll` — Microsoft Direct3D compiler;
2. `iconengines/qsvgicon.dll` — Qt SVG plugin (Qt 5.15.2 metadata);
3. `libEGL.dll` — Qt graphics/ANGLE component;
4. `libGLESv2.dll` — Qt graphics/ANGLE component;
5. `libintl-8.dll` — source/package not independently mapped;
6. `opengl32sw.dll` — Qt software OpenGL graphics component.

No one of these is proven non-redistributable merely because the SDK's component `share/licenses/` collection did not contain a matching filename. **None of the 79 is cleared simply by having an SDK notice.** Exact source provenance, corresponding LGPL sources, any source modifications, copyright texts, Microsoft redistributable rights and applicable codec/patent conditions still require evidence.

## Evidence file fingerprints

Evidence supplied by the community tester (SHA-256 of entire generated report/CSV):

| Evidence filename | SHA-256 |
| --- | --- |
| `OPENKJ-INSTALLED-AUDIT-REPORT.md` | `63b929ae230b4c81b742a0c782d2c5493d1e236140830d7991fc3a885871b5e4` |
| `INSTALLED-RUNTIME-INVENTORY.csv` | `96502cdbd25a5a3c1b1a4cd87b4d182a5bc88b2ca213f9ba27920d799ab2896c` |
| `INSTALLED-LICENSE-INVENTORY.csv` | `09b363d8cd61b6da50a5f8df77b0a55ad9bf22e78b89fe02d88dc946253f000a` |

## Release decision

**The installed runtime integrity subcheck is COMPLETE.** The actual third-party binary/source/licence compliance subcheck is OPEN. See [readiness checklist](RELEASE_READINESS_CHECKLIST_v9.md), [transitive dependency review](TRANSITIVE_DLL_AUDIT_v9.md), and [source availability tracker](SOURCE_DISTRIBUTION_STATUS_v9.md). Keep release 421204266 as a **GitHub draft**.
