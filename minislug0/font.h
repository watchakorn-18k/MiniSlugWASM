#ifndef _FONT_H_
#define _FONT_H_

#include <SDL2/SDL.h>

// Flags.
#define	FONT_NoDisp		(1 << 0)
#define	FONT_Highlight	(1 << 1)

// Prototypes.
u32 Font_Print(s32 nPosX, s32 nPosY, char *pStr, u32 nFlags);
u32 Font_PrintSpc(s32 nPosX, s32 nPosY, char *pStr, u32 nFlags, u32 nSpc);
void Font_FlushQueue(SDL_Surface *pSurf);
void MyItoA(s32 nNb, char *pDst);

#endif
