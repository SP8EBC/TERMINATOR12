/*
 * airspace.c
 *
 *  Created on: Feb 12, 2026
 *      Author: mateusz
 */

#include "airspace.h"
#include "coordinates.h"

#include <SDL2/SDL2_gfxPrimitives.h>

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================
#define TEST_SIZE 5

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
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

void airspace_draw(SDL_Renderer *renderer, const airspace_t* const to_draw)
{

	/*!
	\brief Draw polygon with alpha blending.

	\param renderer The renderer to draw on.
	\param vx Vertex array containing X coordinates of the points of the polygon.
	\param vy Vertex array containing Y coordinates of the points of the polygon.
	\param n Number of points in the vertex array. Minimum number is 3.
	\param color The color value of the polygon to draw (0xRRGGBBAA).

	\returns Returns 0 on success, -1 on failure.


	int GFX_polygonColor(SDL_Renderer * renderer, const Sint16 * vx, const Sint16 * vy, int n,
	Uint32 color)


		const Sint16 testx[TEST_SIZE] = {600, 100, 800, 700, 600};
	const Sint16 testy[TEST_SIZE] = {200, 400, 400, 100, 200};
	*/

	Sint16 basex[to_draw->num_of_vertices];
	Sint16 basey[to_draw->num_of_vertices];

	memset(basex, 0x00, to_draw->num_of_vertices);
	memset(basey, 0x00, to_draw->num_of_vertices);

	for (size_t i = 0; i < to_draw->num_of_vertices; i++)
	{
		coordinates_t* vtx = &to_draw->vertices[i];
		SDL_Point point = coordinates_get_point_from_lonlat(vtx->longitude, vtx->latitude);
		basex[i] = (Sint16)point.x;
		basey[i] = (Sint16)point.y;
	}


	polygonColor (renderer, basex, basey, (int)to_draw->num_of_vertices, 0xFFFFFF40);
}
