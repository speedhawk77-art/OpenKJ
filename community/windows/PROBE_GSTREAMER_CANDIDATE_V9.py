#!/usr/bin/env python3
"""Exercise candidate GStreamer on an isolated Windows runner using fake sinks.

Measures plugin discovery and synthetic audio/video/MP3/H264 pipelines. Results
are diagnostics, not a claim that a physical audio/display or phone UI works.
No user media, application settings, external speakers or application UI used.
"""
from __future__ import annotations
import argparse
import base64
import ctypes as C
import hashlib
import json
import os
from pathlib import Path


class GError(C.Structure):
    _fields_ = [('domain', C.c_uint), ('code', C.c_int), ('message', C.c_char_p)]


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--candidate', type=Path, required=True)
    ap.add_argument('--report', type=Path, required=True)
    ap.add_argument('--fixture', type=Path, required=True)
    ap.add_argument('--from-codec-controller',action='store_true',help='Preserve the Qt controller-selected optional path and preload its pinned codec')
    args = ap.parse_args()
    root = args.candidate.resolve()
    reports = args.report.parent.resolve()
    reports.mkdir(parents=True, exist_ok=True)
    optional_path=os.environ.get('GST_PLUGIN_PATH_1_0','') if args.from_codec_controller else ''
    codec=os.environ.get('OPENKJ_OPTIONAL_CODEC_PATH','') if args.from_codec_controller else ''
    codec_handle=codec_dir=None
    if codec:
        path=Path(codec)
        if hashlib.sha256(path.read_bytes()).hexdigest()!='2076cb5675ec6c1a4c70e7a2a322552f547b6eeed649d6dfcd9e02a543b24691':raise RuntimeError('Controller codec hash mismatch')
        codec_dir=os.add_dll_directory(str(path.parent));codec_handle=C.CDLL(str(path))
    os.environ.update({
        'GST_PLUGIN_SYSTEM_PATH_1_0': str(root / 'gst-plugins'),
        'GST_PLUGIN_PATH_1_0': optional_path, 'GST_REGISTRY_FORK': 'no',
        'GST_REGISTRY': str(reports / 'gst-probe-registry.bin'),
        'GST_REGISTRY_1_0': str(reports / 'gst-probe-registry.bin'),
        'GIO_EXTRA_MODULES': str(root / 'gio-modules'),
        'PATH': str(root) + os.pathsep + os.environ.get('PATH', ''),
    })
    dll_dir = os.add_dll_directory(str(root))
    gst = C.CDLL(str(root / 'libgstreamer-1.0-0.dll'))
    glib = C.CDLL(str(root / 'libglib-2.0-0.dll'))

    def bind(lib, name, result, *params):
        f = getattr(lib, name); f.restype = result; f.argtypes = params; return f

    p = C.c_void_p; errp = C.POINTER(GError)
    init = bind(gst, 'gst_init_check', C.c_int, p, p, C.POINTER(errp))
    parse = bind(gst, 'gst_parse_launch', p, C.c_char_p, C.POINTER(errp))
    set_state = bind(gst, 'gst_element_set_state', C.c_int, p, C.c_int)
    get_bus = bind(gst, 'gst_element_get_bus', p, p)
    pop = bind(gst, 'gst_bus_timed_pop_filtered', p, p, C.c_uint64, C.c_uint)
    structure = bind(gst, 'gst_message_get_structure', p, p)
    structname = bind(gst, 'gst_structure_get_name', C.c_char_p, p)
    parse_error = bind(gst, 'gst_message_parse_error', None, p, C.POINTER(errp), C.POINTER(p))
    obj_unref = bind(gst, 'gst_object_unref', None, p)
    msg_unref = bind(gst, 'gst_mini_object_unref', None, p)
    err_free = bind(glib, 'g_error_free', None, errp)
    mem_free = bind(glib, 'g_free', None, p)
    find_factory = bind(gst, 'gst_element_factory_find', p, C.c_char_p)

    def take_error(error):
        if not error:
            return None
        text = error.contents.message.decode('utf-8', errors='replace')
        err_free(error)
        return text

    error = errp()
    if not init(None, None, C.byref(error)):
        raise RuntimeError('GStreamer initialization failed: ' + str(take_error(error)))
    factories = {}
    for name in ('audiotestsrc', 'videotestsrc', 'lamemp3enc', 'mpg123audiodec',
                 'qtdemux', 'h264parse', 'decodebin', 'avdec_h264',
                 'openh264dec', 'd3d11h264dec', 'd3d12h264dec', 'msdkh264dec', 'nvh264dec', 'qsvh264dec'):
        factory = find_factory(name.encode())
        factories[name] = bool(factory)
        if factory: obj_unref(factory)

    fixture = reports / 'generated-blue-h264.mp4'
    fixture.write_bytes(base64.b64decode(args.fixture.read_bytes(), validate=False))
    if hashlib.sha256(fixture.read_bytes()).hexdigest() != 'e004249adaa97849fdb1d8dc686422884a861ef11ebee64aec9c9b0624fcb2a6':
        raise RuntimeError('Generated H.264 fixture checksum mismatch')
    pipelines = {
        'raw_audio': 'audiotestsrc num-buffers=8 ! audioconvert ! audioresample ! fakesink sync=false',
        'raw_video': 'videotestsrc num-buffers=8 ! videoconvert ! fakesink sync=false',
        'mp3_roundtrip': 'audiotestsrc num-buffers=12 ! audioconvert ! lamemp3enc ! mpegaudioparse ! mpg123audiodec ! fakesink sync=false',
        'h264_mp4': f'filesrc location="{fixture.as_posix()}" ! qtdemux ! h264parse ! decodebin ! fakesink sync=false',
    }
    results = {}
    for name, description in pipelines.items():
        pipeline = bus = message = None
        try:
            error = errp()
            pipeline = parse(description.encode(), C.byref(error))
            parse_problem = take_error(error)
            if not pipeline or parse_problem:
                results[name] = {'passed': False, 'error': parse_problem or 'Pipeline could not be created'}
                continue
            bus = get_bus(pipeline)
            if set_state(pipeline, 4) == 0:  # GST_STATE_PLAYING; state-change failure
                # Still collect the error from the bus when available.
                timeout = 2_000_000_000
            else:
                timeout = 30_000_000_000
            message = pop(bus, timeout, 3)  # GST_MESSAGE_EOS | GST_MESSAGE_ERROR
            if not message:
                results[name] = {'passed': False, 'error': 'No EOS/error within timeout'}
            else:
                data = structure(message)
                if data:
                    detail = structname(data).decode('utf-8', errors='replace')
                    error = errp(); debug = p()
                    if detail == 'GstMessageError':
                        parse_error(message, C.byref(error), C.byref(debug))
                        detail = take_error(error) or detail
                        if debug: mem_free(debug)
                    results[name] = {'passed': False, 'error': detail}
                else:
                    results[name] = {'passed': True, 'result': 'EOS reached with fakesink'}
        finally:
            if pipeline: set_state(pipeline, 1)  # GST_STATE_NULL
            if message: msg_unref(message)
            if bus: obj_unref(bus)
            if pipeline: obj_unref(pipeline)
    report = {
        'check': 'Isolated Windows candidate GStreamer diagnostic pipelines',
        'factories_present': factories, 'pipelines': results,
        'plugin_files': sorted(p.name for p in (root / 'gst-plugins').glob('*.dll')),
        'h264_fixture_sha256': hashlib.sha256(fixture.read_bytes()).hexdigest(),
        'candidate_gstreamer_sha256': hashlib.sha256((root / 'libgstreamer-1.0-0.dll').read_bytes()).hexdigest(),
        'physical_output_tested': False, 'application_ui_tested': False,
        'phone_controls_tested': False, 'release_approved': False,
        'limits': 'This runner can differ from the maintainer laptop in GPU, codecs, audio devices and Windows features.',
    }
    args.report.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    summary = '### Isolated GStreamer probes (diagnostics)\n\n' + '\n'.join(
        f"- {name}: {'PASS' if row['passed'] else 'FAIL: ' + row['error']}"
        for name, row in results.items()) + '\n\nPhysical playback and phone controls are still untested.\n'
    print(summary)
    if os.environ.get('GITHUB_STEP_SUMMARY'):
        with open(os.environ['GITHUB_STEP_SUMMARY'], 'a', encoding='utf-8') as f: f.write(summary)
    dll_dir.close()


if __name__ == '__main__':
    main()
