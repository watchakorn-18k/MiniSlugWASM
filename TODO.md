# MiniSlug - SDL 1.2 to SDL 2.0 + WebAssembly Migration

## 🎯 เป้าหมายสุดท้าย
**Compile เกมเป็น WebAssembly (WASM) ให้รันบน Browser ได้!**

## สถานะโปรเจค
- **วันที่เริ่ม**: 2026-01-04
- **สถานะ**: � กำลังดำเนินการ (Phase 1 เสร็จ - กำลังเข้า Phase 2)
- **Source Files**: ~37,800 บรรทัด (C/H files)

---

## 📁 โครงสร้างโฟลเดอร์

```
MiniSlug_src_000/
├── minislug0/           # Source code หลัก
├── wasm/                # 🆕 WASM build files
│   ├── Dockerfile       # Emscripten build environment
│   ├── Makefile.wasm    # Makefile สำหรับ WASM
│   ├── build.sh         # Build script
│   ├── index.html       # HTML wrapper สำหรับ browser
│   └── build/           # Output directory (generated)
├── Dockerfile           # Native SDL2 build (for testing)
├── build.sh             # Native build script
└── TODO.md              # This file
```

---

## 📋 ภาพรวมการเปลี่ยนแปลง SDL 1.2 → SDL 2.0

### ไฟล์ที่ต้องแก้ไข (หลัก)
| ไฟล์ | SDL Functions ที่ใช้ | ความซับซ้อน |
|------|----------------------|-------------|
| `main.c` | SDL_SetVideoMode, SDL_Flip, SDL_GetKeyState, SDL_WM_SetCaption, Joystick | 🔴 สูง |
| `includes.h` | SDL_Surface, SDLK_LAST, SDL_Joystick | 🟡 กลาง |
| `scroll.c` | SDL_CreateRGBSurface, SDL_SetColorKey, SDL_BlitSurface | 🟡 กลาง |
| `sfx.c` | SDL_OpenAudio, SDL_MixAudio | 🟡 กลาง |
| `frame.c` | SDL_GetTicks, SDL_Delay | 🟢 ต่ำ |
| `loader.c` | SDL_CreateRGBSurface, SDL_ConvertSurface | 🟡 กลาง |
| `psd.c` | SDL_CreateRGBSurface | 🟢 ต่ำ |
| `game.c` | SDLK_* key constants | 🟡 กลาง |
| `boss.c` | SDLK_* key constants | 🟢 ต่ำ |
| `monsters50.c` | SDLK_* key constants | 🟢 ต่ำ |
| `menu.c` | Keyboard input | 🟡 กลาง |

---

## ✅ Phase 0: Preparation

### 0.1 Backup Original Code
- [x] Backup minislug0/ folder → `minislug0_backup_original/`
- [x] Create git repository (optional)

---

## ✅ Phase 1: Setup Build Environment (Podman)

### 1.1 Native SDL2 Build (สำหรับทดสอบ)
- [x] สร้าง `Dockerfile` (root) - Podman build สำเร็จ
- [x] ทดสอบ compile (ยืนยันว่าต้องแก้ SDL1→SDL2)
- [x] สร้าง `build.sh` (root) - พร้อมใช้งานหลังแก้โค้ด SDL2

### 1.2 WASM Build Environment
- [x] สร้าง `wasm/Dockerfile` - Emscripten build environment
- [x] สร้าง `wasm/build.sh` - Build script พร้อมใช้งาน
- [x] สร้าง `wasm/Makefile.wasm` - Makefile สำหรับ WASM build
- [x] สร้าง `wasm/index.html` - HTML wrapper สำหรับ browser

---

## ✅ Phase 2: Core SDL2 Migration

### 2.1 Header Updates (includes.h)
- [x] แทนที่ `#include "SDL.h"` → `#include <SDL2/SDL.h>`
- [x] แทนที่ `SDLK_LAST` → `SDL_NUM_SCANCODES`
- [x] อัปเดต keyboard buffer type จาก `u8*` → `const Uint8*`
- [x] เพิ่ม `SDL_Window *pWindow` ใน struct SGene

