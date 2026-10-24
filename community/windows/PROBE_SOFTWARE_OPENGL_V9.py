#!/usr/bin/env python3
"""Ask the packaged software OpenGL DLL for its actual renderer/version strings."""
import argparse
import ctypes as C
from ctypes import wintypes as W
import hashlib
import json
from pathlib import Path
import os


class PFD(C.Structure):
    _fields_=[('nSize',W.WORD),('nVersion',W.WORD),('dwFlags',W.DWORD)] + [
        (n,C.c_ubyte) for n in ('iPixelType','cColorBits','cRedBits','cRedShift','cGreenBits',
        'cGreenShift','cBlueBits','cBlueShift','cAlphaBits','cAlphaShift','cAccumBits',
        'cAccumRedBits','cAccumGreenBits','cAccumBlueBits','cAccumAlphaBits','cDepthBits',
        'cStencilBits','cAuxBuffers','iLayerType','bReserved')] + [
        ('dwLayerMask',W.DWORD),('dwVisibleMask',W.DWORD),('dwDamageMask',W.DWORD)]


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--candidate',type=Path,required=True);ap.add_argument('--report',type=Path,required=True)
    args=ap.parse_args();root=args.candidate.resolve();path=root/'opengl32sw.dll'
    user=C.WinDLL('user32',use_last_error=True)
    def bind(lib,name,result,*params):
        f=getattr(lib,name);f.restype=result;f.argtypes=params;return f
    p=C.c_void_p
    create=bind(user,'CreateWindowExW',p,W.DWORD,W.LPCWSTR,W.LPCWSTR,W.DWORD,
                C.c_int,C.c_int,C.c_int,C.c_int,p,p,p,p)
    getdc=bind(user,'GetDC',p,p);release=bind(user,'ReleaseDC',C.c_int,p,p)
    destroy=bind(user,'DestroyWindow',W.BOOL,p)
    handle=dc=context=None;keep=os.add_dll_directory(str(root))
    result={'dll_sha256':hashlib.sha256(path.read_bytes()).hexdigest(),
            'physical_display_tested':False,'release_approved':False}
    try:
        gl=C.WinDLL(str(path))
        choose=bind(gl,'wglChoosePixelFormat',C.c_int,p,C.POINTER(PFD))
        setformat=bind(gl,'wglSetPixelFormat',W.BOOL,p,C.c_int,C.POINTER(PFD))
        make=bind(gl,'wglMakeCurrent',W.BOOL,p,p)
        newcontext=bind(gl,'wglCreateContext',p,p)
        delete=bind(gl,'wglDeleteContext',W.BOOL,p)
        getstring=bind(gl,'glGetString',C.c_char_p,C.c_uint)
        handle=create(0,'STATIC','OpenKJ isolated software GL probe',0x80000000,
                      0,0,16,16,None,None,None,None)
        if not handle:raise RuntimeError('Could not create hidden test window')
        dc=getdc(handle);descriptor=PFD();descriptor.nSize=C.sizeof(PFD);descriptor.nVersion=1
        descriptor.dwFlags=0x24;descriptor.cColorBits=24;descriptor.cDepthBits=24
        index=choose(dc,C.byref(descriptor))
        if not index or not setformat(dc,index,C.byref(descriptor)):
            raise RuntimeError('Software pixel format unavailable')
        context=newcontext(dc)
        if not context or not make(dc,context):raise RuntimeError('Software context unavailable')
        for name,code in (('vendor',0x1F00),('renderer',0x1F01),('version',0x1F02)):
            value=getstring(code);result[name]=value.decode('ascii',errors='replace') if value else None
        result['passed']=all(result.get(n) for n in ('vendor','renderer','version'))
        make(None,None)
        if context:delete(context);context=None
    except Exception as error:
        result.update(passed=False,error=str(error))
    finally:
        if dc and handle:release(handle,dc)
        if handle:destroy(handle)
        keep.close()
    args.report.parent.mkdir(parents=True,exist_ok=True)
    args.report.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
    print('Packaged software OpenGL renderer:',result)


if __name__=='__main__':main()
