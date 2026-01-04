#!/bin/bash
set -e

# Define image name
IMAGE_NAME="minislug-web"

echo "=========================================="
echo "  Building Web Server Image (Alpine)"
echo "=========================================="

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Error: 'build' directory not found!"
    echo "Please run ./build.sh first to generate WASM files."
    exit 1
fi

# Build image
podman build -t $IMAGE_NAME -f Dockerfile.web .

echo "=========================================="
echo "  Starting Web Server..."
echo "=========================================="
echo "Open: http://localhost:8080"
echo "Press Ctrl+C to stop."

# Run container
podman run --rm -p 8080:80 $IMAGE_NAME
