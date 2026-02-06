/*
 * text.c
 *
 *  Created on: Feb 1, 2026
 *      Author: mateusz
 */

#include "draw/text.h"
#include <SDL2/SDL_ttf.h>

#include <stdio.h>
#include <stdint.h>

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

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

void text_draw_altitude_or_fl(SDL_Renderer *renderer, const int font_size, int x, int y, unsigned altitude, bool true_for_fl)
{
	char text[6]; // "FLxyz\0"
	memset (text, 0x00, 6);

	if (true_for_fl) {
		snprintf(text, 6, "FL%03d", (uint8_t)(altitude / 100));
	}
	else {
		snprintf(text, 6, "A%03d", (uint8_t)(altitude / 100));
	}

	text_draw(renderer, font_size, text ,x, y);
}
