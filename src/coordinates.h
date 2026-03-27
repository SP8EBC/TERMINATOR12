/*
 * coordinates.h
 *
 *  Created on: Feb 2, 2026
 *      Author: mateusz
 */

#ifndef SRC_COORDINATES_H_
#define SRC_COORDINATES_H_

#include <SDL2/SDL.h>

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
 * Converts longitude and latitude to the xy coordinates on the screen
 * @param longitude
 * @param latitude
 * @return a copy of SDL_Point structure set to represenging xy screen coordinates
 */
SDL_Point coordinates_get_point_from_latlon(float longitude, float latitude);


#endif /* SRC_COORDINATES_H_ */
