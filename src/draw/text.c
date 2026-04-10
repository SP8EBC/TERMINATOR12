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

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

#define TEXT_BUFFER_SHORT_LN (512)

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

static char text_buffer_short[TEXT_BUFFER_SHORT_LN] = {0U};

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

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
	TTF_Font *font = TTF_OpenFont ("DejaVuSansMono.ttf", font_size);

	if (font == NULL) {
		printf ("error loading font \r\n");
		return;
	}

	SDL_Color foregroundColor = {255, 255, 255, 0};
	SDL_Color backgroundColor = {0, 0, 0, 0};

	SDL_Surface *textSurface = TTF_RenderText_Shaded (font, text, foregroundColor, backgroundColor);

	// Set rendering space and render to screen
	SDL_Rect renderQuad = {x, y, textSurface->w, textSurface->h};

	if (textSurface != NULL) {
		SDL_Texture *mTexture = SDL_CreateTextureFromSurface (renderer, textSurface);
		SDL_RenderCopyEx (renderer, mTexture, NULL, &renderQuad, 0, NULL, SDL_FLIP_NONE);
	}
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
	if (strlen (text) > ln) {
		memset (text_buffer_short, 0x00, ln + 1);
		strncpy (text_buffer_short, text, ln);
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
	char text[6]; // "FLxyz\0"
	memset (text, 0x00, 6);

	if (true_for_fl) {
		snprintf (text, 6, "FL%03d", (uint8_t)(altitude / 100));
	}
	else {
		snprintf (text, 6, "A%03d", (uint8_t)(altitude / 100));
	}

	text_draw (renderer, font_size, text, x, y);
}
