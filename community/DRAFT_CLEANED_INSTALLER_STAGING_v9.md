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


## Actual staging outcome — 2026-10-22

The owner ran the staging helper on Windows. Its initial safety checks passed; GitHub accepted all three cleaned candidate assets, verified their SHA-256 digests, and the helper deleted the two obsolete uncleaned draft assets. The full cleaned EXE asset has **205,876,054 bytes** with SHA-256 `bc35c24ef25529b86b90bd3f5a215603de3b283d590620ec58820e02f83421ef`. Asset list:

- `OpenKJ-v9-Cleaned-Setup.exe`
- `OpenKJ-v9-Cleaned-Setup.sha256.txt`
- `OpenKJ-v9-Cleaned-Manifest.json`

**Post-upload verifier bug:** The GitHub release PATCH to update the draft description omitted `tag_name`. GitHub subsequently showed this draft release's `tag_name` as `untagged-d436a7c3c08d99f4560f`. The helper's final check hard-coded `v9.0-offline-remote.1` and therefore reported `STOPPED SAFELY: Expected draft release/tag was not found`, even though the assets were correct. The release identity is still **421204266** and its `draft` status remains **true**, confirmed directly using GitHub API. No new upload is required. All further verification should use the release's numeric ID and should treat the tag as mutable until the draft is published. The release description's original install filename `OpenKJ-v9-Setup.exe` is now outdated and should be changed to `OpenKJ-v9-Cleaned-Setup.exe` before publication.

**Important:** The cleaned installer is still a **draft-only candidate** and the outstanding LGPL corresponding-source and third-party binary redistribution compliance work has not been resolved.