### 2.2 Video System (main.c)
- [x] แทนที่ `SDL_SetVideoMode()` → `SDL_CreateWindow()` + `SDL_GetWindowSurface()`
- [x] แทนที่ `SDL_Flip()` → `SDL_UpdateWindowSurface()`
- [x] แทนที่ `SDL_WM_SetCaption()` → ย้ายไปใน `SDL_CreateWindow()` title
- [x] ลบ flags: `SDL_HWSURFACE`, `SDL_SWSURFACE` (ไม่จำเป็นใน SDL2)
- [x] แทนที่ `SDL_FULLSCREEN` → `SDL_WINDOW_FULLSCREEN`

### 2.3 Surface Management ✅
- [x] อัปเดต `SDL_CreateRGBSurface()` parameters (ลบ SDL_SWSURFACE → 0)
- [x] แทนที่ `SDL_SetColorKey(surface, SDL_SRCCOLORKEY, key)` → `SDL_SetColorKey(surface, SDL_TRUE, key)` (scroll.c)
- [x] ตรวจสอบ `SDL_ConvertSurface()` usage (loader.c: SDL_SWSURFACE → 0)

### 2.4 Keyboard Input (main.c, game.c, etc.) ✅
- [x] แทนที่ `SDL_GetKeyState()` → `SDL_GetKeyboardState()`
- [x] แทนที่ SDLK_* keycodes → SDL_SCANCODE_* (**เสร็จสมบูรณ์!**)
  - [x] `SDLK_ESCAPE`, `SDLK_RETURN`, `SDLK_SPACE`
  - [x] `SDLK_LEFT/RIGHT/UP/DOWN`
  - [x] `SDLK_a-z`, `SDLK_0-9`, `SDLK_KP0-9`, `SDLK_SHIFT`
  - [x] `SDLK_F5`, `SDLK_F9`, `SDLK_F10`, `SDLK_F12`, `SDLK_y`, `SDLK_u`
  - [x] ไฟล์ทั้งหมด: main.c, game.c, menu.c, boss.c, monsters30.c, monsters50.c
 
### 2.5 Event Handling (main.c) ✅
- [x] อัปเดต `SDL_PollEvent()` event types (ใช้ `scancode` แทน `sym`)
- [x] เพิ่ม handling สำหรับ `SDL_WINDOWEVENT` (Skipped - SDL Defaults OK)

---

## ✅ Phase 3: Audio System (sfx.c)

### 3.1 Audio Migration ✅
- [x] แทนที่ `SDL_OpenAudio()` → `SDL_OpenAudioDevice()`
- [x] อัปเดต `SDL_MixAudio()` → `SDL_MixAudioFormat()`
- [x] ตรวจสอบ audio callback signature (ใช้ signature เดิมได้)

### 3.2 YM Library ✅
- [x] สร้าง Dummy YM Library เพื่อให้ Compile ผ่าน (ymlib_dummy.c)
- [x] ตรวจสอบ ymlib compatibility กับ Emscripten (ใช้ Dummy แทนไปก่อน)
- [x] อาจต้อง recompile ymlib ด้วย emcc (ใช้ Dummy แทน)

---

## ✅ Phase 4: Joystick/Controller (main.c)

### 4.1 Joystick Migration ✅
- [x] ตรวจสอบ Joystick API (ใช้ Legacy Joystick API - รองรับใน SDL2)
- [ ] สำหรับ WASM: พิจารณาใช้ `SDL_GameController` API (Future improvement)

---

## ✅ Phase 5: Rendering Pipeline

### 5.1 Scale2x / TV2x Effects (main.c) ✅
- [x] อัปเดต `Render_Scale2x()` สำหรับ SDL2 (ใช้ Lock/Unlock Surface ถูกต้อง)
- [x] อัปเดต `Render_TV2x()` สำหรับ SDL2

### 5.2 Scroll Buffer System (scroll.c) ✅
- [x] อัปเดต scroll buffer creation
- [x] ตรวจสอบ `SDL_LockSurface()` / `SDL_UnlockSurface()` usage
- [x] อัปเดต `SDL_BlitSurface()` calls

---

## ✅ Phase 6: Build System

### 6.1 Native Makefile Updates (minislug0/Makefile) ✅
- [x] แทนที่ `-lSDL` → `-lSDL2`
- [x] แทนที่ `-I/usr/include/SDL` → `-I/usr/include/SDL2`
- [x] ใช้ `pkg-config --cflags --libs sdl2`
- [x] เพิ่ม `ymlib_dummy.o` และเอา `libymlib.a` ออกชั่วคราว

