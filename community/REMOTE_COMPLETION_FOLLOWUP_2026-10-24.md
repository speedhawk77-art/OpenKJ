# OpenKJ v9 — remaining remote engineering completed

**Status updated 2026-10-25: remote software/build checks completed and the user reports the requested karaoke hardware checks passed. The Windows release remains an unpublished draft pending the separate release review.** The historical installer was not overwritten. See the dated hardware confirmation below; authenticated DJ operations beyond the confirmed remote usage are not separately attested.

This report supersedes the engineering items left open in [the earlier remote result](REMOTE_VERIFICATION_RESULT_2026-10-24.md). It records technical licence/source evidence, not a legal certification or a guarantee of rights for every use of media.

## Optional Cisco codec integration

The source-built app now has **Help → Optional H.264 codec…**. It starts disabled. The dialog displays Cisco's exact attribution, the complete original binary licence, and controls to download/enable, disable and re-enable. A restart applies the change. The first optional download needs Internet; subsequent use and re-enabling the verified cached copy work offline. See [setup instructions](OPTIONAL_H264_SETUP_v9.md).

The app downloads directly from Cisco over Windows' native HTTPS stack with no redirects, limits response/decompression size, checks the compressed and decompressed SHA-256 pins and saves the exact binary atomically in the user's cache. It never uploads media or library information. No Cisco binary is bundled in the installer. The original GStreamer adapter is stored outside the normal plugin directory and only made available at startup when the user has enabled a verified codec. Separate on/off registry paths prevent the normal startup from reusing optional-codec features from the other mode's cache. The adapter remains replaceable; no application lock prevents modifying LGPL libraries.

[Source-build run 40098937739](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40098937739) passed **22 codec checks**, including:

- exact original Cisco licence bytes and complete rendered licence text;
- required attribution and actual dialog disable/re-enable controls;
- direct HTTPS download, decompression and pinned binary identity;
- rejection of empty, oversized, incorrect and tampered cache data;
- H.264 decoding through a child GStreamer probe using the controller's enabled environment;
- absence of the OpenH264 decoder in the disabled environment.

A Windows checkout conversion had changed legal-file line endings. `.gitattributes` now preserves original legal bytes; both exact resource hashing and the independently checked rendered text passed. No licence check was removed to make CI pass.

Cisco's complete original terms still govern use, including their use limitations. Implementing its separate-download/user-control conditions does not extend its patent licence or grant rights to karaoke recordings or performances.

## Both application executables rebuilt and recorded

The main application **and** `OpenKJ-Remote.exe` launcher were compiled from public source commit `016d5c3135bbc8073b96881e49e19feb97359101`. The app uses Qt 5.15.2, pinned GStreamer 1.26.9 SDK inputs and MSVC 14.44. The launcher is compiled from `community/windows/OpenKJRemoteLauncher.cpp` with recorded options and no private binary dependency.

| Item | SHA-256 |
| --- | --- |
| `openkj.exe` | `6c22337ae416a2c29d25006343bb23009f5208283295749ce9295c5ba97a4fee` |
| `OpenKJ-Remote.exe` | `11ac9486dbc0dad8b0e4c15bed878a495dee816ef571b06a2ec24b2b97c17e96` |
| Exact application source ZIP, 863 tracked files including recursive submodule content | `d2a5b3dce09cf482ed48378e5ceb842dfdf1fa8259e8d98c4382f80beee8ef02` |
| Source-built app/evidence/source bundle | `2b5958b505e92c2dceaafc27d1f25fdd7fd489571302e47134e2ab6f4d560f98` |

The source bundle is retained in draft 421204266. This establishes the new application/launcher source commit; it does not retroactively establish which commit built the historical executable.

## Direct official supplier comparison

[Supplier run 40099815324](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40099815324) passed: **336 direct byte matches, four separately accounted files, zero unmatched** across the original 340 runtime files.

