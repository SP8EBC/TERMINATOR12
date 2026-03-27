/*
 * coordinates.c
 *
 *  Created on: Feb 2, 2026
 *      Author: mateusz
 */

#include "coordinates.h"

/**
 * Converts longitude and latitude to the xy coordinates on the screen
 * @param longitude
 * @param latitude
 * @return a copy of SDL_Point structure set to represenging xy screen coordinates
 */
SDL_Point coordinates_get_point_from_latlon(float longitude, float latitude)
{
	SDL_Point out = {0u};

	out.x = (int)longitude;
	out.y = (int)latitude;

	return out;
}

