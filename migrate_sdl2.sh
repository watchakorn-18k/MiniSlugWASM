#!/bin/bash

# SDL 1.2 to SDL 2.0 Migration Script for MiniSlug
# Run this script from the project root directory

set -e

echo "🚀 Starting SDL 1.2 → SDL 2.0 Migration..."
echo ""

cd minislug0

# ==== Phase 1: Simple text replacements (safe) ====
echo "📝 Phase 1: Simple text replacements..."

# 1.1 Replace SDL_SWSURFACE with 0 (it's not needed in SDL2)
# Note: SDL_CreateRGBSurface in SDL2 uses 0 for flags
echo "  - Replacing SDL_SWSURFACE → 0"
# We'll keep SDL_SWSURFACE as 0 for now, it's still valid in SDL2

# 1.2 Replace SDL_SRCCOLORKEY with SDL_TRUE
echo "  - Replacing SDL_SRCCOLORKEY → SDL_TRUE"
sed -i.bak 's/SDL_SRCCOLORKEY/SDL_TRUE/g' scroll.c

# 1.3 Replace SDLK_LAST with SDL_NUM_SCANCODES
echo "  - Replacing SDLK_LAST → SDL_NUM_SCANCODES"
sed -i.bak 's/SDLK_LAST/SDL_NUM_SCANCODES/g' main.c

# 1.4 Replace SDL_Flip with SDL_UpdateWindowSurface
echo "  - Replacing SDL_Flip → SDL_UpdateWindowSurface"
sed -i.bak 's/SDL_Flip(\([^)]*\))/SDL_UpdateWindowSurface(gVar.pWindow)/g' main.c

# 1.5 Replace SDL_WM_SetCaption
echo "  - Replacing SDL_WM_SetCaption → SDL_SetWindowTitle"
sed -i.bak 's/SDL_WM_SetCaption(\("[^"]*"\), NULL)/SDL_SetWindowTitle(gVar.pWindow, \1)/g' main.c

echo ""
echo "✅ Phase 1 Complete!"
echo ""

# ==== Phase 2: Keycode replacements ====
echo "📝 Phase 2: Keycode replacements (SDLK_* → SDL_SCANCODE_*)..."

# List of files that use SDLK_ keycodes
FILES_TO_FIX="main.c game.c menu.c boss.c monsters50.c"

