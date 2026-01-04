/* Dummy YM Library for SDL2 Migration */
#include "StSoundLibrary.h"

YMMUSIC* ymMusicCreate() { return (YMMUSIC*)0; }
void ymMusicDestroy(YMMUSIC *pMusic) {}
ymbool ymMusicLoad(YMMUSIC *pMusic, const char *fName) { return 0; }
ymbool ymMusicCompute(YMMUSIC *pMusic, ymsample *pBuffer, ymint nbSample) { return 0; }
void ymMusicSetLoopMode(YMMUSIC *pMusic, ymbool bLoop) {}
void ymMusicStop(YMMUSIC *pMusic) {}
ymbool ymMusicIsOver(YMMUSIC *pMusic) { return 0; }
void ymMusicRestart(YMMUSIC *pMusic) {}
void ymMusicSetLowpassFiler(YMMUSIC *pMus, ymbool bActive) {}
const char* ymMusicGetLastError(YMMUSIC *pMusic) { return ""; }
int ymMusicGetRegister(YMMUSIC *pMusic, ymint reg) { return 0; }
void ymMusicGetInfo(YMMUSIC *pMusic, ymMusicInfo_t *pInfo) {}
void ymMusicPlay(YMMUSIC *pMusic) {}
void ymMusicPause(YMMUSIC *pMusic) {}
ymbool ymMusicIsSeekable(YMMUSIC *pMusic) { return 0; }
ymu32 ymMusicGetPos(YMMUSIC *pMusic) { return 0; }
void ymMusicSeek(YMMUSIC *pMusic, ymu32 timeInMs) {}
