**Unofficial community release.** This is Andrew's separate OpenKJ community edition, not an official release from the OpenKJ maintainers.

OpenKJ Offline Remote v9 for Windows x64 provides local karaoke playback, a singer display and phone/tablet remote operation on the same local network.

This maintenance update fixes EQ slider handling, singer history loading, CD+G scrolling, playback resource cleanup, recording finalization, ticker threading and software video rendering. Credits for adapted upstream fixes are installed under `licenses/maintenance-v9/UPSTREAM_FIX_CREDITS.txt`.

The installer below is the exact maintenance candidate tested by the owner, who reported that it is working. Automated verification passed 52 native regression checks, 22 optional-codec checks, 11 source application HTTP checks and 11 installed launcher HTTP checks. Installation succeeded, with all 339 program-file hashes and 9,597 notice-file hashes verified. The 337 dependency files are unchanged from the preceding public release; only the two source-built application executables changed. These results do not claim testing every media format or hardware configuration.

Application and launcher source: [`39da17fed3644b8ae93d5e114fb77892b8da6655`](https://github.com/speedhawk77-art/OpenKJ/tree/39da17fed3644b8ae93d5e114fb77892b8da6655). The matching `OpenKJ-v9-Application-Source-39da17fed3644b8ae93d5e114fb77892b8da6655.zip`, dependency sources, recipes, notices and `SHA256SUMS.txt` accompany this release. The maintenance audit ZIP contains the automated verification records; its hardware-test fields describe the earlier automated run, before the owner's subsequent hands-on report.

[Successful build and installation verification](https://github.com/speedhawk77-art/OpenKJ/actions/runs/40239122777) · [Fix details and attribution](https://github.com/speedhawk77-art/OpenKJ/blob/offline-remote-v9/community/MAINTENANCE_BUGFIXES_v9.md).

Full notices, source access information and LGPL library replacement instructions are installed in the application's `licenses` directory. The SDK-built OpenH264 runtime is excluded. Optional Cisco OpenH264 support remains off by default and downloads the official binary separately on the end user's computer after showing its original terms and attribution.

Close OpenKJ and back up your existing database/settings before updating. Download the `.exe` asset to install; the other assets provide matching sources, notices, verification evidence and checksums. The previous release remains available in the release history.
