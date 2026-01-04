# MiniSlug SDL2 Build Environment
# Multi-stage build for compiling the game

FROM debian:bullseye-slim AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    g++ \
    make \
    libsdl2-dev \
    libsdl2-mixer-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Build the game
WORKDIR /app/minislug0

# Default command: build the game
CMD ["make", "clean", "&&", "make"]

# -----------------------------------------------------------
# Runtime stage (for testing with X11 forwarding)
# -----------------------------------------------------------
FROM debian:bullseye-slim AS runtime

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libsdl2-2.0-0 \
    libsdl2-mixer-2.0-0 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy built binary from builder stage
COPY --from=builder /app/minislug0/minislug /app/
COPY --from=builder /app/minislug0/gfx /app/gfx
COPY --from=builder /app/minislug0/sfx /app/sfx
COPY --from=builder /app/minislug0/lev* /app/
COPY --from=builder /app/minislug0/*.cfg /app/
COPY --from=builder /app/minislug0/*.scr /app/

# Run the game
CMD ["./minislug"]
