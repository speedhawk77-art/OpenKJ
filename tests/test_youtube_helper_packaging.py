# SPDX-License-Identifier: GPL-3.0-or-later
"""Reject extra/tampered helper programs and preserve installer privacy exclusions."""
import copy,hashlib,importlib.util,json,struct,sys,tempfile,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
helpers=module('helpers','community/windows/helpers/VERIFY_HELPERS_V9.py')
builder=module('builder','community/windows/MAKE_SINGLE_FILE_INSTALLER.py')

class HelperPackagingTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup);self.root=Path(self.temp.name)
        pe=bytearray(128);pe[:2]=b'MZ';struct.pack_into('<I',pe,0x3c,64);pe[64:68]=b'PE\0\0';struct.pack_into('<H',pe,68,0x8664)
        files={}
        for name in ['yt-dlp.exe','deno.exe','ffmpeg.exe','ffprobe.exe','python/python.exe','python/python314.zip','yt-dlp.pyz']:
            p=self.root/'helpers/youtube'/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(bytes(pe) if name.endswith('.exe') else b'synthetic archive');files[p.relative_to(self.root).as_posix()]=helpers.digest(p)
        self.manifest={'source_commit':'a'*40,'files':files,'runtime_files':{n:h for n,h in files.items() if n.endswith('.exe')},'ffmpeg_gpl':False,'ffmpeg_nonfree':False,'cisco_binary_bundled':False}
    def test_exact_helper_inventory_passes(self):self.assertEqual(len(helpers.verify(self.root,self.manifest)),5)
    def test_changed_executable_rejected(self):
        (self.root/'helpers/youtube/deno.exe').write_bytes(b'changed')
        with self.assertRaises(RuntimeError):helpers.verify(self.root,self.manifest)
    def test_unattested_extra_dll_rejected(self):
        (self.root/'helpers/youtube/extra.dll').write_bytes(b'not approved')
        with self.assertRaises(RuntimeError):helpers.verify(self.root,self.manifest)
    def test_stdlib_archive_cannot_be_omitted(self):
        (self.root/'helpers/youtube/python/python314.zip').unlink()
        with self.assertRaises(RuntimeError):helpers.verify(self.root,self.manifest)
    def test_paths_cannot_escape_helper_directories(self):
        m=copy.deepcopy(self.manifest);m['files']['openkj.exe']='0'*64
        with self.assertRaises(RuntimeError):helpers.validate_manifest(m)
    def test_case_colliding_paths_rejected(self):
        m=copy.deepcopy(self.manifest);m['files']['helpers/youtube/DENO.EXE']='0'*64
        with self.assertRaises(RuntimeError):helpers.validate_manifest(m)
    def test_nonfree_flag_rejected(self):
        m=copy.deepcopy(self.manifest);m['ffmpeg_nonfree']=True
        with self.assertRaises(RuntimeError):helpers.validate_manifest(m)
    def test_only_private_stdlib_zip_is_installed(self):
        self.assertTrue(builder.keep_file(Path('helpers/youtube/python/python314.zip')))
        for name in ['backup.zip','helpers/youtube/private.zip','songs/song.mp4','settings.sqlite','helper.py']:
            self.assertFalse(builder.keep_file(Path(name)),name)
if __name__=='__main__':unittest.main()
