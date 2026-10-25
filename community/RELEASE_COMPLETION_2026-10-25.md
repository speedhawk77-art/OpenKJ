# Release completion — 2026-10-25

## Current candidate

Andrew reported that MP3+CDG, H.264 with sound, singer display and phone/tablet
remote tests passed. This records his hardware report, not an independently
captured file inventory.

The selected installer is asset `672399791`, filename
`OpenKJ-v9-Source-Candidate-016d5c3135bbc8073b96881e49e19feb97359101-40099935626-1.exe`,
SHA-256 `8ad438cc61c212377ab5b4999f5ed0644b8ff26fcbda413a4412c98524351228`.
[Run 40099935626](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40099935626)
verified 339 installed program hashes, 1,360 notice files and 11 installed HTTP
checks. [Supplier audit 40099815324](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40099815324)
established 336 direct Qt/GStreamer package matches and accounted separately
for the two source-built executables, Microsoft D3D compiler and Qt renderer.

## Additional source and notice coverage

The previous 79-DLL source mapping did not include the dependencies compiled
inside retained Rust plugins or several statically linked/header dependencies.
The new source-only mirror reads the exact upstream `gst-plugins-rs` lockfile
and pinned Cerbero recipes, without running upstream build code.

Local execution verified 734 Cargo registry checksums, five pinned Git source
snapshots, and seven recipe-pinned dependencies: libvpx, soundtouch, qrencode,
webrtc-audio-processing, DirectXMath, DirectX-Headers and LADSPA. Five main
GStreamer module sources are included for their vendored header notices.
The resulting archive contains 753 input archives including Cerbero and the
Rust plugin workspace. It preserves 8,230 distinct original legal, metadata
and source-header documents, with all original locations in the manifest.
All 8,230 passed the existing strict notice-supplement inventory/hash validator.
This is source and notice preservation, not a claim that every optional crate
was compiled or that every upstream binary was independently rebuilt.

| New local output | SHA-256 |
| --- | --- |
| `OpenKJ-v9-Retained-Plugin-Sources.zip` | `ad8b9b160c80b3a642e88cdf51ded6b8b873b6677f75ea638c27710adb27f9cc` |
| `OpenKJ-v9-Retained-Plugin-Notices.zip` | `2f4c0feebe91b8a1c42d20370288446ae73acec4290a8cc3b9257477632b5a3a` |

The Microsoft runtime's actual Burn metadata points to
`https://aka.ms/VCRedistLicense`; it does not supply an embedded English EULA RTF.
The corrected inspection preserves that metadata and the original linked
Microsoft DOCX (SHA-256
`08651651a7602fc7c0e2763de0fde1ff9f868df2780597cd1775ee9d6441c783`).
Local validation succeeded against the exact pinned redist and preserved 18
documents. Runtime text, SDK terms and provenance are now checked in and
staged as Microsoft-specific component terms. No agreement was accepted on
anyone's behalf.

## Remaining execution and publication gates

GitHub's Actions page currently states: "Workflows are not currently being run
on this fork due to the scale of GitHub Actions usage." A repository maintainer
must re-enable Actions through GitHub's offered control. API access to files
does not override this restriction. Do not evade the pause with another fork
or account.

After re-enabling, on branch `offline-remote-v9`:

1. Run **Preserve retained GStreamer plugin sources and notices**
   (`mirror-openkj-v9-retained-plugin-sources.yml`). It uploads the three new
   source/notice outputs to working draft 421204266, refusing conflicting assets.
2. Run **Inspect embedded Microsoft runtime licence documents**
   (`inspect-openkj-v9-microsoft-terms.yml`). It extracts the exact redist,
   checks its embedded license link, downloads the pinned original terms and
   preserves the evidence in the working draft.
3. Run **Verify source-built OpenKJ v9 installer candidate**
   (`verify-openkj-v9-source-candidate.yml`) with source commit
   `016d5c3135bbc8073b96881e49e19feb97359101` (now the blank-input default).
   The new plugin notices, Microsoft terms, source access index and replacement
   guide are included. A new guard requires every one of the 339 program paths
   and hashes to match the user-tested baseline. Existing import/media probes,
   embedded runtime identity, installer round trip, installed notice hashes and
   HTTP checks remain required. A changed program fails rather than inheriting
   the hardware pass. Installer layout/installation is still checked in CI.
4. Run **Prepare clean public-release draft** with that successful candidate
   run ID. It copies only pinned source/notice assets and that verified installer
   to a separate draft. It never publishes or replaces the working draft.
   Review the resulting draft and source download index before publication.

Do not publish draft 421204266 wholesale: it retains historical installers,
including the old SDK-built OpenH264 payload. Those historical assets are kept
private for audit history. The new draft must contain one verified installer,
matching checksums, notices, source archives and matching release notes.

The remaining checks cannot be marked executed while Actions is paused. No
new installer has been rebuilt, installed or uploaded by these local checks.
No release has been published. A lawyer's certificate or exact reproduction
of all upstream binaries is not being imposed as a new blanket release gate;
specific unresolved licence obligations must still be resolved on their facts.
