# MiniSlug WASM Production Dockerfile
# Multi-stage build: Emscripten (build) → Nginx (serve)

# ============================================================
# Stage 1: Build WASM using Emscripten
# ============================================================
FROM emscripten/emsdk:latest AS builder

# Install additional tools
RUN apt-get update && apt-get install -y \
    python3 \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY . /app/

# Copy WASM Makefile to minislug0
COPY wasm/Makefile.wasm /app/minislug0/

# Build WASM
WORKDIR /app/minislug0
RUN make -f Makefile.wasm

# ============================================================
# Stage 2: Serve with Nginx
# ============================================================
FROM nginx:alpine AS production

# Copy nginx configuration
COPY wasm/nginx.conf /etc/nginx/conf.d/default.conf

# Copy built WASM files from builder stage
COPY --from=builder /app/wasm/build/ /usr/share/nginx/html/

# Copy Landing Page
COPY wasm/index.html /usr/share/nginx/html/

# Copy favicon
COPY wasm/favicon.svg /usr/share/nginx/html/

# Expose port 80
EXPOSE 80

# Start Nginx
CMD ["nginx", "-g", "daemon off;"]
