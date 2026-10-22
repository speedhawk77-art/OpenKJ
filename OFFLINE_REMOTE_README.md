# OpenKJ - Offline Remote Community Edition (Windows v9)

**Unofficial, independently modified community fork of [OpenKJ](https://github.com/OpenKJ/OpenKJ). Not an official release from the OpenKJ maintainers.**

## What is new

- Local HTTP DJ remote on phones/tablets: karaoke and break-music controls, volume, live playback status, song searching, queued song selection and direct playback.
- Singer rotation: add, select, remove (with confirmation), and move a singer up or down.
- Public singer songbook at `/request`: search the DJ's local library and submit requests for DJ approval or rejection.
- Show a scannable singer-request QR code on the DJ remote phone.
- **Now singing** displays the actual current performer/song, separately from the next queued song.
- Windows single-file setup built with Inno Setup and a bundled launcher for Qt/GStreamer DLL/plugin paths.

## Usage

1. Start OpenKJ from the installed **OpenKJ** shortcut.
2. Connect the laptop, DJ phone and singers' phones to the same **trusted private Wi-Fi**. The Wi-Fi does **not** need Internet, but it must allow devices to communicate directly (disable client isolation).
3. In OpenKJ's status bar, read the remote address and six-digit DJ PIN. Example `http://192.168.1.20:8081/`.
4. Enter the PIN on the DJ remote. Do not give the DJ PIN to guests.
5. On the DJ remote, tap **Show Singer Request QR Code**. Guests scan it, search local songs and send requests, and the DJ approves/declines them on the private DJ page.

OpenKJ may select a different port if 8081 is occupied. This is **not** the separate online `okjsongbook.com` request-server integration; leave the online server disabled if only using the offline system.

## Technical & security limitations

- This is a **local-network** HTTP server, **not** an Internet-facing product. No TLS; requests and the DJ PIN are visible to someone capable of intercepting traffic on the LAN. Only use a trusted private Wi-Fi network; do not forward TCP port 8081 to the Internet.
- The DJ control endpoints require a per-session PIN. Guest request endpoints are intentionally accessible without a PIN to other local-network clients. The guest flow has request limits and operator approval, but has not undergone an independent security audit.
- Guest singer names and requests are saved locally to OpenKJ's SQLite profile, and can persist across sessions.
- The Windows software is unsigned and may trigger SmartScreen. Verify the release source and installer origin before running.
- This was tested on two Windows computers by the community maintainer, including karaoke playback and local phone requests. Hardware, codecs and networking vary.

## Building from source

The fork contains the complete corresponding changes directly in the OpenKJ source tree; no patcher is needed when building from this branch.

Clone recursively, configure Qt 5.15.2 for MSVC and GStreamer, and build with a compatible Visual Studio C++ toolchain. The following was used by the original community tester:

```bat
git clone --recurse-submodules https://github.com/speedhawk77-art/OpenKJ.git
cd OpenKJ
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -T v143 -DCMAKE_PREFIX_PATH="C:/Qt/5.15.2/msvc2019_64" -DSPDLOG_USE_BUNDLED=ON
cmake --build build --config Release --parallel 2
```

See `community/windows/` for the portable bundling tool, one-click launcher source, and Inno Setup builder. Their default Windows paths may need adjustment on other machines. The Windows EXE is not a standalone binary; use the complete installer or bundle its dependencies.

## Licensing & attribution

This is derived from OpenKJ. The original OpenKJ copyright and GPL-3.0 license apply; see the upstream `LICENSE` file. The QR-code library and Windows dependency stack have additional obligations and attributions; see `THIRD_PARTY_NOTICES.md` and `community/licenses/`.

No songs, karaoke media files, personal databases, user PINs, or personal settings are included in the source distribution.

Upstream: https://github.com/OpenKJ/OpenKJ
Community fork: https://github.com/speedhawk77-art/OpenKJ
