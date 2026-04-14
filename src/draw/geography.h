/*
 * geography.h
 *
 *  Created on: Feb 12, 2026
 *      Author: mateusz
 */

#ifndef SRC_DRAW_GEOGRAPHY_H_
#define SRC_DRAW_GEOGRAPHY_H_

#include <SDL2/SDL.h>

#include "types/line_style_t.h"

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

void geography_draw_mountain (SDL_Renderer *renderer, float lat, float lon, const char *label,
							  size_t label_ln);

void geography_draw_longitude_lines(SDL_Renderer *renderer, double step, line_style_t style);

void geography_draw_latitude_lines(SDL_Renderer *renderer, double step, line_style_t style);

#endif /* SRC_DRAW_GEOGRAPHY_H_ */
