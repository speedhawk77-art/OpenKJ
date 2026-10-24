# Rust dependency attribution review

The installer supplement retains all 512 original legal documents found in the pinned registry archives, plus 114 original metadata/readme/header documents for the 21 crates without standalone legal files. The 53 source-header excerpts preserve exact original leading bytes; no copyright owner or year was invented.

Additional missing MIT notices have been recovered from upstream:

| Installed notice | Applicable crates | Immutable upstream source |
| --- | --- | --- |
| av-data-0.4.2-MIT.txt | av-data 0.4.2 | rust-av/rust-av, commit d91e32d2d91c45998b0b2b7ee483c2fbe84e3df8, root LICENSE; the crate's own VCS metadata records this commit and `data` subdirectory |
| phf-0.10-MIT.txt | phf 0.10.1; phf_codegen, phf_generator and phf_shared 0.10.0 | rust-phf/rust-phf, commits bed01538ae576876f11189d541875d228acef9e8 and 3ea14b2166553ad6e7b9afe7244144f5d661b6c6; both root LICENSE files have Git blob f8a266f3877d8a6b169b409414f3bce00da5ebf0 |
| phf-macros-0.11.2-MIT.txt | phf_macros 0.11.2 | rust-phf/rust-phf, commit c9c35fd8ba3f1bc228388b0cef6e3814a02a72c0; root LICENSE blob cd5bf6855b571507a080b06132eb018094bb6a89 |
| difflib-0.4.0-MIT.txt | difflib 0.4.0 | DimaKudosh/difflib, commit f035fb8e656f27119e23eca9d5b996df14f3885e; all six published `.rs` files matched this commit byte-for-byte; root LICENSE blob 5acedf0412212a99997b78bd36da90c8a91111a7 |

For these nine crates the package metadata offers Apache-2.0 as an alternative: anes 0.1.6, crunchy 0.2.2, fxhash 0.2.1, mac 0.1.1, servo_arc 0.3.0, winapi-i686-pc-windows-gnu 0.4.0, winapi-x86_64-pc-windows-gnu 0.4.0, zune-core 0.4.12, zune-jpeg 0.4.13. The intended distribution uses that offered Apache-2.0 alternative, retaining original attributions, available notices and the full Apache-2.0 text in `cerbero-1.26.9/Apache-2.0.txt`. It does not rewrite their upstream licence metadata. Cargo documents `OR` as a choice and the older slash notation as deprecated multiple-licence syntax: https://doc.rust-lang.org/cargo/reference/manifest.html#the-license-and-license-file-fields . Any distinct per-file requirements remain applicable.

selectors 0.25.0's original source headers reference MPL 2.0. Those headers are preserved, and full MPL 2.0 terms are in `cerbero-1.26.9/MPL-2.txt`.

Four MIT-only Apple dependencies (block 0.1.6, malloc_buf 0.0.6, objc-foundation 0.1.1, objc_id 0.1.1) have no standalone licence/notice file even in their matching upstream Git tag trees. Their original author/licence declarations are retained, with no fabricated year. The registry graph enters this family through locale_config 0.3.0's `cfg(target_os = "macos")` dependencies. This is platform-specific graph evidence, not a claim that every crate in Cargo.lock is compiled into the Windows DLL. The complete mirror includes other platform and development dependencies as well as runtime dependencies.

These records close specific missing-text retrieval tasks and narrow the review. They do not certify complete licence compliance, exact source correspondence for the prebuilt Windows librsvg DLL, or rights for every use of H.264 media.
