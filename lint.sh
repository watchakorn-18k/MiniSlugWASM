#!/bin/bash
IMAGE_NAME="minislug-lint"

# Change directory to project root (where Dockerfile.lint is)
cd "$(dirname "$0")"

echo "=========================================="
echo "  MiniSlug Lint Script (Podman)"
echo "=========================================="

echo "[1/2] Building Lint Image ($IMAGE_NAME)..."
podman build -t $IMAGE_NAME -f Dockerfile.lint .

echo "[2/2] Running Clang-Tidy..."
echo "Checking all .c files in minislug0/..."

# Flags:
# -checks='*': Enable all checks (adjust as needed, maybe too noisy)
# For now, let's use default checks or basic ones to avoid overwhelming output on legacy code.
# -I /usr/include/SDL2: Include path for SDL2

podman run --rm -v $(pwd):/app -w /app/minislug0 $IMAGE_NAME \
    bash -c 'clang-tidy *.c -- -I/usr/include/SDL2 -D_REENTRANT'

echo "=========================================="
echo "  Linting Complete!"
echo "=========================================="
