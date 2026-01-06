/*
 * main.c
 *
 *  Created on: Dec 29, 2025
 *      Author: mateusz
 */

#include <SDL2/SDL.h>

#include <stdio.h>

typedef struct line_coordinates_t {
	int x1; // begin
	int y1;
	int x2; // end
	int y2;
} line_coordinates_t;

/**
 *
 * @param for_this A rectangle, with the origin at the upper left (integer).
 * @param bearing in range from 0 to 359 degrees
 * @return
 */
line_coordinates_t main_get_bearing_line (SDL_Rect *for_this, short bearing)
{
	line_coordinates_t out;

	if (bearing == 0)
	{
		// draw from the middle of upper border
	}
	if (bearing > 0 && bearing < 90)
	{
		// draw from the top right corner
	}

	//out.x1 =

	return out;
}

int main (int argc, char *argv[])
{

	const int initres = SDL_Init (SDL_INIT_VIDEO);

	if (initres < 0) {
		const char *errormsg = SDL_GetError ();
		printf ("error: %s", errormsg);
		return -1;
	}

	SDL_Rect rectangle = {.x = 0, .y = 0, .w = 5, .h = 5};

	SDL_Window *window = SDL_CreateWindow ("ParaATC",
										   SDL_WINDOWPOS_UNDEFINED,
										   SDL_WINDOWPOS_UNDEFINED,
										   1024,
										   768,
										   SDL_WINDOW_SHOWN);

	SDL_Renderer *renderer = SDL_CreateRenderer (window, -1, SDL_RENDERER_ACCELERATED);

	while (1) {
		SDL_Event e;
		if (SDL_WaitEvent (&e)) {
			if (e.type == SDL_QUIT) {
				break;
			}
		}
		SDL_SetRenderDrawColor (renderer, 255, 255, 255, 0);
		// SDL_RenderDrawPoint( renderer, 100, 100 );
		SDL_RenderDrawLine (renderer, 10, 10, 50, 50);
		SDL_RenderDrawRect (renderer, &rectangle);
		SDL_RenderPresent (renderer);
	}

	SDL_DestroyRenderer (renderer);
	SDL_DestroyWindow (window);

	SDL_Quit ();

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
