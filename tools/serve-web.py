#!/usr/bin/env python3
"""Static file server for the Dusklight web build.

Sends the COOP/COEP headers required for cross-origin isolation
(SharedArrayBuffer) along with correct Content-Types for .wasm/.js,
and disables caching so rebuilds are always picked up.

Usage: serve-web.py [port] [directory] [--https]
  port       port to listen on (default: 8080)
  directory  directory to serve (default: build/web-default)
  --https    serve over HTTPS with a self-signed certificate. Required to
             reach the build from OTHER devices on the network: pthreads
             (SharedArrayBuffer) and WebGPU only work in secure contexts,
             and only https:// or localhost qualify. The certificate is
             generated on first use (needs openssl) and browsers will show
             a warning once; proceed past it and the page works.
"""
import shutil
import socket
import ssl
import subprocess
import sys
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path


class WebHandler(SimpleHTTPRequestHandler):
    extensions_map = {
        **SimpleHTTPRequestHandler.extensions_map,
        '.wasm': 'application/wasm',
        '.js': 'text/javascript',
        '.mjs': 'text/javascript',
    }

    def end_headers(self):
        self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
        self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
        self.send_header('Cache-Control', 'no-store')
        super().end_headers()


def lan_ip():
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
            s.connect(('8.8.8.8', 80))
            return s.getsockname()[0]
    except OSError:
        return None


def ensure_certificate(cert_dir: Path):
    cert = cert_dir / 'dusklight-dev-cert.pem'
    key = cert_dir / 'dusklight-dev-key.pem'
    if cert.exists() and key.exists():
        return cert, key
    cert_dir.mkdir(parents=True, exist_ok=True)
    ip = lan_ip()
    names = ['localhost', '127.0.0.1'] + ([ip] if ip else [])
    # Prefer mkcert: its certificates are signed by a local development CA, so
    # once that CA is trusted (`mkcert -install` on this machine; copy and trust
    # `mkcert -CAROOT`/rootCA.pem on other devices) browsers show no warnings.
    if shutil.which('mkcert'):
        subprocess.run(
            ['mkcert', '-cert-file', str(cert), '-key-file', str(key), *names],
            check=True, capture_output=True)
        print(f'Generated mkcert certificate for {", ".join(names)} at {cert}')
        print("If you haven't yet, run `mkcert -install` once to trust the local CA.")
        return cert, key
    san = 'DNS:localhost,IP:127.0.0.1'
    if ip:
        san += f',IP:{ip}'
    subprocess.run(
        ['openssl', 'req', '-x509', '-newkey', 'rsa:2048', '-sha256', '-days', '825',
         '-nodes', '-subj', '/CN=dusklight-dev', '-addext', f'subjectAltName={san}',
         '-keyout', str(key), '-out', str(cert)],
        check=True, capture_output=True)
    print(f'Generated self-signed certificate at {cert} (install mkcert for warning-free HTTPS)')
    return cert, key


def main():
    args = [a for a in sys.argv[1:] if a != '--https']
    use_https = '--https' in sys.argv[1:]
    port = int(args[0]) if len(args) > 0 else 8080
    directory = args[1] if len(args) > 1 else 'build/web-default'

    server = ThreadingHTTPServer(('', port), partial(WebHandler, directory=directory))
    scheme = 'http'
    if use_https:
        cert, key = ensure_certificate(Path(directory).resolve().parent / '.dev-certs')
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        ctx.load_cert_chain(certfile=cert, keyfile=key)
        server.socket = ctx.wrap_socket(server.socket, server_side=True)
        scheme = 'https'

    print(f'Serving {directory} at {scheme}://localhost:{port}/')
    ip = lan_ip()
    if use_https and ip:
        print(f'Reachable from other devices at {scheme}://{ip}:{port}/dusklight.html')
        print('(self-signed certificate: accept the browser warning once)')
    elif not use_https:
        print('Note: other devices on the network need --https; without a secure '
              'context the browser disables SharedArrayBuffer and WebGPU.')
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass


if __name__ == '__main__':
    main()
