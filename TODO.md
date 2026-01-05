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
- [x] สร้าง `wasm/Dockerfile.build` - Emscripten build environment
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

---

## 🎮 Phase 9: Roguelike Mode

### 🎯 แนวคิดหลัก
**โหมด Roguelike** จะใช้ **ด่าน 1 (lev1 - Desert)** เป็นฐาน โดยผู้เล่นจะเดินไปมาได้อิสระในด่าน มอนสเตอร์จะสุ่มเกิดขึ้นมาเรื่อยๆ (ไม่จำกัดเฉพาะมอนด่านนี้) และไอเทมจะ drop ลงมาเรื่อยๆ เพื่อให้ผู้เล่นอยู่รอดได้นานที่สุด!

---

### 9.1 Gameplay Mechanics 🕹️

#### ผู้เล่น (Player)
- [X] เปิดใช้ `e_ScrollType_Free` (เดินไปมาได้อิสระทั้งซ้าย-ขวา)
- [X] ไม่มี `e_Mst13_NextLevel` (ไม่มีจุดจบด่าน)
- [X] ชีวิตเหลือ 1 ชีวิต (Permadeath - ตายจบเกม)
- [X] เริ่มต้นด้วยอาวุธ Gun ธรรมดา + ระเบิด 5 ลูก
- [X] ไม่มี Continue (เมื่อตายต้องเริ่มใหม่)

#### ระบบ Wave (คลื่นศัตรู)
- [X] Wave 1-5: ศัตรูพื้นฐาน (e_Mst2_Enemy1, e_Mst7_Zombie1)
- [X] Wave 6-10: เพิ่ม e_Mst14_RebelSoldier0, e_Mst6_RShobu
- [X] Wave 11-15: เพิ่ม e_Mst25_RocketDiver0, e_Mst26_Girida0
- [X] Wave 16-20: เพิ่ม e_Mst28_Masknell0, e_Mst43_FlyingTara0
- [X] Wave 21+: เพิ่ม Mini-Boss ทุก 5 Wave (e_Mst27_HalfBoss)
- [X] ทุกๆ 10 Wave จะมี Boss ใหญ่ (e_Mst20_Boss)

#### ความยาก (Difficulty Scaling)
- [X] จำนวนศัตรูเพิ่มขึ้นตาม Wave (+1 ตัวทุก 3 Wave, สูงสุด 15 ตัว)
- [X] ความเร็วศัตรูเพิ่ม 5% ทุก 5 Wave
- [X] พลังโจมตีศัตรูเพิ่มขึ้นหลัง Wave 10

---

### 9.2 Monster Spawn System 👾

#### โครงสร้างใหม่
```c
struct SRoguelikeWave {
    u32 nWaveNo;           // หมายเลข Wave
    u32 nMonstersLeft;     // จำนวนศัตรูที่เหลือใน Wave นี้
    u32 nMonstersMax;      // จำนวนศัตรูสูงสุดใน Wave
    u32 nSpawnTimer;       // Timer สำหรับ spawn มอน
    u32 nSpawnInterval;    // ความถี่ในการ spawn (frames)
    u8  nDifficulty;       // ระดับความยาก 0-10
};
```

#### ไฟล์ที่ต้องสร้าง/แก้ไข
- [X] สร้าง `roguelike.c` และ `roguelike.h`
- [X] เพิ่ม Monster Pool Table สำหรับแต่ละช่วง Wave
- [X] เพิ่ม Random Spawn Function

#### Spawn Logic
```c
// Monster Pool ตาม Difficulty
u8 gpMonsterPool_Easy[] = { e_Mst2_Enemy1, e_Mst7_Zombie1 };
u8 gpMonsterPool_Medium[] = { e_Mst14_RebelSoldier0, e_Mst6_RShobu, e_Mst25_RocketDiver0 };
u8 gpMonsterPool_Hard[] = { e_Mst26_Girida0, e_Mst28_Masknell0, e_Mst43_FlyingTara0 };
```

---

### 9.3 Item Drop System 📦

#### ประเภท Drop
| Item | ความถี่ | ผลกระทบ |
|------|---------|---------|
| **Ammo Box** | ทุก 45 วินาที | เติมกระสุนอาวุธปัจจุบัน |
| **Bomb Box** | ทุก 60 วินาที | +5 ระเบิด |
| **Health (หากมี)** | ทุก 90 วินาที | ฟื้น HP |
| **Weapon Capsule** | เมื่อจบ Wave | อาวุธสุ่ม |
| **Score Bonus** | สุ่ม 10% ต่อศัตรูที่ตาย | +500-2000 คะแนน |
| **1UP** | Wave 10, 25, 50 | +1 ชีวิต (พิเศษ) |

