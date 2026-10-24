# Remote verification result — 24 October 2026

The original Windows installer remains unchanged in unpublished draft release 421204266. This report supersedes older pending-test statements only for checks explicitly evidenced below.

## Original failures and smallest repair

| Run | Exact observed failure | Cause |
| --- | --- | --- |
| [39880461825](https://github.com/speedhawk77-art/OpenKJ/actions/runs/39880461825) | `gh: Resource not accessible by integration (HTTP 403)` | Workflow token could not read the draft release; extraction never ran. |
| [39880549278](https://github.com/speedhawk77-art/OpenKJ/actions/runs/39880549278) | `Could not determine setup data version!`, exit 3 | Linux innoextract 1.9 could not parse the newer installer. |
| [39880728389](https://github.com/speedhawk77-art/OpenKJ/actions/runs/39880728389) | `Cannot open the file as archive`, exit 3 | 7-Zip could not inspect this Inno Setup archive. |

These access/parser failures did not prove corruption or an absent VC redistributable. The narrow parser repair is [ba7c271](https://github.com/speedhawk77-art/OpenKJ/commit/ba7c271): Windows 2025 plus native innounp 2.71.2, pinned to upstream revision and archive SHA-256. Current workflow explicitly grants `contents: write` for draft access and guarded draft-only evidence uploads. No personal token is embedded in source.

The audit still checks draft state, exact installer asset ID 667394901, installer SHA-256 `bc35c24ef25529b86b90bd3f5a215603de3b283d590620ec58820e02f83421ef`, fatal extractor errors, exactly one embedded redistributable, its 18,731,856-byte size and SHA-256 `843068991daaa1f73ad9f6239bce4d0f6a07a51f18c37ea2a867e9beca71295c`. [Initial repaired run 40065659931](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40065659931) passed and identified Inno Setup 7.0.0. No check was bypassed to obtain that result.

## Executed Windows checks

[Run 40070755772](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40070755772) passed with 394 installed notices. Observed results:

- A separate candidate retained 338 of the original 340 runtime binaries, with every retained hash unchanged. Only `libopenh264-7.dll` and `gst-plugins/libgstopenh264.dll` were omitted. There are 195 retained GStreamer plugins.
- Every retained PE file parsed. Zero normal/delay imports reference an omitted OpenH264 component. This does not exclude arbitrary dynamic loading.
- Raw audio, raw video and MP3 round-trip pipelines passed to fakesinks. The bare candidate's H.264 MP4 fixture failed with a missing plugin on the headless runner.
- A separate experiment downloaded pinned official Cisco OpenH264 2.6.0, required valid Cisco Authenticode and restored the original GStreamer plugin only inside the experiment. H.264 decoding passed. Neither Cisco's binary nor that plugin was added to the candidate installer or release.
- Software OpenGL created a context and identified Mesa 12.0.0-rc2 / LLVM 3.6.2. The packaged DLL matched Qt's checksum-pinned published Mesa RC2 binary byte-for-byte.
- All three GNU runtime DLLs matched the pinned Cerbero Windows MinGW 12 / GCC 14.2 toolchain archive byte-for-byte.
- A separate installer was built with checksum-pinned Inno Setup 6.4.3, re-extracted and silently installed. All 338 program and 394 notice hashes matched. Its embedded redist matched the original pin; omitted codecs stayed absent; the expected generated uninstaller was accounted for.
- The installed app started and passed 11/11 localhost HTTP checks, including public pages, unauthenticated access rejection, malformed requests and unknown routes.

Expanding the legal texts exposed real builder bugs: `COPYING.LIB` was filtered as a compiled library, and legal directories named `source` were pruned. Fixes [4c4a9a0](https://github.com/speedhawk77-art/OpenKJ/commit/4c4a9a0) and [64f419d](https://github.com/speedhawk77-art/OpenKJ/commit/64f419d) preserve those documents and check every notice hash before compilation. Build-output, binary and script filtering remains in place.

No physical audio, actual singer display, real phone or authenticated DJ operations were tested. A green diagnostic step is not proof every reported probe passed: the bare candidate's H.264 failure remains an explicit unresolved result.

## Fresh source build

[Build 40069867475](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40069867475) compiled source commit `c7b42022c37d8d628645fd2a369a326c8537aed3` with recursive submodules, Qt 5.15.2, pinned GStreamer 1.26.9 SDK inputs and bundled spdlog. MSVC 14.44 is selected explicitly because the runner's newer compiler removed an API used by these older Qt headers.

The resulting executable SHA-256 was `fb1ab3d77bcc26aa1dbce75119da1b39dbc032e9a24143ea9465ebdd3915ee79`. It also passed 11/11 HTTP checks. [Text build evidence](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40069867475/artifacts/12551085913) records source/tree/submodule and compiler identity. This proves the current source builds; it does not establish the historical draft executable's build commit or a reproducible byte-identical build. No replacement executable was published.

## Source and notice preservation

[Six-batch mirror 40069525753](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40069525753) passed, including toolchain source inputs and the Windows default manifest snapshot. Alternate official download mirrors retain the same checksum pins. The separate upstream notice collection preserves 239 original documents from 61 archives, including SPDX texts and SQLite's public-domain dedication header.

[Graphics mirror 40070755767](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40070755767) preserved Mesa 12.0.0-rc2 and LLVM 3.6.2 sources. Original legal texts are in the candidate. Matching Qt's binary identifies the supplier bytes; exact graphics patches/build settings are not thereby reconstructed.

[Locked Rust mirror 40070755763](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40070755763) preserved all 292 registry crates from librsvg 2.59.2's shipped Cargo.lock, verifying every archive against its lockfile checksum. Seven local workspace packages remain in the base source. The source bundle contains the vendor tree, per-file Cargo checksums and offline registry configuration. No Rust build script was executed.

It recovered 512 original Rust legal documents. Twenty-one crates have no standalone legal filename collected. Their complete source is preserved; applicable source-header attributions and notice completeness still require review. This limitation is recorded in the notice evidence.

| Draft asset | SHA-256 |
| --- | --- |
| OpenKJ-v9-librsvg-2.59.2-Cargo-Sources.zip | `dc45d766ef02a71bf8a6015b674e2e236171c9ff657aaeab51b01a802cb08c47` |
| OpenKJ-v9-librsvg-Cargo-Source-Manifest.json | `17f871e5433a1300990f2afa2741796a9271db167ae595137eadff78649a62d8` |
| OpenKJ-v9-librsvg-Rust-Notices.zip | `3082565fe65a2dcf6e80fb387ec89f92663bd9e7096d5626ff4365f1da0030ef` |
| OpenKJ-v9-Upstream-Notices-v2.zip | `e84daa9fb5afb05ba86508a52d67bd535082f4020d056551c3288363389f540a` |

The Mesa/LLVM source bundle SHA-256 is `03593861589b98eaa16b84fe09221029a53caafb4b6dc3f04a7c9106af945c1b`; its source manifest SHA-256 is `93e9da4c9aeb74d0ee8eb7eb3e121d16f04443107133223c9fbbcf47fced2b5b`.

All these assets remain in draft 421204266. Private draft assets are preparation for source delivery, not public corresponding-source delivery. The machine-readable manifests record pins and limitations.

## Final expanded installer audit

[Run 40071327999](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40071327999) tests commit `dea4c64c778701df61f11e80d485fd3133bfab32`, adding all 512 Rust documents and their evidence through strict safe-path, inventory and per-file checksum checks. Expected installed notice count: 907.

**Confirmed PASS:** all 338 installed program hashes and all 907 installed notice hashes match. Installed-app HTTP checks passed 11/11. Every step in this run succeeded.

Durable text evidence is draft asset 671517418, `OpenKJ-v9-Remote-Audit-40071327999-1.zip`, 76379 bytes, SHA-256 `83cdce57fd754d7370661e06e982d08d03084537493cd6af93c8a679579db88c`. A final GitHub API read confirmed `draft=true`, `published_at=null` and the original installer asset/digest unchanged.

The workflow retains a JSON-only evidence ZIP in the same draft with a unique run/attempt name, checks the uploaded SHA-256 and rechecks draft state. No installer, executable, codec, raw app log or user profile is included. Ordinary CI artifacts expire after 30 days; draft evidence/source assets remain until explicitly deleted.

## Rerun steps

1. Open [the verification workflow](https://github.com/speedhawk77-art/OpenKJ/actions/workflows/verify-openkj-v9-embedded-vc-redist.yml).
2. Select **Run workflow**, branch **offline-remote-v9**, then **Run workflow**. Dispatch the corrected branch; rerunning an old failed job reuses old code.
3. Inspect `verify-embedded-runtime`: redist identity, installer round-trip counts, actual JSON probe results and evidence upload. Inspect diagnostic results rather than assuming all media works from a green step.
4. Other independently dispatchable workflows are **Compile current OpenKJ v9 source and record build evidence**, **Mirror pinned transitive source packages to draft**, **Preserve identified Mesa and LLVM source and notices**, and **Preserve librsvg locked Rust sources and notices**. None requires the development laptop.

## Remaining release gates

- Decide and implement the final H.264 distribution approach. The Cisco experiment proves compatibility only. Its applicable end-user download, enable/disable control, attribution and full licence requirements must be implemented before production use. A separately downloaded codec affects a completely offline first-use requirement.
- Finish source/build correspondence and original notice review, including crate headers, exact graphics build changes and historical executable provenance.
- Establish applicable redistribution rights and a durable public source-delivery plan before publishing the binary.
- On the karaoke hardware, test real library media including MP3+CDG and H.264, actual audio outputs, singer/second display, and a real phone on the intended network.

The existing draft installer has not been replaced or published. Remote checks cannot establish these physical test outcomes.
