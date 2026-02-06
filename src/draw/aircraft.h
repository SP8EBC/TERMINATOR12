/*
 * aircraft.h
 *
 *  Created on: Feb 1, 2026
 *      Author: mateusz
 */

#ifndef SRC_DRAW_AIRCRAFT_H_
#define SRC_DRAW_AIRCRAFT_H_

#include <SDL2/SDL.h>

#include "types/aircraft_stv_t.h"

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
 *
 * @param aircraft state vector, altitude, coordinates etc.
 * @param aircraft
 * @return rectangle at display coordinates representing
 */
SDL_Rect aircraft_draw_w_bearing_line (SDL_Renderer *renderer, aircraft_stv_t *aircraft);

/**
 *
 * @param renderer
 * @param aircraft state vector, altitude, coordinates etc.
 * @param callsign which must be 7 characters and space padded if a string is shorter.
 */
void aircraft_draw_w_bearing_line_label (SDL_Renderer *renderer, aircraft_stv_t *aircraft,
										 char *callsign);

#endif /* SRC_DRAW_AIRCRAFT_H_ */
