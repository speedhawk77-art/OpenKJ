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
