/*
 * main.c
 *
 *  Created on: Dec 29, 2025
 *      Author: mateusz
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <svg.h>
#include <unistd.h>

#include "draw/aircraft.h"
#include "draw/airspace.h"
#include "draw/geography.h"
#include "draw/text.h"
#include "types/aircraft_stv_t.h"

#include "coordinates.h"
#include "main_application_config.h"

int main (int argc, char *argv[])
{
	(void)argc;

	const char *skrzyczne = "SKRZYCZNE\0";
	const char *zar = "ZAR\0";

	svgDrawing *ptSvg;
	const int initres = SDL_Init (SDL_INIT_VIDEO);

	if (initres < 0) {
		const char *errormsg = SDL_GetError ();
		printf ("error: %s", errormsg);
		return -1;
	}

	TTF_Init ();

	SDL_Window *window = SDL_CreateWindow ("TERMINATOR12",
										   SDL_WINDOWPOS_UNDEFINED,
										   SDL_WINDOWPOS_UNDEFINED,
										   MAIN_WIDTH,
										   MAIN_HEIGHT,
										   SDL_WINDOW_SHOWN);

	SDL_Renderer *renderer = SDL_CreateRenderer (window, -1, SDL_RENDERER_ACCELERATED);

	aircraft_stv_t stv = {0U};
	stv.lat = 49.7194486f;
	stv.lon = 19.1048506f;
	stv.altitude = 3500;

	// #define MAIN_VIEWPORT_DEFAULT_LOCATION(ENTRY) ENTRY (49.7481156, 18.9444758)

	while (1) {
		SDL_Event e;
		if (SDL_WaitEventTimeout (&e, 10)) {
			if (e.type == SDL_QUIT) {
				break;
			}
			else if (e.type == SDL_KEYDOWN) {
				if (e.key.keysym.sym == SDLK_z) {
					coordinates_scale_zoom_out (0.1);
				}
				else if (e.key.keysym.sym == SDLK_x) {
					coordinates_scale_zoom_in (0.1);
				}
				else if (e.key.keysym.sym == SDLK_a) {
					coordinates_output_scale_zoom_out (10);
				}
				else if (e.key.keysym.sym == SDLK_s) {
					coordinates_output_scale_zoom_in (10);
				}
				else {
					;
				}
			}
		}
		geography_draw_longitude_lines(renderer, 0.015, LINE_STYLE_DOTTED);

		SDL_SetRenderDrawColor (renderer, 0, 0, 0, 0);
		SDL_RenderClear (renderer);
		stv.bearing++;
		if (stv.bearing > 359) {
			stv.bearing = 0;
		}
		aircraft_draw_w_bearing_line_label (renderer, &stv, "SPSWWW");
		geography_draw_mountain (renderer, 49.6855667f, 19.0312978f, skrzyczne, strlen (skrzyczne));
		geography_draw_mountain (renderer, 48.7872189f, 19.2248306f, zar, strlen (zar));
		// aircraft_draw_w_bearing_line (renderer, &stv);
		//		SDL_SetRenderDrawColor (renderer, 255, 255, 255, 0);
		//		SDL_RenderDrawRect (renderer, &rectangle);
		//		const line_coordinates_t line = main_get_bearing_line(&rectangle, (++bearing) %
		// 360); 		SDL_RenderDrawLine (renderer, line.x1, line.y1, line.x2, line.y2);
		// airspace_test (renderer, stv.bearing);
		SDL_RenderPresent (renderer);
	}

	SDL_DestroyRenderer (renderer);
	SDL_DestroyWindow (window);

	TTF_Quit ();
	SDL_Quit ();

	ptSvg = svgOpenFile (argv[1]);
	if (ptSvg == NULL) {
		printf ("ERROR(%d): %s.\n", svgGetLastError (), svgGetLastErrorDescription ());
	}

	return 0;
}

/**
 * https://www.studyplan.dev/sdl2/building-sdl/q/sdl2-renderer-vs-surface
 *
 * SDL_Surface

An SDL_Surface is a pixel buffer that represents an image in memory. It contains the pixel data and
format information. You can manipulate the pixels directly and perform software-based rendering
using an SDL_Surface.

SDL_Renderer

An SDL_Renderer is a rendering context that provides hardware-accelerated 2D rendering. It abstracts
the rendering process and allows you to draw primitives, textures, and perform transformations
efficiently.
 */

/*!
\brief Draw polygon with alpha blending.

\param renderer The renderer to draw on.
\param vx Vertex array containing X coordinates of the points of the polygon.
\param vy Vertex array containing Y coordinates of the points of the polygon.
\param n Number of points in the vertex array. Minimum number is 3.
\param color The color value of the polygon to draw (0xRRGGBBAA).

\returns Returns 0 on success, -1 on failure.


int GFX_polygonColor(SDL_Renderer * renderer, const Sint16 * vx, const Sint16 * vy, int n, Uint32
color)
*/
