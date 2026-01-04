/* Dummy YM Library for SDL2 Migration */
#include <stdio.h>
#include "StSoundLibrary.h"

// Define a dummy structure so specific pointers work if invoked
struct YMMUSIC {
    int dummy_data;
};
static struct YMMUSIC g_DummyMusic;

YMMUSIC* ymMusicCreate() {
    printf("DEBUG: ymMusicCreate() called. Returning dummy pointer.\n");
    return &g_DummyMusic;
}

void ymMusicDestroy(YMMUSIC *pMusic) {
    // printf("DEBUG: ymMusicDestroy()\n");
}

ymbool ymMusicLoad(YMMUSIC *pMusic, const char *fName) {
    printf("DEBUG: ymMusicLoad(%s) called. Returning TRUE.\n", fName);
    return 1; // Return TRUE to simulate success
}

ymbool ymMusicCompute(YMMUSIC *pMusic, ymsample *pBuffer, ymint nbSample) {
    // Fill buffer with silence (0)
    if (pBuffer) {
        // memset(pBuffer, 0, nbSample * sizeof(ymsample));
        // Manual loop if memset is an issue (it shouldn't be, but this is a dummy file)
        for (int i=0; i<nbSample; i++) pBuffer[i] = 0;
    }
    return 1;
}

void ymMusicSetLoopMode(YMMUSIC *pMusic, ymbool bLoop) {}
void ymMusicStop(YMMUSIC *pMusic) {}
ymbool ymMusicIsOver(YMMUSIC *pMusic) { return 0; }
void ymMusicRestart(YMMUSIC *pMusic) {}
void ymMusicSetLowpassFiler(YMMUSIC *pMus, ymbool bActive) {}
const char* ymMusicGetLastError(YMMUSIC *pMusic) { return "No Error (Dummy)"; }
int ymMusicGetRegister(YMMUSIC *pMusic, ymint reg) { return 0; }
void ymMusicGetInfo(YMMUSIC *pMusic, ymMusicInfo_t *pInfo) {}
void ymMusicPlay(YMMUSIC *pMusic) {}
void ymMusicPause(YMMUSIC *pMusic) {}
ymbool ymMusicIsSeekable(YMMUSIC *pMusic) { return 0; }
ymu32 ymMusicGetPos(YMMUSIC *pMusic) { return 0; }
void ymMusicSeek(YMMUSIC *pMusic, ymu32 timeInMs) {}