The comparison uses the official Qt 5.15.2 SDK's base, SVG and image-format packages, and the official GStreamer 1.26.9 runtime MSI with SHA-256 `f06f729f965deaff5d499a7feb596a001c25badbc8e9d5a791d0f23a530d66ab`. It locates supplier files by filename and requires the entire SHA-256 to match, including plugin scanner and image plugins in their SDK subdirectories. Earlier misses were missing supplier search locations/modules, not accepted mismatches; those bytes were checked in the successful run.

The four exceptions are explicitly accounted for: two historical app executables replaced by the source-built pair, `d3dcompiler_47.dll` already matched to the developer's Windows SDK Redist copy with valid Microsoft signature, and `opengl32sw.dll` separately matched to Qt's exact published Mesa 12 RC2 binary. The three GNU runtimes also retain their independent pinned-toolchain byte matches. This strengthens supplier provenance beyond comparisons only with the user's installed SDKs; it does not claim a bit-for-bit rebuild of all supplier libraries.

## Expanded original notices and sources

The revised installer includes **1,360 legal/attribution/evidence files**, checked individually against their staged hashes after installation. In addition to existing notices:

- 239 original upstream documents from 61 archives are preserved with recipe/source pins and the Cerbero recipes/patches.
- librsvg's 292 locked registry source archives are mirrored with their lockfile checksums. Its notice supplement now contains 626 original documents, including original metadata/readmes and 53 source attribution headers for crates lacking standalone notices.
- Additional upstream MIT notices cover av-data, the PHF family and difflib. All six published difflib Rust source files were matched to the recorded upstream commit. [Per-crate provenance and limitations](licenses/rust-upstream/PROVENANCE.md) records immutable upstream references and the four Apple-only dependencies whose matching upstream trees themselves lack standalone licence files; no author or year was invented.
- The pinned Cerbero bootstrap selects Rust **1.85.0**. Its channel manifest, standard-library source and **87 registry dependencies** from its shipped lockfiles are now preserved separately; 197 original legal documents are included. This is additional to the librsvg registry mirror.
- Qt's base, SVG and image-format sources supplied **134 original legal/attribution documents**. All **62 declared licence-file references** in **43 attribution metadata files** resolved; unsupported field types and unresolved references were zero.
- Mesa 12.0.0-rc2 and LLVM 3.6.2 source/notice evidence remains preserved. Their exact supplier binary origin is independently checked; private supplier build changes/configuration are not invented.

Important supplemental pins:

| Draft asset | SHA-256 |
| --- | --- |
| OpenKJ-v9-librsvg-Rust-Notices-v2.zip | `ed4998c77dcd9f0d2a9378860579011f7c12ae781cd66ce84025c17299d43a3f` |
| OpenKJ-v9-Rust-Standard-Sources.zip | `c77f014a1299b5c6048fd052a2e23a702efc292e5bc499c39001fc2d50c6cead` |
| OpenKJ-v9-Rust-Standard-Notices.zip | `049b80431100e4108e6d26cfeaa6a00433b7231f2f03024d03ddd42f688a57eb` |
| OpenKJ-v9-Qt-Notices.zip | `0f9f9d177e18ff52c61264bad6cdb256db7a42bcf4c8a491407a5e34f4301238` |

Sources, patches/recipes, application source and manifests are prepared in the same draft as the installer. For an eventual public release, retain and publish these corresponding source assets with the matching binary; do not publish just the installer and leave source assets private or delete them. Existing public application source remains in this repository. No unfulfillable written source offer has been substituted for delivery.

## Installer and startup verification

[Run 40099605240](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40099605240) successfully built, re-extracted and silently installed the source-built candidate:

- **339 installed program hashes** matched, including both source-built executables and the isolated optional GStreamer adapter.
- **1,360 installed notice hashes** matched.
- Exactly one embedded VC redist retained the original size/hash pin.
- No Cisco DLL or active OpenH264 plugin was bundled in the normal runtime/plugin directory.
- Static imports passed: the only permitted optional dependency is the byte-identical original adapter isolated outside the scanned plugin directory. Any other retained OpenH264 import is fatal. The historical 338-file audit still defaults to its original no-edge requirement.
- The freshly compiled launcher started the installed app, which passed **11/11 localhost route checks**. CI terminates its process tree after the check.

