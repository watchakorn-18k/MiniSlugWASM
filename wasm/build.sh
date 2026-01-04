#!/bin/bash
# MiniSlug WASM Build Script using Podman + Emscripten

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
IMAGE_NAME="minislug-wasm"

echo "=========================================="
echo "  MiniSlug WASM Build Script (Podman)"
echo "=========================================="
echo ""

# Build Podman image
echo "[1/4] Building Emscripten Podman image..."
podman build -t ${IMAGE_NAME} -f "$SCRIPT_DIR/Dockerfile" "$PROJECT_ROOT"

# Copy Makefile.wasm to minislug0
echo "[2/4] Preparing build files..."
cp "$SCRIPT_DIR/Makefile.wasm" "$PROJECT_ROOT/minislug0/"

# Run WASM build inside container
echo "[3/4] Compiling to WebAssembly..."
podman run --rm \
    -v "$PROJECT_ROOT:/app:Z" \
    -w /app/minislug0 \
    ${IMAGE_NAME} \
    make -f Makefile.wasm

echo "[4/4] Build complete!"
echo ""
echo "=========================================="
echo "  Output files in: wasm/build/"
echo "=========================================="
ls -la "$SCRIPT_DIR/build/" 2>/dev/null || echo "(build directory will be created after successful build)"
echo ""
echo "To test locally:"
echo "  cd wasm/build && python3 -m http.server 8080"
echo "  Open: http://localhost:8080/minislug.html"
