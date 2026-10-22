# OpenKJ - Offline Remote Community Edition v9 (Windows x64)

**UNOFFICIAL COMMUNITY BUILD** based on [OpenKJ](https://github.com/OpenKJ/OpenKJ). This is **not** an official OpenKJ maintainers' release.

## Features

- Fully offline phone/tablet DJ controls for karaoke and break music
- Live performer/song display with correct **Now Singing** vs **Up Next** details
- Singer queue search, add/remove singer, change rotation order, select and play queued songs
- Guest song requests with DJ approval/decline and a shareable QR code
- One-file Windows installer with local runtime DLLs and OpenKJ launcher

## Requirements

- 64-bit Windows and a network allowing phone-to-laptop communication
- Each phone must join the same trusted local Wi-Fi; no Internet connection needed
- Karaoke songs and OpenKJ database **not included**

## Installation and caution

Download `OpenKJ-v9-Setup.exe` from this release only after checking the publisher and checksum, run the installer, and use the **OpenKJ** shortcut. If the Windows installation already has OpenKJ, back up `%LOCALAPPDATA%\OpenKJ` first; the community fork and original may share a user data directory.

This application exposes an unaudited local HTTP service. Use it only on a trusted private LAN, never port-forward it, and keep the six-digit DJ PIN private.

**Corresponding source code** is this GitHub fork on the `offline-remote-v9` branch at the tag corresponding to this release. Original project and GPL-3.0 copyright/attribution remain intact. Third-party runtime licensing must be verified before a draft binary release is made public; see `THIRD_PARTY_NOTICES.md`.

Original OpenKJ project: https://github.com/OpenKJ/OpenKJ
