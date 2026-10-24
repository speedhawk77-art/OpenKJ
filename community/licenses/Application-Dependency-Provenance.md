# Source-level application dependency notices

The vendored `src/taglib/toolkit/taglib.h` declares TagLib **1.11.1**. Its
leading copyright and licence comments, and those of 213 other vendored
TagLib headers/sources, are preserved in
`taglib-1.11.1/Bundled-Source-Headers.txt`, with full-source SHA-256 hashes.
The complete LGPL-2.1 and MPL-1.1 texts are also in this licence collection.
This static application dependency is distinct from the GStreamer-side
`libtag` DLL, whose mapped Cerbero recipe is TagLib **1.13.1**.

The checked-in spdlog submodule points to commit
`5df9b11141f8a6f01b069f8c2d7120b25cca6a02` (1.9.2), with bundled fmt 8.0.1
(`FMT_VERSION 80001`). The alternate CPM path in CMake names spdlog 1.8.2,
with bundled fmt 7.1.3 (`FMT_VERSION 70103`). The fresh source-audit workflow
selects the exact checked-in submodule using `SPDLOG_USE_BUNDLED=ON`.

`spdlog/MIT.txt` is copied from the pinned submodule's LICENSE. GitHub returns
the same upstream blob, `12beadb27e7a9aefdd862374aea37ba94a70977e`, for
spdlog 1.8.2 and the checked-in 1.9.2 commit. `fmt/MIT.txt` is copied from
fmt 8.0.1 LICENSE.rst; fmt 7.1.3 returns the same upstream blob,
`f0ec3db4d2a03368a9f1357b38e79486bdf41076`.

These are source-level notice records. The existing draft executable's exact
original dependency/configuration choices are not retroactively attested by
a successful build of today's checkout. Nor does this file assert that every
notice required by every dependency has been identified.