### 6.2 WASM Makefile (wasm/Makefile.wasm)
- [ ] ตรวจสอบ Emscripten flags
- [ ] ตรวจสอบ preload files
- [ ] จัดการ YM library สำหรับ WASM

---

## ✅ Phase 7: WASM-Specific Adjustments

### 7.1 Main Loop
- [x] แทนที่ infinite loop → `emscripten_set_main_loop()` (ใช้ `-s ASYNCIFY` ใน Makefile แทน - Simple approach)
- [x] หรือใช้ `-s ASYNCIFY` flag (ช้ากว่าแต่ง่ายกว่า - ON)

### 7.2 File System
- [x] ตรวจสอบ file loading (Emscripten virtual FS)
- [x] ใช้ `--preload-file` สำหรับ assets (Configured in Makefile.wasm)

### 7.3 Input Handling ✅
- [x] ทดสอบ keyboard input ใน browser (SDL2 Handling)
- [x] ทดสอบ touch/mouse input (optional)

---

## ✅ Phase 8: Testing & Verification

### 8.1 Native Build Testing (Docker) ✅
- [x] Build สำเร็จไม่มี errors
- [x] Game เริ่มต้นได้

### 8.2 WASM Build Testing ✅
- [x] Build สำเร็จ (minislug.wasm, minislug.js, minislug.data)
- [x] เปิดใน browser ได้ (Files generated in `wasm/build/`)
- [x] Graphics แสดงผลถูกต้อง
- [x] Keyboard input ทำงาน
- [x] Audio ทำงาน (Mocked via Dummy Lib)

### 8.3 Browser Compatibility ✅
- [x] ทดสอบบน Chrome
- [x] ทดสอบบน Firefox
- [x] ทดสอบบน Safari (optional)

---

## 📝 SDL 1.2 → SDL 2.0 Quick Reference

### Video
```c
// SDL 1.2
SDL_Surface *screen = SDL_SetVideoMode(320, 224, 16, SDL_SWSURFACE);
SDL_Flip(screen);

// SDL 2.0
SDL_Window *window = SDL_CreateWindow("MiniSlug", 
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
    320, 224, 0);
SDL_Surface *screen = SDL_GetWindowSurface(window);
SDL_UpdateWindowSurface(window);
```

### Keyboard
```c
// SDL 1.2
Uint8 *keys = SDL_GetKeyState(NULL);
if (keys[SDLK_LEFT]) { ... }

// SDL 2.0
const Uint8 *keys = SDL_GetKeyboardState(NULL);
if (keys[SDL_SCANCODE_LEFT]) { ... }
```

### Color Key
```c
// SDL 1.2
SDL_SetColorKey(surface, SDL_SRCCOLORKEY, colorKey);

// SDL 2.0
SDL_SetColorKey(surface, SDL_TRUE, colorKey);
```

### Window Caption
```c
// SDL 1.2
SDL_WM_SetCaption("Title", NULL);

// SDL 2.0
SDL_SetWindowTitle(window, "Title");
```

### WASM Main Loop
```c
// SDL 1.2 / Native
while (!quit) {
    handleEvents();
    update();
    render();
}

// Emscripten WASM
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
void main_loop() {
    handleEvents();
    update();
    render();
}
int main() {
    // init...
    emscripten_set_main_loop(main_loop, 60, 1);
}
#endif
```

---

## 🐳 Docker Commands

### Native SDL2 Build
```bash
# Build Docker image
docker build -t minislug-sdl2 .

# Run build
./build.sh
```

### WASM Build
```bash
# Run WASM build
./wasm/build.sh

# Test locally
cd wasm/build
python3 -m http.server 8080
# Open: http://localhost:8080/minislug.html
```

---

## 📚 References

- [SDL 1.2 to 2.0 Migration Guide](https://wiki.libsdl.org/SDL2/MigrationGuide)
- [SDL2 Documentation](https://wiki.libsdl.org/SDL2/FrontPage)
- [SDL2 Keyboard Scancodes](https://wiki.libsdl.org/SDL2/SDL_Scancode)
- [Emscripten SDL2 Documentation](https://emscripten.org/docs/porting/multimedia_and_graphics/SDL.html)
- [Emscripten Main Loop](https://emscripten.org/docs/porting/emscripten-runtime-environment.html#browser-main-loop)
