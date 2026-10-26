# Host remote YouTube downloads (v9 test candidate)

This extends the download-first feature. Andrew confirmed live download, library import,
playback, tempo and key changes on his Windows laptop on 26 October 2026 after enabling
Help → Optional H.264 codec. This confirmation applies to the preceding desktop candidate;
the new phone controls still need a phone/laptop test.

## Use

1. Install the new host-remote candidate on the laptop. Keep the existing helper programs,
   save folder and optional H.264 configuration. If not configured, download once using
   Tools → Download YouTube video… on the laptop to save these settings.
2. Open the usual local remote address on the phone, refresh the page, and enter the
   laptop's current session PIN.
3. In **YouTube Downloads**, paste one video link and tap **Download video**.
4. Follow progress on the phone. The laptop must have internet; the phone only needs
   access to the laptop's local remote. **Cancel download** stops that download.
5. After verification completes, confirm the artist and title and tap **Add to library**.
6. Select a singer and use **Find & Add Songs** to queue the imported title. Playing it
   remains a separate host action.

The phone uses the downloader programs and destination configured on the laptop. It
cannot set executable paths, change the save directory, import an arbitrary local file,
or supply shell commands. Desktop and remote share the same controller and pending
import. Closing the desktop download window cancels its current download.

## Verification scope

The same strict download, path, media, duration and database checks remain in force.
New POST commands (`youtube_download`, `youtube_cancel`, `youtube_import`) are routed
only after host PIN authentication; the guest request page has no download controls.
Cancel and import require the current random download session identifier, preventing
late commands from another tab acting on a newer video. Import does not change singer,
queue or playback. Titles and status are rendered with DOM text properties.

Automated tests exercise the actual HTTP server, downloader/controller, media validation,
dialog and SQLite import with synthetic helper/media fixtures. Unauthorized and GET
commands, guest isolation, busy downloads, stale sessions, invalid metadata, duplicate
import and cancellation are covered. Existing Windows installer/runtime/license/icon
checks are retained. A physical phone test is still required before release approval.

The first host-remote Windows build passed 76 native download/remote checks, 52
maintenance checks, 22 optional-codec checks and 14 HTTP checks. Packaging's HTTP
count is explicitly set to 14 for this candidate; legacy callers retain their original
11-check requirement. The installer must also pass the full roundtrip before delivery.

The optional DOM test can be run with Node and jsdom 30.1.3 installed in an isolated
test folder: set NODE_PATH to that folder's node_modules, then run
`node community/windows/TEST_HOST_REMOTE_UI_V9.cjs <report.json>`. It checks actual
embedded page interactions with a simulated server; it does not test browser layout
or real phone/laptop connectivity.
