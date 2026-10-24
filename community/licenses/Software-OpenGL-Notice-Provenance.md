# Software OpenGL notice/source provenance

The exact packaged `opengl32sw.dll` SHA-256 is
`963641a718f9cae2705d5299eae9b7444e84e72ab3bef96a691510dd05fa1da4`.
Its embedded version strings identify Mesa 12.0.0-rc2 and LLVM 3.6.2.
A hidden-window WGL test independently reported `3.0 Mesa 12.0.0-rc2`
and the llvmpipe LLVM 3.6 renderer; no physical display was tested.

Original legal files are copied from these unmodified upstream archives:

- Mesa 12.0.0-rc2: https://archive.mesa3d.org/older-versions/12.x/12.0.0/mesa-12.0.0-rc2.tar.xz
  SHA-256 `4fea2e873e8baf92cefa58695d4e719bcddb279c601077f0e7fafaed1259fbc0`.
  Original `docs/license.html` is preserved under `mesa-12.0.0-rc2/`.
- LLVM 3.6.2: https://releases.llvm.org/3.6.2/llvm-3.6.2.src.tar.xz
  SHA-256 `f60dc158bfda6822de167e87275848969f0558b3134892ff54fced87e4667b94`.
  The root and five component/build/test LICENSE files are preserved under
  `llvm-3.6.2/`. Inclusion does not assert that every listed subcomponent was
  compiled into the shipped renderer.

These original notices and version-matched source archives do not attest
the exact Qt-specific patches, compiler configuration or complete rights
review for this compiled DLL. The committed byte-origin and renderer probes
record the independent runtime/reference evidence.
