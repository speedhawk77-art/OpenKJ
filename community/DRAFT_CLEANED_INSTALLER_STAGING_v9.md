# Cleaned v9 installer — safely staging the GitHub draft

**This document does not authorize public redistribution.** The executable has been tested by the community maintainer on a second Windows computer, but third-party DLL licensing, complete corresponding LGPL library source availability, and installed-notice verification still need resolution.

The existing GitHub release ID is **421204266**, tag **`v9.0-offline-remote.1`**. The current draft contains the previous, uncleaned `OpenKJ-v9-Setup.exe` and checksum. Do not publish that binary.

A locally prepared helper named `STAGE_CLEANED_DRAFT_RELEASE.py` and one-click BAT can be run on the owner's Windows development PC to:

1. Verify the existing release is a draft with the expected ID/tag, and GitHub CLI is signed in.
2. Verify `C:\OpenKJ-Development\OpenKJ-Distribution-Test-v9` has exactly 196 GStreamer plugin DLLs, lacks specified suspect plugins and contains staged core licence documents.
3. Hash `C:\OpenKJ-Development\OpenKJ-Installer-Output\OpenKJ-v9-Cleaned-Setup.exe`, write a checksum and test-candidate manifest.
4. Upload those three files to the same **draft only** via `gh release upload`, and validate their GitHub SHA-256 digests.
5. Remove the two old draft assets **only after** the three new uploaded files pass digest checks.
6. Update the draft description to say the cleaned installer is a *candidate awaiting licence clearance*.
7. Verify the release is still a draft.

**The helper never sets `draft=false`; it is not a publication tool.** The owner must run it locally because GitHub's connector does not expose binary release-asset upload, and the full installer exists only on the development PC. The project will remain unpublished until the outstanding licensing obligations identified in `community/LICENSING_REVIEW_v9.md` and `community/TRANSITIVE_DLL_AUDIT_v9.md` are met.

Upstream licences and accurate component source delivery requirements: [Qt LGPL obligations](https://www.qt.io/development/open-source-lgpl-obligations), [GStreamer licensing](https://gstreamer.freedesktop.org/documentation/frequently-asked-questions/licensing.html), [Microsoft VC redistributable rules](https://learn.microsoft.com/en-us/cpp/windows/redistributing-visual-cpp-files).