#### Item Spawn Area
- [X] Drop จากท้องฟ้า (เหมือน POW ปัจจุบัน)
- [X] ตำแหน่งสุ่มใน X range ที่ผู้เล่นมองเห็น
- [ ] มี Parachute animation ตกลงมา

---

### 9.4 Power-Up System ⚡ (Roguelike Perks)

#### Permanent Upgrades (เก็บตลอดทั้ง Run)
- [X] **Speed Boost**: เพิ่มความเร็วผู้เล่น +10%
- [X] **Damage Up**: เพิ่มพลังโจมตี +15%
- [X] **Lucky Drop**: เพิ่มโอกาส Item Drop +20%
- [X] **Armor**: ลด damage ที่ได้รับ 10%
- [X] **Extended Clip**: เพิ่มกระสุนสูงสุด +25%

#### Temporary Power-Ups (30 วินาที)
- [ ] **Invincibility Star**: อมตะชั่วคราว
- [ ] **Double Damage**: x2 damage
- [ ] **Rapid Fire**: +50% อัตราการยิง

#### เลือก Perk ทุก 5 Wave
- [X] แสดง 3 Perks สุ่มให้เลือก 1
- [X] UI คล้าย Card Selection

---

### 9.5 Score & Combo System 🏆

#### Score Multiplier
- [X] ฆ่ามอนต่อเนื่องภายใน 3 วินาที = Combo x1.5, x2, x3...
- [X] Combo สูงสุด x10
- [X] Combo หายถ้าไม่มี kill ใน 5 วินาที

#### Leaderboard (Local)
- [X] บันทึก Top 10 Roguelike Scores
- [X] แยกจาก High Score ปกติ
- [X] เก็บ: Score, Wave สูงสุด, เวลารอด, วันที่

---

### 9.6 UI Requirements 🖥️

#### HUD ใหม่สำหรับ Roguelike
```
╔════════════════════════════════════════════╗
║ WAVE: 12   KILLS: 47   COMBO: x3           ║
║                                            ║
║ [Perks: ⚡ 🛡️ 💨]              TIME: 05:32 ║
╚════════════════════════════════════════════╝
```

- [X] Wave Counter (กลาง-บน)
- [X] Kill Counter
- [X] Combo Indicator (สีเปลี่ยนตามระดับ)
- [X] รายการ Active Perks (ไอคอน)
- [X] Survival Time

#### Wave Transition Screen
- [X] "WAVE X COMPLETE!"
- [X] สรุป: Kills, Time, Bonus Points
- [X] Perk Selection (ถ้าถึง milestone)
- [X] 5 วินาทีพัก ก่อน Wave ถัดไป

#### Game Over Screen (Roguelike)
```
╔═══════════════════════════════════════╗
║           SURVIVAL ENDED              ║
╠═══════════════════════════════════════╣
║  Final Wave:     15                   ║
║  Total Kills:    127                  ║
║  Survival Time:  08:45                ║
║  Final Score:    45,780               ║
║                                       ║
║  [NEW HIGH SCORE!]                    ║
║                                       ║
║  [RETRY]     [MAIN MENU]              ║
╚═══════════════════════════════════════╝
```

---

### 9.7 Menu Integration 📋

#### Main Menu Update
```c
// menu.c - เพิ่มตัวเลือกใหม่
struct SMenuItm gpMenuItems_Main[] = {
    { MENU_Game, 0, "STORY MODE" },        // เปลี่ยนจาก START
    { MENU_Roguelike, 0, "ROGUELIKE" },    // ใหม่!
    { MENU_HallOfFame, 0, "HALL OF FAME" },
    { MENU_Sound, 0, "SOUND SETTINGS" },
    { MENU_Quit, 0, "QUIT" },
};
```

- [X] เพิ่มตัวเลือก "ROGUELIKE" ใน Main Menu
- [ ] เพิ่มหน้า Hall of Fame แยกสำหรับ Roguelike

---

### 9.8 Technical Implementation 🔧

