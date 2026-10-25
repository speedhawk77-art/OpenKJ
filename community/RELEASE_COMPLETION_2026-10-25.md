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

## Final execution results

Actions was re-enabled by the owner. The remaining remote checks completed:

| Check | Successful run | Result |
| --- | --- | --- |
| Retained-plugin source/notice mirror | [40222018007](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40222018007) | 753 source inputs and 8,230 notice/header documents preserved; output ZIP hashes match local results |
| Exact Microsoft runtime and linked terms | [40222179367](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40222179367) | Embedded redist identity and linked original terms verified; 18 documents preserved |
| Refreshed installer | [40222179361](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40222179361) | All 339 program files match the preceding user-tested candidate; 9,596 installed notice hashes and 11 installed HTTP checks pass |
| Clean release draft | [40222885879](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40222885879) | Exactly 41 assets; all names and SHA-256 digests independently compared against the selection and calculated checksum file |

Normal/delay import checks, isolated playback pipelines, software renderer and
supplier/toolchain checks, optional Cisco codec compatibility, embedded runtime
identity, actual installer installation and installed-file checks all passed.
The installer returned exit code 0. No program binary changed from the baseline;
this carries the previous candidate's user-reported hardware result without
claiming an additional physical test of this documentation-only refresh.

The Microsoft workflow's first upload attempt lacked GH_TOKEN in that step;
that setting was corrected. Its archive timestamps are now deterministic.
The first clean-draft attempt created the draft but a tag-based lookup returned
404 for the unpublished release; preparation now uses numeric release IDs for
creation responses and uploads. The successful reruns above supersede those
failed preparation attempts; no verification check was bypassed.

## Ready draft and exact installer

- [Clean draft release 423591307](https://github.com/speedhawk77-art/OpenKJ/releases/tag/untagged-0bda9e68a675f5c69a51), planned public tag `v9.0-offline-remote.2`.
- Installer: `OpenKJ-v9-Source-Candidate-016d5c3135bbc8073b96881e49e19feb97359101-40222179361-1.exe`.
- Size: **206,894,379 bytes**; SHA-256 **`7accc965efcc2a7f13df5699cc66bdd0c5f186cf09c829f737d77c89d62a1c12`**.
- Installer asset in the clean draft: **675832181**.
- `SHA256SUMS.txt` SHA-256: `4d8c89863996843219ce468c7274319c1e8250ee76cc4f903d884dba4fe01aef`.
- Exactly one installer, 39 source/notice/evidence assets and the checksum file.
- Exact asset IDs, sizes, hashes and test scope: `windows/data/verified-clean-draft-v9.json`.

The owner can download this draft while signed in. All required selected source
archives accompany it, but draft assets remain private until publication. The
release was independently confirmed **draft=true, published_at=null**. No
publication action has been taken.

Do not publish working draft 421204266 wholesale: it retains historical
installers, including the old SDK-built OpenH264 payload. Those assets remain
private for audit history. The separate clean draft excludes those installers.

## Reproduce the final checks

Use branch `offline-remote-v9` and the Actions **Run workflow** controls:

1. `mirror-openkj-v9-retained-plugin-sources.yml` reproduces and verifies the
   source and notice archives. Existing matching assets are retained; conflicting
   assets fail rather than being replaced.
2. `inspect-openkj-v9-microsoft-terms.yml` reproduces the Microsoft evidence.
3. `verify-openkj-v9-source-candidate.yml`, source commit
   `016d5c3135bbc8073b96881e49e19feb97359101` (blank-input default), repeats all
   program, codec, installer and installed notice/HTTP checks. Any program change
   fails the 339-file baseline guard.
4. A successful candidate run automatically starts clean-draft preparation.
   It can also be run manually with the successful candidate run ID. A different
   candidate will not overwrite a conflicting existing draft or source asset.

The engineering and packaging checks listed here are complete. Publication is
an owner decision, and the sources must remain available with the installer.
These results do not claim independent reproduction of every upstream binary,
exercise of every codec/hardware configuration, or a legal certification.