for file in $FILES_TO_FIX; do
    echo "  - Processing $file"
    
    # Direction keys
    sed -i.bak 's/SDLK_LEFT/SDL_SCANCODE_LEFT/g' "$file"
    sed -i.bak 's/SDLK_RIGHT/SDL_SCANCODE_RIGHT/g' "$file"
    sed -i.bak 's/SDLK_UP/SDL_SCANCODE_UP/g' "$file"
    sed -i.bak 's/SDLK_DOWN/SDL_SCANCODE_DOWN/g' "$file"
    
    # Common keys
    sed -i.bak 's/SDLK_RETURN/SDL_SCANCODE_RETURN/g' "$file"
    sed -i.bak 's/SDLK_SPACE/SDL_SCANCODE_SPACE/g' "$file"
    sed -i.bak 's/SDLK_ESCAPE/SDL_SCANCODE_ESCAPE/g' "$file"
    sed -i.bak 's/SDLK_BACKSPACE/SDL_SCANCODE_BACKSPACE/g' "$file"
    
    # Letter keys (a-z)
    sed -i.bak 's/SDLK_a/SDL_SCANCODE_A/g' "$file"
    sed -i.bak 's/SDLK_b/SDL_SCANCODE_B/g' "$file"
    sed -i.bak 's/SDLK_c/SDL_SCANCODE_C/g' "$file"
    sed -i.bak 's/SDLK_d/SDL_SCANCODE_D/g' "$file"
    sed -i.bak 's/SDLK_e/SDL_SCANCODE_E/g' "$file"
    sed -i.bak 's/SDLK_f/SDL_SCANCODE_F/g' "$file"
    sed -i.bak 's/SDLK_g/SDL_SCANCODE_G/g' "$file"
    sed -i.bak 's/SDLK_h/SDL_SCANCODE_H/g' "$file"
    sed -i.bak 's/SDLK_i/SDL_SCANCODE_I/g' "$file"
    sed -i.bak 's/SDLK_j/SDL_SCANCODE_J/g' "$file"
    sed -i.bak 's/SDLK_k/SDL_SCANCODE_K/g' "$file"
    sed -i.bak 's/SDLK_l/SDL_SCANCODE_L/g' "$file"
    sed -i.bak 's/SDLK_m/SDL_SCANCODE_M/g' "$file"
    sed -i.bak 's/SDLK_n/SDL_SCANCODE_N/g' "$file"
    sed -i.bak 's/SDLK_o/SDL_SCANCODE_O/g' "$file"
    sed -i.bak 's/SDLK_p/SDL_SCANCODE_P/g' "$file"
    sed -i.bak 's/SDLK_q/SDL_SCANCODE_Q/g' "$file"
    sed -i.bak 's/SDLK_r/SDL_SCANCODE_R/g' "$file"
    sed -i.bak 's/SDLK_s/SDL_SCANCODE_S/g' "$file"
    sed -i.bak 's/SDLK_t/SDL_SCANCODE_T/g' "$file"
    sed -i.bak 's/SDLK_u/SDL_SCANCODE_U/g' "$file"
    sed -i.bak 's/SDLK_v/SDL_SCANCODE_V/g' "$file"
    sed -i.bak 's/SDLK_w/SDL_SCANCODE_W/g' "$file"
    sed -i.bak 's/SDLK_x/SDL_SCANCODE_X/g' "$file"
    sed -i.bak 's/SDLK_y/SDL_SCANCODE_Y/g' "$file"
    sed -i.bak 's/SDLK_z/SDL_SCANCODE_Z/g' "$file"
    
    # Number keys
    sed -i.bak 's/SDLK_0/SDL_SCANCODE_0/g' "$file"
    sed -i.bak 's/SDLK_1/SDL_SCANCODE_1/g' "$file"
    sed -i.bak 's/SDLK_2/SDL_SCANCODE_2/g' "$file"
    sed -i.bak 's/SDLK_3/SDL_SCANCODE_3/g' "$file"
    sed -i.bak 's/SDLK_4/SDL_SCANCODE_4/g' "$file"
    sed -i.bak 's/SDLK_5/SDL_SCANCODE_5/g' "$file"
    sed -i.bak 's/SDLK_6/SDL_SCANCODE_6/g' "$file"
    sed -i.bak 's/SDLK_7/SDL_SCANCODE_7/g' "$file"
    sed -i.bak 's/SDLK_8/SDL_SCANCODE_8/g' "$file"
    sed -i.bak 's/SDLK_9/SDL_SCANCODE_9/g' "$file"
    
    # Function keys
    sed -i.bak 's/SDLK_F1/SDL_SCANCODE_F1/g' "$file"
    sed -i.bak 's/SDLK_F2/SDL_SCANCODE_F2/g' "$file"
    sed -i.bak 's/SDLK_F3/SDL_SCANCODE_F3/g' "$file"
    sed -i.bak 's/SDLK_F4/SDL_SCANCODE_F4/g' "$file"
    sed -i.bak 's/SDLK_F5/SDL_SCANCODE_F5/g' "$file"
    sed -i.bak 's/SDLK_F6/SDL_SCANCODE_F6/g' "$file"
    sed -i.bak 's/SDLK_F7/SDL_SCANCODE_F7/g' "$file"
    sed -i.bak 's/SDLK_F8/SDL_SCANCODE_F8/g' "$file"
    sed -i.bak 's/SDLK_F9/SDL_SCANCODE_F9/g' "$file"
    sed -i.bak 's/SDLK_F10/SDL_SCANCODE_F10/g' "$file"
    sed -i.bak 's/SDLK_F11/SDL_SCANCODE_F11/g' "$file"
    sed -i.bak 's/SDLK_F12/SDL_SCANCODE_F12/g' "$file"
    
    # Modifier keys
    sed -i.bak 's/SDLK_LSHIFT/SDL_SCANCODE_LSHIFT/g' "$file"
    sed -i.bak 's/SDLK_RSHIFT/SDL_SCANCODE_RSHIFT/g' "$file"
    sed -i.bak 's/SDLK_LCTRL/SDL_SCANCODE_LCTRL/g' "$file"
    sed -i.bak 's/SDLK_RCTRL/SDL_SCANCODE_RCTRL/g' "$file"
    sed -i.bak 's/SDLK_LALT/SDL_SCANCODE_LALT/g' "$file"
    sed -i.bak 's/SDLK_RALT/SDL_SCANCODE_RALT/g' "$file"
    
    # Keypad keys
    sed -i.bak 's/SDLK_KP0/SDL_SCANCODE_KP_0/g' "$file"
    sed -i.bak 's/SDLK_KP1/SDL_SCANCODE_KP_1/g' "$file"
    sed -i.bak 's/SDLK_KP2/SDL_SCANCODE_KP_2/g' "$file"
    sed -i.bak 's/SDLK_KP3/SDL_SCANCODE_KP_3/g' "$file"
    sed -i.bak 's/SDLK_KP4/SDL_SCANCODE_KP_4/g' "$file"
    sed -i.bak 's/SDLK_KP5/SDL_SCANCODE_KP_5/g' "$file"
    sed -i.bak 's/SDLK_KP6/SDL_SCANCODE_KP_6/g' "$file"
    sed -i.bak 's/SDLK_KP7/SDL_SCANCODE_KP_7/g' "$file"
    sed -i.bak 's/SDLK_KP8/SDL_SCANCODE_KP_8/g' "$file"
    sed -i.bak 's/SDLK_KP9/SDL_SCANCODE_KP_9/g' "$file"
    sed -i.bak 's/SDLK_KP_DIVIDE/SDL_SCANCODE_KP_DIVIDE/g' "$file"
    sed -i.bak 's/SDLK_KP_MULTIPLY/SDL_SCANCODE_KP_MULTIPLY/g' "$file"
    sed -i.bak 's/SDLK_KP_MINUS/SDL_SCANCODE_KP_MINUS/g' "$file"
    sed -i.bak 's/SDLK_KP_PLUS/SDL_SCANCODE_KP_PLUS/g' "$file"
    sed -i.bak 's/SDLK_KP_PERIOD/SDL_SCANCODE_KP_PERIOD/g' "$file"
    
    # Special keys
    sed -i.bak 's/SDLK_SEMICOLON/SDL_SCANCODE_SEMICOLON/g' "$file"
    sed -i.bak 's/SDLK_PERIOD/SDL_SCANCODE_PERIOD/g' "$file"
    sed -i.bak 's/SDLK_COMMA/SDL_SCANCODE_COMMA/g' "$file"
    sed -i.bak 's/SDLK_SLASH/SDL_SCANCODE_SLASH/g' "$file"
    sed -i.bak 's/SDLK_RIGHTBRACKET/SDL_SCANCODE_RIGHTBRACKET/g' "$file"
    sed -i.bak 's/SDLK_LEFTBRACKET/SDL_SCANCODE_LEFTBRACKET/g' "$file"
    sed -i.bak 's/SDLK_EQUALS/SDL_SCANCODE_EQUALS/g' "$file"
    sed -i.bak 's/SDLK_MINUS/SDL_SCANCODE_MINUS/g' "$file"
done

echo ""
echo "✅ Phase 2 Complete!"
echo ""

# Clean up backup files
echo "🧹 Cleaning up backup files..."
rm -f *.bak

echo ""
echo "✅ Simple replacements done!"
echo ""
echo "⚠️  IMPORTANT: Manual changes still required in main.c:"
echo "   1. VideoModeSet() function needs complete rewrite"
echo "   2. SDL_GetVideoInfo() must be removed"
echo "   3. Window creation and management needs SDL2 API"
echo ""
