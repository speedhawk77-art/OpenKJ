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
