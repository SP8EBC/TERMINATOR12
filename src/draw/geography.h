/*
 * geography.h
 *
 *  Created on: Feb 12, 2026
 *      Author: mateusz
 */

#ifndef SRC_DRAW_GEOGRAPHY_H_
#define SRC_DRAW_GEOGRAPHY_H_

#include <SDL2/SDL.h>

void geography_draw_mountain (SDL_Renderer *renderer, float lat, float lon, const char *label,
							  size_t label_ln);

#endif /* SRC_DRAW_GEOGRAPHY_H_ */
