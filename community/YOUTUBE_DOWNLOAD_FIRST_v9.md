# YouTube download-first test feature

This experimental feature adds **Tools → Download YouTube video…** to the host's
OpenKJ window. It downloads one video, checks the completed local audio/video file,
and lets the host confirm its artist/title before adding it to the normal library.
It does not start playback, add a singer, change a queue or accept remote download
commands. Existing local playback and the phone remote operate as before.

## One-time helper setup on Windows

Install these helpers separately from their original projects. They are not
included in the OpenKJ installer or its audited dependency directory.

1. Download the official Windows x64 `yt-dlp.exe` from
   <https://github.com/yt-dlp/yt-dlp/releases/latest>. Its official standalone
   executable includes the YouTube challenge scripts. Use the current executable,
   not a third-party repackaging or a script wrapper.
2. Install `ffmpeg.exe` using the Windows build links supplied by the FFmpeg
   project at <https://ffmpeg.org/download.html#build-windows>. Extract the archive;
   the actual program is usually in its `bin` directory. It joins the downloaded
   video/audio tracks without transcoding them.
3. Install the Windows x64 **Deno** executable (`deno.exe`, not `denort.exe`) using
   <https://docs.deno.com/runtime/getting_started/installation/>. yt-dlp's current
   guide requires Deno 2.3.0 or later:
   <https://github.com/yt-dlp/yt-dlp/wiki/EJS>.
4. In OpenKJ's download window, use Browse to select each of those three `.exe`
   files and a folder for completed videos. OpenKJ remembers these choices after
   a valid download starts. Helpers already on PATH are detected automatically.

Use each helper under its own original terms. Download and use videos you are
authorized to save and use. OpenKJ does not obtain content rights for the user.
The feature does not use browser cookies, sign into YouTube, bypass restricted
videos or update executable programs automatically. You can update the external
helpers yourself from their original projects when YouTube changes.

## Download and import

1. Find a karaoke video on YouTube and copy its video link.
2. Open **Tools → Download YouTube video…**, paste the link and click **Download video**.
3. Wait for **Video saved and checked**. Downloads use H.264/AAC MP4, up to 1080p,
   with both audio and video. Live/upcoming streams and playlist downloads are excluded.
4. Confirm the title and enter the artist or karaoke channel, then click **Add to library**.
5. Search the library and add the local song to a singer's queue as usual.

Downloads need internet; the completed local video can play offline. There is no
direct streaming or automatic vocal removal. Local files use the existing OpenKJ
playback path, including the existing key/tempo controls; their behavior still
needs a hands-on check with the downloaded video on the karaoke laptop.

## Failure and cancellation behavior

Only real single-video YouTube links are passed to the downloader, using argument
arrays without a command shell. User/portable yt-dlp configurations and third-party
plugins are disabled for these jobs. Completed metadata is read from a dedicated
JSON file, separate from progress output, and its source ID/path are checked.

Incomplete downloads remain in a temporary cache, outside the selected music
folder. GStreamer validates that the finished file has both audio and video and
a usable duration before a complete MP4 appears in the destination. Existing
files are never overwritten; repeat downloads receive a numbered suffix.

Cancel stops the downloader and its helper processes. Copy/validation runs in a
worker and supports cancellation. Network attempts have bounded timeouts/retries,
and an overall 15-minute limit stops stalled jobs. Closing the download window
cancels an active download. Closing OpenKJ also stops its download helper processes.

Adding to the library uses bound SQL and the existing schema. If a watched folder
indexed the completed file first, import updates that row's chosen metadata without
duplicating the song or resetting its existing play history.

## Verification scope

The native test runs the real controller, media validation, dialog and SQLite
import using a synthetic helper and a generated one-second H.264/AAC fixture.
It tests success, invalid links, missing helpers, failures, malformed metadata,
outside paths, corrupt/silent files, duplicate names, responsive UI, process-tree
cancellation, timeout and explicit library import. No real account or user media
is accessed. Passing these tests does not claim a live YouTube download or physical
playback test. A dedicated Windows workflow also runs all existing maintenance,
codec, runtime-origin, icon, installed-notice, installer and remote HTTP checks.