[Final rerun 40099935626](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40099935626) also passed every step with **339 program hashes, 1,360 notice hashes and 11/11 installed-app HTTP checks**. It retained the exact standalone application source ZIP in the draft and used a unique installer name to make reruns non-overwriting.

- `OpenKJ-v9-Application-Source-016d5c3135bbc8073b96881e49e19feb97359101.zip`: asset 672400238, 2286471 bytes; SHA-256 `d2a5b3dce09cf482ed48378e5ceb842dfdf1fa8259e8d98c4382f80beee8ef02`.
- `OpenKJ-v9-Official-Supplier-Audit-40099815324.json`: asset 672391769, 87707 bytes; SHA-256 `5dbd5a0094acef46cab62c02165666ef97b554f7b385fbe808d0d2875014c6d7`.
- `OpenKJ-v9-Source-Candidate-016d5c3135bbc8073b96881e49e19feb97359101-40099935626-1.exe`: asset 672399791, 204378210 bytes; SHA-256 `8ad438cc61c212377ab5b4999f5ed0644b8ff26fcbda413a4412c98524351228`.
- `OpenKJ-v9-Source-Candidate-Audit-40099935626-1.zip`: asset 672399683, 151500 bytes; SHA-256 `a26f92ebb23a0edafaf6059a44c314db261103c38d360176912d23f603334c09`.

These are the specifically identified draft assets for the next hardware test. Earlier candidates remain historical evidence; their presence does not make them the selected final test candidate.

The original `OpenKJ-v9-Cleaned-Setup.exe` asset 667394901 remains unchanged with SHA-256 `bc35c24ef25529b86b90bd3f5a215603de3b283d590620ec58820e02f83421ef`. New candidate files have separate names; nothing was made public.

## Rerun and next practical step

The [source-candidate workflow](https://github.com/speedhawk77-art/OpenKJ/actions/workflows/verify-openkj-v9-source-candidate.yml) can be dispatched on `offline-remote-v9`; leave `source_commit` blank to select the latest successfully preserved source-built bundle. Installer/evidence names include run/attempt IDs, so this does not overwrite another candidate. The standalone source asset is reused only when its digest matches.

The [source-build workflow](https://github.com/speedhawk77-art/OpenKJ/actions/workflows/build-openkj-v9-source-audit.yml) builds new source changes and triggers candidate verification on success. Source-build bundles are immutable by source commit; a different rebuild cannot silently replace existing evidence. The original embedded-runtime workflow remains independently runnable with all original payload checks intact. Supplier, Qt-notice and Rust-source workflows can also be dispatched independently.

On the karaoke laptop, use the specifically identified source-built candidate and test actual MP3+CDG/H.264 library media, audio outputs, singer display and a real phone on the intended network. Enable the optional Cisco decoder only after reviewing its terms and restart before testing it. These are the remaining physical test outcomes.

The report does not certify organisational Microsoft licence eligibility, codec patent rights for a particular paid/non-personal use, rights in media, or unseen supplier build records. Those limits remain explicit rather than being marked as proved by CI. The Windows binary release remains draft-only.


## User-reported hardware test — 2026-10-25

Andrew Burgess confirmed that all three checks requested in the conversation passed:

- MP3+CDG and H.264 video song playback with sound.
- Lyrics/video displayed correctly on the singer screen.
- Phone/tablet remote connection and operation.

This is a hands-on user report for the source-built candidate most recently identified in the conversation, `OpenKJ-v9-Source-Candidate-016d5c3135bbc8073b96881e49e19feb97359101-40099935626-1.exe`. The installed checksum, precise hardware/network configuration, selected H.264 decoder and exhaustive media-library coverage were not separately supplied, so this record does not invent those details. No independent test log was provided.

This confirmation supersedes earlier statements that these three physical checks remained pending. It does not establish the separate licence/rights conditions or authorize publication. The release stays in draft.
