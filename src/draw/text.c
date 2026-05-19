/*
 * text.c
 *
 *  Created on: Feb 1, 2026
 *      Author: mateusz
 */

#include "draw/text.h"

#include <SDL2/SDL_ttf.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

#define TEXT_BUFFER_SHORT_LN (512)

//!< Maximum number of distinct font sizes cached at once. Opening a TTF font is
//!< expensive (file IO + glyph table build), so we keep each opened size around
//!< for the lifetime of the program and release them all in text_shutdown().
#define TEXT_FONT_CACHE_SIZE (8)

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

/**
 * @brief Single entry of the font cache. An entry is in use when @c font != NULL.
 */
typedef struct text_font_cache_entry_t {
	int font_size;
	TTF_Font *font;
} text_font_cache_entry_t;

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

static char text_buffer_short[TEXT_BUFFER_SHORT_LN] = {0U};

static text_font_cache_entry_t text_font_cache[TEXT_FONT_CACHE_SIZE] = {0U};

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/**
 * @brief Returns a cached TTF_Font for the given size, opening (and caching) it on the
 * first call. Returns NULL if the font file cannot be opened or the cache is full.
 */
static TTF_Font *text_get_font (int font_size)
{
	// look for an already-opened font with this size
	for (int i = 0; i < TEXT_FONT_CACHE_SIZE; i++) {
		if (text_font_cache[i].font != NULL && text_font_cache[i].font_size == font_size) {
			return text_font_cache[i].font;
		}
	}

	// not cached yet, find a free slot
	for (int i = 0; i < TEXT_FONT_CACHE_SIZE; i++) {
		if (text_font_cache[i].font == NULL) {
			TTF_Font *opened = TTF_OpenFont ("DejaVuSansMono.ttf", font_size);
			if (opened == NULL) {
				printf ("error loading font \r\n");
				return NULL;
			}
			text_font_cache[i].font = opened;
			text_font_cache[i].font_size = font_size;
			return opened;
		}
	}

	// cache full, refuse rather than thrash
	return NULL;
}

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 * Draw given text on the screen
 * @param renderer pointer to globally used renderer
 * @param font_size to be used to draw
 * @param text null terminated string. There is no size limit per-se
 * @param x display coordinate to draw this text at
 * @param y display coordinate to draw this text at
 */
void text_draw (SDL_Renderer *renderer, const int font_size, const char *text, int x, int y)
{
	TTF_Font *font = text_get_font (font_size);

	if (font == NULL) {
		return;
	}

	const SDL_Color foregroundColor = {255, 255, 255, 0};
	const SDL_Color backgroundColor = {0, 0, 0, 0};

	SDL_Surface *textSurface = TTF_RenderText_Shaded (font, text, foregroundColor, backgroundColor);

	if (textSurface == NULL) {
		return;
	}

	// Set rendering space and render to screen
	const SDL_Rect renderQuad = {x, y, textSurface->w, textSurface->h};

	SDL_Texture *mTexture = SDL_CreateTextureFromSurface (renderer, textSurface);
	if (mTexture != NULL) {
		SDL_RenderCopyEx (renderer, mTexture, NULL, &renderQuad, 0, NULL, SDL_FLIP_NONE);
		SDL_DestroyTexture (mTexture);
	}

	SDL_FreeSurface (textSurface);
}

/**
 * Draw the text on the screen, taking into account a size limit. If a value of ln is smaller than
 * strlen(text), this function will use intermediate buffer to extract first ln characters from the
 * string.
 * @param renderer pointer to globally used renderer
 * @param font_size to be used to draw
 * @param text null terminated string.
 * @param ln maximum size to be printed. may be less than strlen(text)
 * @param x display coordinate to draw this text at
 * @param y display coordinate to draw this text at
 */
void text_ndraw (SDL_Renderer *renderer, const int font_size, const char *text, size_t ln, int x,
				 int y)
{
	// clamp caller-provided length to the static buffer so we cannot write past it
	if (ln >= TEXT_BUFFER_SHORT_LN) {
		ln = TEXT_BUFFER_SHORT_LN - 1;
	}

	if (strlen (text) > ln) {
		memcpy (text_buffer_short, text, ln);
		text_buffer_short[ln] = '\0';
		text_draw (renderer, font_size, text_buffer_short, x, y);
	}
	else {
		text_draw (renderer, font_size, text, x, y);
	}
}

/**
 *
 * @param renderer
 * @param font_size
 * @param x
 * @param y
 * @param altitude
 * @param true_for_fl
 */
void text_draw_altitude_or_fl (SDL_Renderer *renderer, const int font_size, int x, int y,
							   unsigned altitude, bool true_for_fl)
{
	// "FLxxxx\0" or "Axxxx\0" - 8 bytes leaves headroom up to 999900 ft and beyond
	char text[8];
	memset (text, 0x00, sizeof (text));

	if (true_for_fl) {
		snprintf (text, sizeof (text), "FL%03u", altitude / 100U);
	}
	else {
		snprintf (text, sizeof (text), "A%03u", altitude / 100U);
	}

	text_draw (renderer, font_size, text, x, y);
}

void text_shutdown (void)
{
	for (int i = 0; i < TEXT_FONT_CACHE_SIZE; i++) {
		if (text_font_cache[i].font != NULL) {
			TTF_CloseFont (text_font_cache[i].font);
			text_font_cache[i].font = NULL;
			text_font_cache[i].font_size = 0;
		}
	}
}
