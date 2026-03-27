/*
 * airspace.c
 *
 *  Created on: Feb 12, 2026
 *      Author: mateusz
 */

#include "airspace.h"
#include <SDL2/SDL2_gfxPrimitives.h>

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================
#define TEST_SIZE	5

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

void airspace_test(SDL_Renderer *renderer, short i)
{

	/*!
	\brief Draw polygon with alpha blending.

	\param renderer The renderer to draw on.
	\param vx Vertex array containing X coordinates of the points of the polygon.
	\param vy Vertex array containing Y coordinates of the points of the polygon.
	\param n Number of points in the vertex array. Minimum number is 3.
	\param color The color value of the polygon to draw (0xRRGGBBAA).

	\returns Returns 0 on success, -1 on failure.


	int GFX_polygonColor(SDL_Renderer * renderer, const Sint16 * vx, const Sint16 * vy, int n, Uint32 color)


		const Sint16 testx[TEST_SIZE] = {600, 100, 800, 700, 600};
	const Sint16 testy[TEST_SIZE] = {200, 400, 400, 100, 200};
	*/



	const Sint16 basex[TEST_SIZE] = {900 + i / 3, 400 + i / 3, 1100 + i / 3, 1000 + i / 3, 900 + i / 3};
	const Sint16 basey[TEST_SIZE] = {300 + i, 500 + i, 500 + i, 200 + i, 300 + i};

	polygonColor(renderer, basex, basey, TEST_SIZE, 0xFFFFFF40);
}
