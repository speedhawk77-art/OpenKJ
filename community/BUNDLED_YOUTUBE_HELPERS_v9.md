# Bundled Windows YouTube helpers

The Windows test installer includes yt-dlp 2026.10.26, FFmpeg 8.1.3 and Deno 2.9.8
under `helpers/youtube`. No system PATH, Python installation, file association or
administrator-only helper setup is required after installing OpenKJ.

Fresh configurations find the bundled helpers automatically. Existing valid custom
helper paths take priority. Missing custom paths fall back to the bundle. The laptop's
YouTube download dialog has a **Use bundled helpers** button; the host can still
select external helper executables. Helper choices are saved when a download starts,
as before. The phone host console uses that same dialog/controller configuration.

The downloader is the unmodified official yt-dlp zip application, launched by a small
OpenKJ executable with a private, unmodified CPython 3.14.9 embeddable runtime. This
avoids bundling the optional curl-cffi, mutagen and PyInstaller dependencies of the
upstream Windows one-file executable. The official EJS JavaScript support is included.
The launcher preserves arguments/standard streams/exit codes and stays inside OpenKJ's
existing Windows Job Object so cancellation also stops Python, Deno and FFmpeg.

FFmpeg and ffprobe are compiled from the checksum-pinned FFmpeg source with MSVC,
static runtime, no external codec libraries, no GPL/nonfree/version3 components,
no network protocols, and only the MP4/MOV stream-copy functions needed by the
existing H.264/AAC download selection. They do not replace any Qt/GStreamer playback
DLL or the separately consented Cisco codec. External full-featured FFmpeg paths
remain supported.

`community/windows/helpers/inputs.json` pins binary/source input hashes.
`notices.json` pins additional V8/Rust notices. The helper build preserves original
licences and source archives (including Deno's Cargo.lock registry dependencies) as
a superset; it does not claim every locked dependency is linked into the Deno binary.
The installed `licenses/youtube-helpers` directory identifies the matching source ZIP.
For eventual publication, deliver that exact helper-source ZIP beside the matching
OpenKJ source and installer. Do not publish the entire historical working draft.

The workflow checks the original 337 dependency programs unchanged before and after
adding helpers. It accepts only the separately attested helper paths/hashes, verifies
all PE programs are x64, tests the actual tools offline with synthetic H.264/AAC media,
checks argument quoting in a path with spaces, and verifies every helper file again
after installation. The original embedded VC runtime, icon, codec, notice and native
playback regression checks remain required.

## Laptop acceptance

1. Close OpenKJ, install the new candidate, and reopen it.
2. Existing helper selections keep working. To explicitly test the included copies,
   open the laptop's YouTube download dialog, click **Use bundled helpers**, and keep
   your existing destination folder.
3. Download one known-working authorised test video from the laptop or phone host
   controls. Check verification/import, then local playback and key/tempo controls.
4. Confirm no separate helper downloads or file browsing were needed. Downloads
   still require internet; ordinary library playback and the local remote remain
   usable offline.

No public release is automatically promoted by building this candidate.
