#!/usr/bin/env python3
"""Check the real yt-dlp CLI/metadata contract using only generated local media."""
import argparse
import base64
import hashlib
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile
import threading


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--downloader', type=Path, required=True)
    args = parser.parse_args()
    downloader = args.downloader.resolve()
    expected = '33121d192d706e62f684a06f80918e3f3a8ba52ac3a65213b6951c39c7d9f555'
    if hashlib.sha256(downloader.read_bytes()).hexdigest() != expected:
        raise RuntimeError('Not the pinned official yt-dlp 2026.10.21 executable')
    source = Path('src/youtubedownload.cpp').read_text(encoding='utf-8')

    def option(name):
        match = re.search(r'"' + re.escape(name) + r'",\s*"([^"\n]+)"', source)
        if not match:
            raise RuntimeError('Cannot identify production option: ' + name)
        return match[1]

    media = base64.b64decode(Path('tests/fixtures/youtube-av.mp4.b64').read_text())

    class Fixture(BaseHTTPRequestHandler):
        def do_GET(self):
            if self.path != '/fixture.mp4':
                self.send_error(404)
                return
            self.send_response(200)
            self.send_header('Content-Type', 'video/mp4')
            self.send_header('Content-Length', str(len(media)))
            self.end_headers()
            self.wfile.write(media)

        def log_message(self, *unused):
            pass

    server = ThreadingHTTPServer(('127.0.0.1', 0), Fixture)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    try:
        with tempfile.TemporaryDirectory(prefix='OpenKJ space percent % unicode é ') as directory:
            work = Path(directory)
            info = {
                'id': '00000000001', 'title': 'Generated karaoke "title" café %',
                'webpage_url': 'https://www.youtube.com/watch?v=00000000001',
                'extractor': 'youtube', 'extractor_key': 'Youtube', 'duration': 1,
                'is_live': False, 'is_upcoming': False,
                'formats': [{'url': f'http://127.0.0.1:{server.server_port}/fixture.mp4',
                             'format_id': '18', 'ext': 'mp4', 'vcodec': 'avc1.42E01E',
                             'acodec': 'mp4a.40.2', 'height': 90}],
            }
            info_path = work / 'input.json'
            info_path.write_text(json.dumps(info), encoding='utf-8')
            escaped = str(work).replace('%', '%%')
            command = [str(downloader), '--ignore-config', '--no-plugin-dirs', '--no-playlist',
                       '--no-simulate', '--no-overwrites', '--no-live-from-start',
                       '--match-filter', option('--match-filter'), '--socket-timeout', '20',
                       '--retries', '2', '--fragment-retries', '2',
                       '--format', option('--format'), '--merge-output-format', 'mp4',
                       '--no-js-runtimes', '--no-remote-components',
                       '--output', escaped + '/video.%(ext)s',
                       '--print-to-file', option('--print-to-file'), escaped + '/result.json',
                       '--newline', '--progress', '--progress-template', option('--progress-template'),
                       '--load-info-json', str(info_path)]
            env = dict(os.environ, PYTHONIOENCODING='utf-8', PYTHONUTF8='1')
            result = subprocess.run(command, env=env, capture_output=True, text=True,
                                    encoding='utf-8', errors='replace', timeout=90)
            if result.returncode:
                raise RuntimeError('Actual downloader contract failed: ' + result.stderr[-2000:])
            metadata = json.loads((work / 'result.json').read_text(encoding='utf-8'))
            if (metadata['id'] != info['id'] or metadata['title'] != info['title']
                    or Path(metadata['filepath']).resolve() != (work / 'video.mp4').resolve()
                    or (work / 'video.mp4').read_bytes() != media):
                raise RuntimeError('Actual downloader output differs from the controller contract')
    finally:
        server.shutdown()
        server.server_close()
        thread.join(timeout=5)
    report = {'passed': True, 'downloader_version': '2026.10.21', 'downloader_sha256': expected,
              'real_downloader_cli_tested': True, 'metadata_template_tested': True,
              'combined_format_selection_tested': True, 'unicode_percent_paths_tested': True,
              'live_youtube_tested': False, 'separate_track_merging_tested': False,
              'media_source': 'Generated local one-second H.264/AAC fixture, served on loopback'}
    Path('youtube-real-downloader-contract.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report))


if __name__ == '__main__':
    main()
