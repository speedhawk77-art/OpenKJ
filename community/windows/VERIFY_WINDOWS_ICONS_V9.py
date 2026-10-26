#!/usr/bin/env python3
"""Verify both executables contain every image from the original OpenKJ icon."""
import ctypes
from ctypes import wintypes
import json
from pathlib import Path
import struct


def verify(executable, original):
    kernel = ctypes.WinDLL('kernel32', use_last_error=True)
    kernel.LoadLibraryExW.argtypes = [wintypes.LPCWSTR, wintypes.HANDLE, wintypes.DWORD]
    kernel.LoadLibraryExW.restype = wintypes.HMODULE
    kernel.FindResourceW.argtypes = [wintypes.HMODULE, ctypes.c_void_p, ctypes.c_void_p]
    kernel.FindResourceW.restype = wintypes.HANDLE
    kernel.SizeofResource.argtypes = [wintypes.HMODULE, wintypes.HANDLE]
    kernel.SizeofResource.restype = wintypes.DWORD
    kernel.LoadResource.argtypes = [wintypes.HMODULE, wintypes.HANDLE]
    kernel.LoadResource.restype = wintypes.HANDLE
    kernel.LockResource.argtypes = [wintypes.HANDLE]
    kernel.LockResource.restype = ctypes.c_void_p
    kernel.FreeLibrary.argtypes = [wintypes.HMODULE]
    kernel.FreeLibrary.restype = wintypes.BOOL
    module = kernel.LoadLibraryExW(str(executable.resolve()), None, 0x22)
    if not module:
        raise ctypes.WinError(ctypes.get_last_error())
    try:
        def resource(name, kind):
            handle = kernel.FindResourceW(module, name, kind)
            if not handle:
                raise RuntimeError(f'Missing icon resource {name}/{kind} in {executable}')
            size = kernel.SizeofResource(module, handle)
            data = kernel.LockResource(kernel.LoadResource(module, handle))
            if not data or not size:
                raise RuntimeError('Unable to read icon resource')
            return ctypes.string_at(data, size)
        group = resource(102, 14)
        if group[:6] != original[:6]:
            raise RuntimeError('Icon image count/header differs from the original')
        count = struct.unpack_from('<H', original, 4)[0]
        for i in range(count):
            source_entry = original[6 + i * 16:22 + i * 16]
            group_entry = group[6 + i * 14:20 + i * 14]
            if source_entry[:12] != group_entry[:12]:
                raise RuntimeError('Embedded icon dimensions/format differ')
            size, offset = struct.unpack_from('<II', source_entry, 8)
            icon_id = struct.unpack_from('<H', group_entry, 12)[0]
            if resource(icon_id, 3) != original[offset:offset + size]:
                raise RuntimeError('Embedded icon pixels differ from the original')
        return {'executable': executable.name, 'original_icon_images_verified': count, 'passed': True}
    finally:
        kernel.FreeLibrary(module)


if __name__ == '__main__':
    original = Path('src/Icons/okjicon.ico').read_bytes()
    checks = [verify(Path('source-build') / name, original)
              for name in ('openkj.exe', 'OpenKJ-Remote.exe')]
    report = {'check': 'Original Windows executable icons', 'checks': checks, 'passed': True}
    Path('source-build/windows-icon-evidence.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report))
