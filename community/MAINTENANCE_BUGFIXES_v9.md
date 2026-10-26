# OpenKJ v9 maintenance candidate

This branch prepares a separate update to Andrew's published Offline Remote v9.
It retains Qt 5.15.2, the existing SDK runtime and the offline remote/optional
codec features. The public installer is not replaced by this work.

Changes:

- Equalizer sliders capture their own band index; the backend rejects invalid indices.
- Existing singers reload history correctly; model resets match the loaded rows.
- CD+G scrolling uses reusable byte buffers and overlap-safe copies. Invalid fine
  offsets are clamped, and scrolling left fills the newly exposed right edge.
- Playback teardown releases its retained audio/video bin references once.
- Recording releases messages once and finishes encoded files through EOS.
  Finalization is bounded, its timer is cancelled on completion, paused recordings
  drain, and the next recording can wait for the previous file to finish.
- Ticker workers render QImage instead of QPixmap, synchronize their shared state,
  discard stale renders and stop before their objects are destroyed.
- Software video rendering validates frame format and mapped layout, uses the
  actual row stride and holds the sample until its final QImage copy is released.

Source credits are in `licenses/maintenance-v9/UPSTREAM_FIX_CREDITS.txt` and are
included in the installer. No Qt 6 migration or unrelated fork features are merged.

The dedicated Windows workflow builds the app and launcher from the same commit,
executes the actual-class maintenance regressions and existing codec integration
tests, then performs the existing import, media, runtime-origin, installer round
trip, installed-notice and HTTP checks. Only the two source-attested application
executables may change; all 337 dependency paths and hashes must match released v9.

Successful test installers and matching application sources are held in the
existing private working draft. Publishing an update is a separate action after
review of its results and a hands-on playback/remote check of the changed program.

## Executed Windows verification

[Run 40239122777](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40239122777)
passed for exact application source commit `39da17fed3644b8ae93d5e114fb77892b8da6655`.

- **52/52 native maintenance regressions** passed. The app and test executable share
  the same compiled application objects. Recording tests use the production encoder
  graph with only the microphone replaced by a synthetic source.
- **22/22 optional-codec tests**, **11/11 source-app HTTP checks** and **11/11
  installed-launcher HTTP checks** passed.
- All **337 dependency paths and hashes** match released v9. Only the two
  source-attested application executables change; runtime inventory remains 339.
- Installer extraction, exact embedded VC runtime, installation (exit 0), all
  **339 installed program hashes** and **9,597 installed notice hashes** passed.
- Existing import, media-pipeline, renderer, compiler-runtime and separately
  downloaded Cisco compatibility checks passed.

Installer: `OpenKJ-v9-Maintenance-Candidate-39da17fed3644b8ae93d5e114fb77892b8da6655-40239122777-1.exe`.
Size: 206,896,527 bytes. SHA-256:
`8851c2aaf04085f0a6802da6645c268cf8e69a3140707322f5ade406b6d5466d`.
The installer and matching application source are retained in private working
release 421204266. Full asset identities are in `windows/data/maintenance-verification-v9.json`.

The first test build exposed a duplicate QString streaming helper in the test
harness. Removing that duplicate and sharing the production objects resolved the
linker error. The successful run includes the final product code and all tests;
no failing check was bypassed.

The existing public installer remains available. Before publishing this changed
application, check playback/singer display, EQ, history refresh, ticker,
phone/tablet remote and recording if used on the karaoke machine. No physical
hardware test of this maintenance candidate is claimed by the automated results.

## Owner testing and public release

After testing the maintenance candidate, the owner reported “all seems to be working”
and explicitly approved publication. This is an owner report, separate from the
automated results above; it does not imply exhaustive hardware or media coverage.

The exact tested installer and matching sources are published as
[`v9.0-offline-remote.3`](https://github.com/speedhawk77-art/OpenKJ/releases/tag/v9.0-offline-remote.3).
The release asset pins are in `windows/data/maintenance-publication-v9.json`.
The previous public release and private historical working draft are retained.