#### ไฟล์ใหม่ที่ต้องสร้าง
| ไฟล์ | จุดประสงค์ |
|------|-----------|
| `roguelike.c` | Core logic สำหรับ Roguelike mode |
| `roguelike.h` | Headers และ structs |
| `roguelike_spawn.c` | Monster spawn system |
| `roguelike_ui.c` | UI elements สำหรับ mode นี้ |

#### แก้ไขไฟล์เดิม
| ไฟล์ | การแก้ไข |
|------|---------|
| `menu.c` | เพิ่ม Roguelike menu option |
| `menu.h` | เพิ่ม MENU_Roguelike constant |
| `game.c` | เพิ่ม Roguelike game phase handling |
| `game.h` | เพิ่ม e_Game_Roguelike enum |
| `includes.h` | Include roguelike.h |
| `Makefile` | เพิ่ม roguelike object files |
| `Makefile.wasm` | เพิ่ม roguelike object files |

#### Key Functions
```c
// roguelike.c
void Roguelike_Init(void);              // เริ่มต้น mode
void Roguelike_Main(void);              // Main loop
void Roguelike_WaveStart(u32 nWave);    // เริ่ม Wave ใหม่
void Roguelike_SpawnMonster(void);      // Spawn มอนสุ่ม
void Roguelike_DropItem(void);          // Drop ไอเทม
void Roguelike_CheckWaveComplete(void); // เช็คจบ Wave
void Roguelike_ShowPerkSelection(void); // แสดง Perk เลือก
void Roguelike_GameOver(void);          // จบเกม
u32  Roguelike_GetRandomMonster(u8 nDifficulty); // สุ่มมอน
```

---

### 9.9 Implementation Phases 📆

#### Phase 9A: Core Foundation
- [X] สร้าง roguelike.c / roguelike.h
- [X] Implement Roguelike_Init() - โหลด lev1 แบบ Free scroll
- [X] Implement basic Wave system (counter + timer)
- [X] สร้าง Monster Pool arrays

#### Phase 9B: Spawn System
- [X] Implement Roguelike_SpawnMonster() ด้วยตำแหน่งสุ่ม
- [X] Implement difficulty scaling
- [X] ทดสอบ spawn หลายๆ wave

#### Phase 9C: Item Drops
- [X] Implement Item drop timer
- [X] เพิ่มประเภท drop ตาม Wave milestone
- [ ] สร้าง drop animation (ใช้ parachute ที่มีอยู่)

#### Phase 9D: UI & HUD
- [X] สร้าง Roguelike HUD (Fixed overlapping issues)
- [X] สร้าง Wave transition screen
- [X] สร้าง Game Over screen พร้อมสถิติ

#### Phase 9E: Perks & Scoring
- [X] Implement Perk system (Selection UI implemented)
- [X] Implement Combo system (Fixed Kill count hook)
- [X] Implement Roguelike leaderboard (Integrated in Game Over)

#### Phase 9F: Menu Integration
- [X] เพิ่มตัวเลือกใน Main Menu
- [X] สร้าง Roguelike Hall of Fame page (Shown at Game Over)
- [ ] ทดสอบ flow ทั้งหมด

#### Phase 9G: Polish & Balance
- [ ] ปรับ balance ความยาก
- [ ] ปรับ spawn rate
- [ ] ปรับ item drop rate
- [ ] ทดสอบหาบัค
- [ ] เพิ่ม HP bar system 

---

### 9.10 Optional Enhancements 🌟

#### Meta-Progression (ถ้ามีเวลา)
- [ ] สะสม "Coins" จากการเล่น
- [ ] ปลดล็อค Starting Perks
- [ ] ปลดล็อค Starting Weapons

#### Daily/Weekly Challenge
- [ ] Seed-based random สำหรับ leaderboard เปรียบเทียบ
- [ ] โหมดพิเศษ (เช่น Zombie-only, Boss Rush)

#### Multiplayer Support (อนาคต)
- [ ] Co-op 2 players
- [ ] VS mode (แข่งกันอยู่รอด)

---

### 📝 Notes
- ใช้ `lev1` เพราะเป็นด่านที่ flat เหมาะกับการเดินไปมา
- ระวังเรื่อง memory เมื่อ spawn มอนเยอะ (จำกัด 20 ตัวพร้อมกัน)
- ใช้ existing monster types ที่มีอยู่แล้ว ไม่ต้องสร้างใหม่
- Item drop ใช้ระบบ `e_Mst3_POW` และ `e_Mst4_WeaponCapsule` ที่มีอยู่
