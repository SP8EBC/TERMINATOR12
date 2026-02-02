/*
 * coordinates.c
 *
 *  Created on: Feb 2, 2026
 *      Author: mateusz
 */

#include "coordinates.h"

SDL_Point coordinates_get_point_from_latlon(float longitude, float latitude)
{
	SDL_Point out = {0u};

	out.x = (int)longitude;
	out.y = (int)latitude;

	return out;
}

