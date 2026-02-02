/*
 * text.c
 *
 *  Created on: Feb 1, 2026
 *      Author: mateusz
 */

#include "draw/text.h"
#include <SDL2/SDL_ttf.h>

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

void draw_text_test (SDL_Renderer *renderer, const int font_size)
{
	TTF_Font *font = TTF_OpenFont ("DejaVuSansMono.ttf", font_size);

	if (font == NULL)
	{
		printf("error loading font \r\n");
		return;
	}

	SDL_Color foregroundColor = {255, 255, 255, 0};
	SDL_Color backgroundColor = {0, 0, 0, 0};

	SDL_Surface *textSurface =
		TTF_RenderText_Shaded (font, "TEST123", foregroundColor, backgroundColor);

	// Set rendering space and render to screen
	SDL_Rect renderQuad = {100, 100, textSurface->w, textSurface->h};

	if (textSurface != NULL)
	{
		SDL_Texture *mTexture = SDL_CreateTextureFromSurface (renderer, textSurface);
		SDL_RenderCopyEx (renderer, mTexture, NULL, &renderQuad, 0, NULL, SDL_FLIP_NONE);

	}

}
