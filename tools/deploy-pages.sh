#!/usr/bin/env bash
# Stages the Dusklight web build for static hosting (Cloudflare Pages, Netlify…).
#
# Usage: tools/deploy-pages.sh [build-dir] [out-dir]
#   build-dir  default: build/web-default-release
#   out-dir    default: dist/web
#
# If dusklight.wasm exceeds MAX_FILE_MB (Cloudflare Pages free tier: 25 MiB),
# the wasm is stored pre-compressed as dusklight.wasm.br and the JS loader is
# pointed at it; tools/pages/_headers serves it with Content-Encoding: br so
# the browser decompresses transparently (streaming compile still works).
set -euo pipefail

BUILD_DIR=${1:-build/web-default-release}
OUT_DIR=${2:-dist/web}
MAX_FILE_MB=25

rm -rf "$OUT_DIR"
mkdir -p "$OUT_DIR"

cp "$BUILD_DIR"/dusklight.html "$OUT_DIR"/index.html
cp "$BUILD_DIR"/dusklight.js "$BUILD_DIR"/dusklight.data "$OUT_DIR"/
cp tools/pages/_headers "$OUT_DIR"/_headers

wasm="$BUILD_DIR/dusklight.wasm"
size_mb=$(( $(stat -f%z "$wasm" 2>/dev/null || stat -c%s "$wasm") / 1024 / 1024 ))
if [ "$size_mb" -ge "$MAX_FILE_MB" ]; then
    echo "dusklight.wasm is ${size_mb} MiB (>= ${MAX_FILE_MB}): storing pre-compressed .wasm.br"
    command -v brotli >/dev/null || { echo "brotli not found (brew install brotli)"; exit 1; }
    brotli -f -q 11 -o "$OUT_DIR"/dusklight.wasm.br "$wasm"
    # Point the loader at the pre-compressed file. The server decompresses the
    # name's content via Content-Encoding, so the runtime still receives wasm.
    expr="s/wasmBinaryFile ??= findWasmBinary();/wasmBinaryFile ??= 'dusklight.wasm.br';/"
    sed -i '' "$expr" "$OUT_DIR"/dusklight.js 2>/dev/null || sed -i "$expr" "$OUT_DIR"/dusklight.js
    grep -q "dusklight.wasm.br" "$OUT_DIR"/dusklight.js || {
        echo "could not patch dusklight.js loader"; exit 1; }
else
    echo "dusklight.wasm is ${size_mb} MiB: stored as-is"
    cp "$wasm" "$OUT_DIR"/
fi

echo "Staged in $OUT_DIR:"
ls -lh "$OUT_DIR"
echo
echo "Deploy with: npx wrangler pages deploy $OUT_DIR --project-name dusklight"
