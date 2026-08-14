#include "includes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

static stbtt_fontinfo g_font;
static unsigned char *g_font_buffer = NULL;
static int g_font_ready = 0;
static float g_font_scale = 0.0f;
static int g_font_ascent = 0;
static int g_font_descent = 0;
static int g_font_lineGap = 0;

// Optimal font height for Google Font Kanit on 320x240 screen
#define FONT_PIXEL_HEIGHT 15.0f

// Text Queue for deferred rendering over sprites & backgrounds
#define MAX_TEXT_QUEUE 256
typedef struct {
	s32 x;
	s32 y;
	u32 flags;
	char text[256];
} STextQueueItem;

static STextQueueItem g_text_queue[MAX_TEXT_QUEUE];
static int g_text_queue_count = 0;

static int Font_Init(void)
{
	if (g_font_ready) return 1;

	const char *paths[] = {
		"Kanit-Bold.ttf",
		"minislug0/Kanit-Bold.ttf",
		"../minislug0/Kanit-Bold.ttf",
		"/app/minislug0/Kanit-Bold.ttf",
		NULL
	};

	FILE *fp = NULL;
	for (int i = 0; paths[i] != NULL; i++) {
		fp = fopen(paths[i], "rb");
		if (fp) {
			printf("[Font] Loaded Kanit font: %s\n", paths[i]);
			break;
		}
	}

	if (!fp) {
		fprintf(stderr, "[Font] Kanit-Bold.ttf not found.\n");
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	g_font_buffer = (unsigned char *)malloc(size);
	if (!g_font_buffer) {
		fclose(fp);
		return 0;
	}

	if (fread(g_font_buffer, 1, size, fp) != (size_t)size) {
		fclose(fp);
		free(g_font_buffer);
		g_font_buffer = NULL;
		return 0;
	}
	fclose(fp);

	if (!stbtt_InitFont(&g_font, g_font_buffer, stbtt_GetFontOffsetForIndex(g_font_buffer, 0))) {
		free(g_font_buffer);
		g_font_buffer = NULL;
		return 0;
	}

	g_font_scale = stbtt_ScaleForPixelHeight(&g_font, FONT_PIXEL_HEIGHT);
	stbtt_GetFontVMetrics(&g_font, &g_font_ascent, &g_font_descent, &g_font_lineGap);
	g_font_ready = 1;
	printf("[Font] Google Font Kanit (Bold) initialized successfully!\n");
	return 1;
}

static unsigned int utf8_next_codepoint(const char **pStr)
{
	const unsigned char *s = (const unsigned char *)*pStr;
	if (!*s) return 0;
	unsigned int cp = 0;
	if (*s < 0x80) {
		cp = *s++;
	} else if ((*s & 0xE0) == 0xC0) {
		if ((s[1] & 0xC0) == 0x80) {
			cp = ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
			s += 2;
		} else {
			cp = *s++;
		}
	} else if ((*s & 0xF0) == 0xE0) {
		if ((s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80) {
			cp = ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
			s += 3;
		} else {
			cp = *s++;
		}
	} else if ((*s & 0xF8) == 0xF0) {
		if ((s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80 && (s[3] & 0xC0) == 0x80) {
			cp = ((s[0] & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
			s += 4;
		} else {
			cp = *s++;
		}
	} else {
		cp = *s++;
	}
	*pStr = (const char *)s;
	return cp;
}

// Thai character category classifiers
static int is_upper_vowel(unsigned int cp) {
	return (cp == 0x0E31 || (cp >= 0x0E34 && cp <= 0x0E37) || cp == 0x0E47 || cp == 0x0E4D);
}
static int is_tone_or_garan(unsigned int cp) {
	return (cp >= 0x0E48 && cp <= 0x0E4C);
}
static int is_lower_mark(unsigned int cp) {
	return (cp >= 0x0E38 && cp <= 0x0E3A);
}
static int is_ascender_consonant(unsigned int cp) {
	return (cp == 0x0E1B || cp == 0x0E1D || cp == 0x0E1F || cp == 0x0E2C); // ป, ฝ, ฟ, ฬ
}
static int is_descender_consonant(unsigned int cp) {
	return (cp == 0x0E0E || cp == 0x0E0F || cp == 0x0E10 || cp == 0x0E0D); // ฎ, ฏ, ฐ, ญ
}

void MyItoA(s32 nNb, char *pDst)
{
	char cMin = ' ';
	char *pPtr;
	u32 nTmp;

	if (nNb < 0)
	{
		cMin = '-';
		nNb = -nNb;
	}

	pPtr = pDst + strlen(pDst) - 1;
	nTmp = nNb;
	do
	{
		*pPtr-- = (char)((nTmp % 10) + '0');
	} while (pPtr >= pDst && (nTmp /= 10) > 0);

	if (cMin != ' ' && pPtr >= pDst) *pPtr = cMin;
}

static inline void blend_pixel_16(u16 *pDst, u8 r, u8 g, u8 b, u8 alpha) {
	if (alpha < 15) return;
	if (alpha >= 240) {
		*pDst = (u16)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
		return;
	}
	u16 bg = *pDst;
	u32 bgr = (bg >> 11) & 0x1F;
	u32 bgg = (bg >> 5) & 0x3F;
	u32 bgb = bg & 0x1F;
	bgr = (bgr << 3) | (bgr >> 2);
	bgg = (bgg << 2) | (bgg >> 4);
	bgb = (bgb << 3) | (bgb >> 2);

	u32 inv = 255 - alpha;
	u32 outr = (r * alpha + bgr * inv) >> 8;
	u32 outg = (g * alpha + bgg * inv) >> 8;
	u32 outb = (b * alpha + bgb * inv) >> 8;

	*pDst = (u16)(((outr >> 3) << 11) | ((outg >> 2) << 5) | (outb >> 3));
}

// Crisp, smooth glyph drawing with solid 8-directional black outline
static void draw_smooth_glyph(SDL_Surface *surf, int x, int y, int bw, int bh, unsigned char *bmp, u8 r, u8 g, u8 b)
{
	if (!surf || !bmp) return;
	u16 *pixels = (u16 *)surf->pixels;
	int pitch_pixels = surf->pitch / 2;

	// 1. Draw solid 1px black outline in 8 directions + drop shadow
	static const int ox[] = { -1,  0,  1, -1, 1, -1, 0, 1, 0 };
	static const int oy[] = { -1, -1, -1,  0, 0,  1, 1, 1, 2 };
	for (int k = 0; k < 9; k++) {
		int dx = ox[k];
		int dy = oy[k];
		for (int j = 0; j < bh; j++) {
			int py = y + j + dy;
			if (py < 0 || py >= surf->h) continue;
			for (int i = 0; i < bw; i++) {
				int px = x + i + dx;
				if (px < 0 || px >= surf->w) continue;
				u8 val = bmp[j * bw + i];
				if (val > 35) {
					u8 outline_alpha = (val > 140) ? 255 : (val * 240 / 255);
					blend_pixel_16(&pixels[py * pitch_pixels + px], 0, 0, 0, outline_alpha);
				}
			}
		}
	}

	// 2. Draw core glyph with high-contrast smooth alpha curve
	for (int j = 0; j < bh; j++) {
		int py = y + j;
		if (py < 0 || py >= surf->h) continue;
		for (int i = 0; i < bw; i++) {
			int px = x + i;
			if (px < 0 || px >= surf->w) continue;
			u8 val = bmp[j * bw + i];
			if (val > 20) {
				blend_pixel_16(&pixels[py * pitch_pixels + px], r, g, b, val);
			}
		}
	}
}

// Measure or draw string onto surface with sophisticated Thai multi-tier mark typesetting
static u32 Font_RenderDirect(SDL_Surface *surf, s32 nPosX, s32 nPosY, const char *pStr, u32 nFlags)
{
	if (!pStr || !*pStr) return 0;
	if (!g_font_ready) {
		if (!Font_Init()) return 0;
	}

	float scale = g_font_scale;
	int baseline_y = nPosY + (int)(g_font_ascent * scale + 0.5f);
	s32 cur_x = nPosX;
	s32 prev_base_x = nPosX;
	int prev_advance = 8;
	unsigned int prev_base_cp = 0;
	int had_upper_vowel = 0;

	// Vibrant colors: High contrast White / Bright Golden Yellow
	u8 fg_r = (nFlags & FONT_Highlight) ? 255 : 255;
	u8 fg_g = (nFlags & FONT_Highlight) ? 225 : 255;
	u8 fg_b = (nFlags & FONT_Highlight) ? 35  : 255;

	const char *ptr = pStr;
	while (*ptr)
	{
		unsigned int cp = utf8_next_codepoint(&ptr);
		if (cp == 0) break;

		if (cp == ' ') {
			cur_x += 5;
			prev_base_x = cur_x;
			prev_base_cp = ' ';
			had_upper_vowel = 0;
			continue;
		}

		int glyph = stbtt_FindGlyphIndex(&g_font, cp);
		if (glyph == 0) {
			glyph = stbtt_FindGlyphIndex(&g_font, '?');
		}

		int advanceWidth, leftSideBearing;
		stbtt_GetGlyphHMetrics(&g_font, glyph, &advanceWidth, &leftSideBearing);
		int adv_pixels = (int)(advanceWidth * scale + 0.5f);
		if (adv_pixels < 1) adv_pixels = 1;

		int bw, bh, xoff, yoff;
		unsigned char *bmp = NULL;
		if (surf != NULL) {
			bmp = stbtt_GetGlyphBitmap(&g_font, scale, scale, glyph, &bw, &bh, &xoff, &yoff);
		}

		// Calculate precise multi-tier positioning centered on consonant
		if (is_upper_vowel(cp)) {
			// Upper vowel on Level 1 centered over consonant
			int shift_left = is_ascender_consonant(prev_base_cp) ? 2 : 0;
			int draw_x = prev_base_x + (prev_advance - bw) / 2 - shift_left;
			int draw_y = baseline_y + yoff;
			if (bmp && surf) {
				draw_smooth_glyph(surf, draw_x, draw_y, bw, bh, bmp, fg_r, fg_g, fg_b);
			}
			had_upper_vowel = 1;
		}
		else if (is_tone_or_garan(cp)) {
			// Tone mark on Level 2 if stacked over upper vowel, or Level 1 if directly over consonant
			int shift_left = is_ascender_consonant(prev_base_cp) ? 2 : 0;
			int shift_up = had_upper_vowel ? 4 : 0;
			int draw_x = prev_base_x + (prev_advance - bw) / 2 - shift_left;
			if (!had_upper_vowel && !is_ascender_consonant(prev_base_cp)) draw_x += 1;
			int draw_y = baseline_y + yoff - shift_up;
			if (bmp && surf) {
				draw_smooth_glyph(surf, draw_x, draw_y, bw, bh, bmp, fg_r, fg_g, fg_b);
			}
		}
		else if (is_lower_mark(cp)) {
			// Lower vowel beneath baseline
			int shift_down = is_descender_consonant(prev_base_cp) ? 3 : 0;
			int draw_x = prev_base_x + (prev_advance - bw) / 2;
			int draw_y = baseline_y + yoff + shift_down;
			if (bmp && surf) {
				draw_smooth_glyph(surf, draw_x, draw_y, bw, bh, bmp, fg_r, fg_g, fg_b);
			}
		}
		else {
			// Base consonant or normal Latin/digit character
			int draw_x = cur_x + xoff;
			int draw_y = baseline_y + yoff;
			if (bmp && surf) {
				draw_smooth_glyph(surf, draw_x, draw_y, bw, bh, bmp, fg_r, fg_g, fg_b);
			}
			prev_base_x = cur_x;
			prev_advance = adv_pixels;
			prev_base_cp = cp;
			had_upper_vowel = 0;
			cur_x += adv_pixels;
		}

		if (bmp) {
			stbtt_FreeBitmap(bmp, NULL);
		}
	}

	return (u32)(cur_x - nPosX);
}

// Queue text to be rendered on top of everything before flip
u32 Font_Print(s32 nPosX, s32 nPosY, char *pStr, u32 nFlags)
{
	if (!pStr || !*pStr) return 0;

	// Calculate width
	u32 nWidth = Font_RenderDirect(NULL, nPosX, nPosY, pStr, nFlags);

	if ((nFlags & FONT_NoDisp) == 0) {
		if (g_text_queue_count < MAX_TEXT_QUEUE) {
			STextQueueItem *item = &g_text_queue[g_text_queue_count++];
			item->x = nPosX;
			item->y = nPosY;
			item->flags = nFlags;
			strncpy(item->text, pStr, sizeof(item->text) - 1);
			item->text[sizeof(item->text) - 1] = '\0';
		}
	}

	return nWidth;
}

// Flush all queued text to the screen buffer
void Font_FlushQueue(SDL_Surface *pSurf)
{
	if (!pSurf || g_text_queue_count == 0) return;

	for (int i = 0; i < g_text_queue_count; i++) {
		Font_RenderDirect(pSurf, g_text_queue[i].x, g_text_queue[i].y, g_text_queue[i].text, g_text_queue[i].flags);
	}
	g_text_queue_count = 0;
}

u32 Font_PrintSpc(s32 nPosX, s32 nPosY, char *pStr, u32 nFlags, u32 nSpc)
{
	if (!pStr || !*pStr) return 0;
	s32 nPosXOrg = nPosX;
	const char *ptr = pStr;
	char single[16];

	while (*ptr) {
		const char *prev = ptr;
		unsigned int cp = utf8_next_codepoint(&ptr);
		if (cp == 0) break;
		size_t len = ptr - prev;
		if (len < sizeof(single)) {
			memcpy(single, prev, len);
			single[len] = '\0';
			Font_Print(nPosX, nPosY, single, nFlags);
		}
		nPosX += nSpc;
	}

	return (u32)(nPosX - nPosXOrg);
}
