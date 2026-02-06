/*
 * aircraft.c
 *
 *	Paint an aicraft position on the map
 *
 *  Created on: Feb 1, 2026
 *      Author: mateusz
 */

#include "draw/aircraft.h"
#include "draw/text.h"
#include "types/line_coordinates_t.h"

#include "coordinates.h"

#include "draw_aircraft_config.h"

#include <math.h>

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

#define BEARING_TO_SDLANGLE(b) ((b + 270) % 360)

// clang-format off
#define RECT_MIDDLE_UPPER_BODER(for_this) 	{.x = for_this->x + for_this->w / 2, .y = for_this->y};
#define RECT_TOP_RIGHT(for_this)		  	{.x = for_this->x + for_this->w, .y = for_this->y};
#define RECT_MIDDLE_RIGHT_BORDER(for_this)	{.x = for_this->x + for_this->w, .y = for_this->y + for_this->h / 2};
#define RECT_BOTTOM_RIGHT(for_this) 		{.x = for_this->x + for_this->w, .y = for_this->y + for_this->h};
#define RECT_MIDDLE_BOTTOM_BORDER(for_this) {.x = for_this->x + for_this->w / 2, .y = for_this->y + for_this->h};
#define RECT_BOTTOM_LEFT(for_this)			{.x = for_this->x, .y = for_this->y + for_this->h};
#define RECT_MIDDLE_LEFT_BORDER(for_this)	{.x = for_this->x, .y = for_this->y + for_this->h / 2};
#define RECT_TOP_LEFT(for_this)				{.x = for_this->x, .y = for_this->y};
// clang-format on

#define GET_X_FOR_NEWLINE(x, fontsize) (x)
#define GET_Y_FOR_NEWLINE(y, fontsize) (y + fontsize + 2)

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/**
 *
 * @param for_this A rectangle, with the origin at the upper left (integer).
 * @param bearing in range from 0 to 359 degrees
 * @return
 */
static line_coordinates_t aircraft_get_bearing_line (SDL_Rect *for_this, short bearing)
{
	line_coordinates_t out = {0u};

	if (bearing == 0) {
		// draw from the middle of upper border
		const SDL_Point point = RECT_MIDDLE_UPPER_BODER (for_this);
		SET_LINE_STARTP (out, point);
	}
	else if (bearing > 0 && bearing < 90) {
		// draw from the top right corner
		const SDL_Point point = RECT_TOP_RIGHT (for_this);
		SET_LINE_STARTP (out, point);
	}
	else if (bearing == 90) {
		// draw from the middle of right border
		const SDL_Point point = RECT_MIDDLE_RIGHT_BORDER (for_this);
		SET_LINE_STARTP (out, point);
	}
	else if (bearing > 90 && bearing < 180) {
		// draw from the bottom right corner
		const SDL_Point point = RECT_BOTTOM_RIGHT (for_this);
		SET_LINE_STARTP (out, point);
	}
	else if (bearing == 180) {
		// draw from the middle of bottom border
		const SDL_Point point = RECT_MIDDLE_BOTTOM_BORDER (for_this);
		SET_LINE_STARTP (out, point);
	}
	else if (bearing > 180 && bearing < 270) {
		// draw from left bottom corner
		const SDL_Point point = RECT_BOTTOM_LEFT (for_this);
		SET_LINE_STARTP (out, point);
	}
	else if (bearing == 270) {
		// draw from the middle of left border
		const SDL_Point point = RECT_MIDDLE_LEFT_BORDER (for_this);
		SET_LINE_STARTP (out, point);
	}
	else if (bearing > 270 && bearing < 360) {
		// draw from the top left corner
		const SDL_Point point = RECT_TOP_LEFT (for_this);
		SET_LINE_STARTP (out, point);
	}
	else {
		// ???
	}

	const int sdlangle = BEARING_TO_SDLANGLE (bearing);

	// calculate end point basing on angle

	int help_x =
		(int)round (cos ((double)sdlangle * (M_PI / 180.0)) * CONFIG_DRAW_AIRCRAFT_BEARING_LINE_LN);
	int help_y =
		(int)round (sin ((double)sdlangle * (M_PI / 180.0)) * CONFIG_DRAW_AIRCRAFT_BEARING_LINE_LN);

	out.x2 = out.x1 + help_x;
	out.y2 = out.y1 + help_y;

	return out;
}

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 *
 * @param renderer
 * @param aircraft
 */
SDL_Rect aircraft_draw_w_bearing_line (SDL_Renderer *renderer, aircraft_stv_t *aircraft)
{
	const SDL_Point point = coordinates_get_point_from_latlon (aircraft->lon, aircraft->lat);

	// point x=0,y=0 is in top left corner. Width goes along x-axis towards right. height goes along
	// y-axis towards down
	SDL_Rect rectangle = {.x = point.x,
						  .y = point.y,
						  .w = CONFIG_DRAW_AIRCRAFT_SQUARE_SIZ,
						  .h = CONFIG_DRAW_AIRCRAFT_SQUARE_SIZ};

	SDL_SetRenderDrawColor (renderer, 255, 255, 255, 0);
	SDL_RenderDrawRect (renderer, &rectangle);
	const line_coordinates_t line = aircraft_get_bearing_line (&rectangle, aircraft->bearing % 360);
	SDL_RenderDrawLine (renderer, line.x1, line.y1, line.x2, line.y2);

	return rectangle;
}

void aircraft_draw_w_bearing_line_label (SDL_Renderer *renderer, aircraft_stv_t *aircraft,
										 char *callsign)
{
	(void)callsign;

	// draw rectangle icon w/ bearing line
	SDL_Rect rectangle = aircraft_draw_w_bearing_line (renderer, aircraft);

	// where the label will be printed
	SDL_Point label;

	// draw on the right from the aircraft icon
	if (aircraft->bearing < 80 || aircraft->bearing > 160) {
		label.x = rectangle.x + CONFIG_DRAW_AIRCRAFT_SQUARE_SIZ + 3;
		label.y = rectangle.y; // test +3
	}
	else {
		// draw below the aircraft icon
		label.x = rectangle.x + CONFIG_DRAW_AIRCRAFT_SQUARE_SIZ +
				  (int)CONFIG_DRAW_AIRCRAFT_BEARING_LINE_LN + 3;
		label.y = rectangle.y; // test +3
	}

	// draw callsign
	text_draw (renderer, CONFIG_DRAW_AIRCRAFT_LABEL_FONT_SIZE, callsign, label.x, label.y);

	// draw altitude
	text_draw_altitude_or_fl (renderer,
							  CONFIG_DRAW_AIRCRAFT_LABEL_FONT_SIZE,
							  GET_X_FOR_NEWLINE (label.x, CONFIG_DRAW_AIRCRAFT_LABEL_FONT_SIZE),
							  GET_Y_FOR_NEWLINE (label.y, CONFIG_DRAW_AIRCRAFT_LABEL_FONT_SIZE),
							  aircraft->altitude,
							  false);
}
