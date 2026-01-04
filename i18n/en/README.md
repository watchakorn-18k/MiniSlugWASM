# 🎮 MiniSlug (SDL2 + WASM Port)

[**ไทย**](../th/README.md) | [**English**](./README.md)

A port of the MiniSlug game engine from SDL 1.2 to **SDL 2.0**, with added support for **WebAssembly (WASM)** via Emscripten. This allows the game to be played directly in modern web browsers.

## ✨ Features

- **SDL 2.0 Migration**: Upgraded graphics, input, and audio subsystems to SDL2.
- **WebAssembly Support**: Compile and run the game in any modern web browser.
- **Containerized Build System**: Uses **Podman/Docker** for building, ensuring a clean environment with no local dependencies required.
- **Graphic Glitch Fixes**: Implemented 16-bit shadow buffering to fix color artifacts on modern 32-bit displays.

## 🚀 How to Play (WebAssembly)

You can run the compiled WASM version locally:

1.  Navigate to the build directory:
    ```bash
    cd wasm/build
    ```
2.  Start a local HTTP server (requires Python 3):
    ```bash
    python3 -m http.server 8080
    ```
3.  Open your browser and visit: [http://localhost:8080/minislug.html](http://localhost:8080/minislug.html)

## 🛠️ How to Build

This project uses **Podman** (or Docker) to handle dependencies. You do not need to install SDL2 or Emscripten on your host machine.

### Prerequisites
- **Podman** (recommended) or Docker installed.

### 1. Native Build (Linux/macOS Binary)
To build the native executable using SDL2:

```bash
./build.sh
```
This will create the executable at `minislug0/minislug`.

### 2. WebAssembly Build
To compile for the web using Emscripten:

```bash
./wasm/build.sh
```
This will generate the web files in `wasm/build/`.

## 🎮 Controls

- **Arrow Keys**: Move
- **C**: Jump
- **V**: Shoot
- **X**: Grenade

## 📂 Project Structure

- `minislug0/`: Source code (C/C++)
- `wasm/`: WASM build scripts and Dockerfile
- `i18n/`: Documentation in other languages

## 📜 Credits

- **Original Game**: 17o2
- **SDL2 & WASM Port**: [Your Name/Repo]
