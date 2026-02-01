/*
 * main.c
 *
 *  Created on: Dec 29, 2025
 *      Author: mateusz
 */

#include "svg.h"
#include <SDL2/SDL.h>

#include <stdio.h>
#include <unistd.h>

#include "types/aircraft_stv_t.h"
#include "draw/aircraft.h"

int main (int argc, char *argv[])
{
	(void) argc;

	svgDrawing *ptSvg;
	const int initres = SDL_Init (SDL_INIT_VIDEO);

	if (initres < 0) {
		const char *errormsg = SDL_GetError ();
		printf ("error: %s", errormsg);
		return -1;
	}

	SDL_Window *window = SDL_CreateWindow ("TERMINATOR12",
										   SDL_WINDOWPOS_UNDEFINED,
										   SDL_WINDOWPOS_UNDEFINED,
										   1024,
										   768,
										   SDL_WINDOW_SHOWN);

	SDL_Renderer *renderer = SDL_CreateRenderer (window, -1, SDL_RENDERER_ACCELERATED);

	aircraft_stv_t stv = {0U};

	while (1) {
		SDL_Event e;
		if (SDL_WaitEventTimeout (&e, 10)) {
			if (e.type == SDL_QUIT) {
				break;
			}
		}
		SDL_SetRenderDrawColor (renderer, 0, 0, 0, 0);
		SDL_RenderClear (renderer);
		aircraft_draw_with_bearing_line (renderer, &stv);
		//		SDL_SetRenderDrawColor (renderer, 255, 255, 255, 0);
		//		SDL_RenderDrawRect (renderer, &rectangle);
		//		const line_coordinates_t line = main_get_bearing_line(&rectangle, (++bearing) %
		//360); 		SDL_RenderDrawLine (renderer, line.x1, line.y1, line.x2, line.y2);
		SDL_RenderPresent (renderer);
	}

	SDL_DestroyRenderer (renderer);
	SDL_DestroyWindow (window);

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
