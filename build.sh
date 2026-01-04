#!/bin/bash
# MiniSlug Build Script using Podman

set -e

IMAGE_NAME="minislug-sdl2"

echo "=========================================="
echo "  MiniSlug SDL2 Build Script (Podman)"
echo "=========================================="

# Build Podman image
echo "[1/3] Building Podman image..."
podman build -t ${IMAGE_NAME} --target builder .

# Run build inside container
echo "[2/3] Compiling MiniSlug..."
podman run --rm \
    -v "$(pwd)/minislug0:/app/minislug0:Z" \
    -w /app/minislug0 \
    ${IMAGE_NAME} \
    make

echo "[3/3] Build complete!"
echo ""
echo "Binary location: minislug0/minislug"
