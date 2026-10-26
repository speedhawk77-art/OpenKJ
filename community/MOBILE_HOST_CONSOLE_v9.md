# Mobile and tablet host console

Built on the user-tested YouTube download-first branch, retaining the classic host
remote at `/classic`, public singer requests at `/request`, and QR display at `/qr`.
The root page is the new dark, blue-accented touch console. All assets are embedded
in OpenKJ and served locally. No browser audio/video, lyrics preview or desktop sharing.

## Stages

1. Native host adapter and PIN-protected APIs: read live state without moving the
   laptop's selected singer; validate queue/rotation/settings revisions; reuse
   OpenKJ's models, widgets, media backend and existing download controller.
2. Responsive workspace: phone bottom navigation; tablet search/singers/queue panels;
   transport, seek, volume, key/tempo; host tools and applicable show settings.
3. Real native/model/HTTP tests, DOM and viewport checks, then the existing Windows
   build, exact-source packaging, installer and dependency/licence/icon audits.
4. Install the resulting test build on the laptop and test from actual devices.

## Functions

- Paginated local song browsing/search by all words, artist, title, disc ID, popularity
  or selected singer history. Add songs using their stable local library IDs.
- Add singers with native fair/bottom/next placement; browse saved names; rename,
  remove, reorder, track regular singers and choose the rotation top.
- Inspect each singer's queue independently on each device. Reorder, remove, clear,
  mark played/unplayed, save per-song key changes, play an exact entry or next song.
- Karaoke play/pause, stop, next singer, seek, key −12…+12, tempo 50…200%, volume/mute.
- Break playback/seek/volume/fades, playlist selection, search/add/move/remove tracks.
- YouTube download/progress/cancel and explicit verified local import; guest request
  approval/decline; QR display; already-configured sound clip playback.
- Auto-advance, fades, crossfade/autostart, mono/multiplex, EQ, singer display and
  fullscreen, ticker text/speed/rotation; enable configured recording between songs.

## Safety and compatibility

All host APIs require the startup PIN. Repeated wrong PIN attempts are limited
per client address; a valid PIN remains usable. Invalid credentials stop browser
auto-retries until the host enters a PIN again. Mutations require POST and the current host
session. Stable IDs and the displayed revision protect queue/rotation/settings edits
against other devices. Song changes invalidate outstanding seek/tuning revisions.
The phone confirms disruptive actions; the native adapter suppresses only the duplicate
interruption/pause confirmation for that one confirmed action. Normal laptop warnings
remain enabled. Import, media verification and optional-codec consent are preserved.

The classic APIs remain available. Synchronization polls once a second, pauses while
the browser is hidden, refreshes after actions and reconnects after Wi-Fi loss. Search
is paginated (50 results), debounced and cancellable; metadata/history revisions
refresh visible results after laptop changes even when the catalog count is unchanged.
Saved singer names have their own searchable, paged picker. No full 76,000-song catalog is
sent to the phone. Updates use DOM text/value properties for external names/titles.

## Laptop-only setup and limits

OS audio devices/drivers, monitor assignment, source/recording/helper paths, codec
terms, file import/export, bulk library maintenance, account/API credentials and
background image/file pickers stay in the laptop's native setup dialogs. The remote
never accepts executable paths or arbitrary local file paths. Recordings use the
already-configured input/output. A queue over 2,000 entries is reported explicitly
and must be managed on the laptop. Native media errors (e.g. a corrupt local karaoke
archive) can still require attention to OpenKJ's error dialog on the laptop.

This is a staged test candidate, not a public release. Physical phone/tablet playback,
queue editing, simultaneous-device synchronization and reconnect tests remain required.

## Laptop, phone and tablet acceptance test

1. Close OpenKJ and install the verified candidate on the laptop using the existing installation and saved configuration. Reopen it and read its remote address and current PIN from the status bar.
2. On the same local Wi-Fi, refresh that address on the phone and enter the PIN. No phone app, Node server or desktop-sharing program is required. Check Library, Singers, Queue, Tools and Settings in the bottom tabs.
3. On a tablet, check portrait and landscape layouts. Landscape shows library, singers and queue together; narrower tablet layouts rearrange the panels.
4. Add a test singer and two known-working local songs. Reorder songs, edit a saved key, mark played/unplayed and rename/reorder the singer. Check changes in OpenKJ and on a second device. Each device can inspect a different singer without changing the laptop selection.
5. Play a song, then test pause/resume, seek, volume/mute, key/tempo and stop. Listen/watch on the laptop's outputs; the phone/tablet plays no media.
6. Check break music, guest request approval/decline and a configured sound clip. In Tools, repeat YouTube download/verify/import using the existing helpers and H.264 setup.
7. Edit library metadata on the laptop and verify visible phone results refresh. Check a ticker/show setting and an EQ band in both directions.
8. Disconnect/reconnect one device's Wi-Fi while the laptop continues running. Check status reconnects and failed actions are not automatically replayed.
9. The existing remote is available at the same base address plus `/classic`. Singer requests and QR keep their existing routes.

Physical-device results remain separate from the automated tests.

## Verified Windows candidate

[Download the mobile host console test installer](https://github.com/speedhawk77-art/OpenKJ/releases/download/untagged-1680625e7f315aeedaac/OpenKJ-v9-Maintenance-Candidate-958a56747230883675a8cc90a8d8f6296e834ec7-40419895167-1.exe). The download is in the owner's private working draft; sign into GitHub if needed.

- Exact compiled source: `958a56747230883675a8cc90a8d8f6296e834ec7`.
- Installer SHA256: `d7bc5cc1316be4503ead480b6fc20fcbeee786fe9a59475b70f96bbc7e7791f5` (207,053,833 bytes).
- [Windows verification run 40419895167](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40419895167) passed: 107 native host checks, 38 browser checks, 76 existing download checks, 52 maintenance checks, 22 codec checks, and 20 source plus 20 installed HTTP checks.
- Native tests used a synthetic 76,010-song catalog. Browser checks covered 320/390/600/768/1024/1366-pixel layouts, two-device queue inspection, reconnect behaviour, metadata refresh, saved-singer paging and rapid tuning taps across a held network response.
- Installer roundtrip matched all 339 programs and 9,597 notices; all 337 dependency programs are unchanged. Both EXEs retain all ten original icon sizes; the exact embedded VC redistributable remains verified.

Exact assets and check names are recorded in `community/windows/data/mobile-host-console-verification-v9.json`. The matching application source and complete audit ZIP are retained with the installer. This remains a draft candidate for physical phone/tablet testing, not a public release.

## Multiword remote search repair

The phone uses `URLSearchParams`, which encodes spaces as `+`. Qt's generic URL query parser preserves that character, so the original host adapter searched for literal plus signs. Host GET parameters now replace raw `+` with spaces before percent decoding exactly once. Encoded literal plus signs and percent signs remain literal. Authentication and existing classic/guest routes are unchanged.

Eight actual local HTTP regressions cover multiple words, percent-encoded spaces, apostrophes, repeated spaces, words across artist/title, literal plus signs, decoding exactly once, and history searches with spaced singer names. All passed in the Windows run above. Install this corrected candidate on the laptop, reopen OpenKJ, refresh the phone remote and re-enter its current PIN; retry a known title with two or more words.
