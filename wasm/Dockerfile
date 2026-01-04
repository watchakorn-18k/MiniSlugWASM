# MiniSlug WASM Build Environment
# Uses Emscripten to compile to WebAssembly

FROM emscripten/emsdk:latest

# Install additional tools
RUN apt-get update && apt-get install -y \
    python3 \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY .. /app/

# Build command
WORKDIR /app/minislug0

# Default: run the WASM build
CMD ["make", "-f", "Makefile.wasm"]
