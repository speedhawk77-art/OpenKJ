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

## Verified candidate

[Download the bundled-helper test installer](https://github.com/speedhawk77-art/OpenKJ/releases/download/untagged-1680625e7f315aeedaac/OpenKJ-v9-Maintenance-Candidate-108c89718e78830f3977d779320626f9f9b0ced5-40486360620-1.exe) (251,580,173 bytes).

- Exact application/helper build source: `108c89718e78830f3977d779320626f9f9b0ced5`.
- Installer SHA256: `f0b4874f2864fc9387c94c90ac16c8200ac947c2baa630f30dd235f0ca22a81f`.
- [Windows verification run 40486360620](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40486360620) passed both jobs.
- 111 native host checks, 46 browser checks, 82 native YouTube checks, 52 maintenance checks, 22 codec checks and 20 source plus 20 installed HTTP checks passed.
- All 8 helper packaging/privacy tests and 11 real-helper checks before and after installation passed. Helper binaries were verified as Windows x64; static import auditing found no added OpenH264 dependency.
- Installer roundtrip matched all 376 program files and 11,703 notice files. Every one of the original 337 dependency programs is unchanged. Original embedded VC redistributable and both program icons remain verified.
- Matching helper sources: [OpenKJ-v9-YouTube-Helper-Sources-108c89718e78830f3977d779320626f9f9b0ced5.zip](https://github.com/speedhawk77-art/OpenKJ/releases/download/untagged-1680625e7f315aeedaac/OpenKJ-v9-YouTube-Helper-Sources-108c89718e78830f3977d779320626f9f9b0ced5.zip). SHA256: `7ec3d1610057fbc7311d22aa59c58a5a723e1a031bca5d1519009ed897e55537`.

The owner must sign into GitHub to download private draft assets. Exact asset IDs, hashes and check names are recorded in `community/windows/data/mobile-host-console-verification-v9.json`. A physical live YouTube download with the bundled copies is pending; prior console/search/rotation testing is already confirmed by the user.
