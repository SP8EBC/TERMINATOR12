/*
 * geography.c
 *
 *  Created on: Feb 12, 2026
 *      Author: mateusz
 */

#include "geography.h"
#include "coordinates.h"
#include "text.h"

#include <draw_aircraft_config.h>
#include <draw_geography_config.h>

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL2_gfxPrimitives_font.h>
/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

#define SQRT3		 (1.732050808F)
#define TR_HEIGHT(a) (0.5F * (SQRT3 * a))

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

static void geography_xpoints_to_vect (const SDL_Point *const up, const SDL_Point *const left,
									   const SDL_Point *const right, int16_t *out)
{
	assert (up != NULL);
	assert (left != NULL);
	assert (right != NULL);
	assert (out != NULL);

	out[0] = (int16_t)up->x;
	out[1] = (int16_t)left->x;
	out[2] = (int16_t)right->x;
}

static void geography_ypoints_to_vect (const SDL_Point *const up, const SDL_Point *const left,
									   const SDL_Point *const right, int16_t *out)
{
	assert (up != NULL);
	assert (left != NULL);
	assert (right != NULL);
	assert (out != NULL);

	out[0] = (int16_t)up->y;
	out[1] = (int16_t)left->y;
	out[2] = (int16_t)right->y;
}

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

void geography_draw_mountain (SDL_Renderer *renderer, float lat, float lon, const char *label,
							  size_t label_ln)
{
	(void)label;
	(void)label_ln;
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
	*/

	const float half_a = CONFIG_DRAW_GEOGRAPHY_MOUNTIN_SIZ * 0.5F;
	const float h = TR_HEIGHT (CONFIG_DRAW_GEOGRAPHY_MOUNTIN_SIZ);

	const SDL_Point upper_point = coordinates_get_point_from_latlon (lon, lat);

	// for simplicity, let's assume that the top of a triangle is located exactly on the
	const SDL_Point left = {.x = (int)(round ((float)upper_point.x - half_a)),
							.y = (int)(round ((float)upper_point.y + h))};
	const SDL_Point right = {.x = (int)(round ((float)upper_point.x + half_a)),
							 .y = (int)(round ((float)upper_point.y + h))};

	int16_t vx[3];
	int16_t vy[3];

	geography_xpoints_to_vect (&upper_point, &left, &right, vx);
	geography_ypoints_to_vect (&upper_point, &left, &right, vy);

	polygonColor (renderer, vx, vy, 3, CONFIG_DRAW_GEOGRAPHY_MOUNTIN_COLOR);

	if ((label != NULL) && (label_ln > 1)) {
		if (label_ln > 8) {
			label_ln = 8;
		}
		// void text_draw (SDL_Renderer *renderer, const int font_size, const char *text, int x,
		// int y);
		// text_ndraw(renderer, CONFIG_DRAW_AIRCRAFT_LABEL_FONT_SIZE, label, label_ln, left.x +
		// 5, left.y - 15);
		text_ndraw (renderer,
					CONFIG_DRAW_AIRCRAFT_LABEL_FONT_SIZE,
					label,
					label_ln,
					left.x - (int)(label_ln * 2),
					left.y + (CONFIG_DRAW_AIRCRAFT_LABEL_FONT_SIZE / 5));
	}
}
