/*
 * text.h
 *
 *  Created on: Feb 1, 2026
 *      Author: mateusz
 */

#ifndef SRC_DRAW_TEXT_H_
#define SRC_DRAW_TEXT_H_

#include <SDL2/SDL.h>
#include <stdbool.h>

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL VARIABLES
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
void text_draw (SDL_Renderer *renderer, const int font_size, const char *text, int x, int y);

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
void text_ndraw (SDL_Renderer *renderer, const int font_size, const char *text, size_t ln, int x, int y);

/**
 *
 * @param renderer pointer to globally used renderer
 * @param font_size
 * @param x
 * @param y
 * @param altitude
 * @param true_for_fl
 */
void text_draw_altitude_or_fl(SDL_Renderer *renderer, const int font_size, int x, int y, unsigned altitude, bool true_for_fl);

#endif /* SRC_DRAW_TEXT_H_ */
