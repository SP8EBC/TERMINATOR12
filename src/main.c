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

#define LINE_LN		(25.0)

#define BEARING_TO_SDLANGLE(b)			((b + 270) % 360)

#define RECT_MIDDLE_UPPER_BODER(for_this) 	{.x = for_this->x + for_this->w / 2, .y = for_this->y};
#define RECT_TOP_RIGHT(for_this)		  	{.x = for_this->x + for_this->w, .y = for_this->y};
#define RECT_MIDDLE_RIGHT_BORDER(for_this)	{.x = for_this->x + for_this->w, .y = for_this->y + for_this->h / 2};
#define RECT_BOTTOM_RIGHT(for_this) 		{.x = for_this->x + for_this->w, .y = for_this->y};
#define RECT_MIDDLE_BOTTOM_BORDER(for_this) {.x = for_this->x + for_this->w / 2, .y = for_this->y + for_this->h};
#define RECT_BOTTOM_LEFT(for_this)			{.x = for_this->x, .y = for_this->y + for_this->h};
#define RECT_MIDDLE_LEFT_BORDER(for_this)	{.x = for_this->x, .y = for_this->y + for_this->h / 2};
#define RECT_TOP_LEFT(for_this)				{.x = for_this->x, .y = for_this->y};

#define SET_LINE_STARTP(line, point)		\
		line.x1 = point.x;					\
		line.y1 = point.y;					\

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
	line_coordinates_t out = {0u};

	if (bearing == 0) {
		// draw from the middle of upper border
		const SDL_Point point = RECT_MIDDLE_UPPER_BODER(for_this);
		SET_LINE_STARTP(out, point);
	}
	else if (bearing > 0 && bearing < 90) {
		// draw from the top right corner
		const SDL_Point point = RECT_TOP_RIGHT(for_this);
		SET_LINE_STARTP(out, point);
	}
	else if (bearing == 90) {
		// draw from the middle of right border
		const SDL_Point point = RECT_MIDDLE_RIGHT_BORDER(for_this);
		SET_LINE_STARTP(out, point);
	}
	else if (bearing > 90 && bearing < 180) {
		// draw from the bottom right corner
		const SDL_Point point = RECT_BOTTOM_RIGHT(for_this);
		SET_LINE_STARTP(out, point);
	}
	else if (bearing == 180) {
		// draw from the middle of bottom border
		const SDL_Point point = RECT_MIDDLE_BOTTOM_BORDER(for_this);
		SET_LINE_STARTP(out, point);
	}
	else if (bearing > 180 && bearing < 270) {
		// draw from left bottom corner
		const SDL_Point point = RECT_BOTTOM_LEFT(for_this);
		SET_LINE_STARTP(out, point);
	}
	else if (bearing == 270) {
		// draw from the middle of left border
		const SDL_Point point = RECT_MIDDLE_LEFT_BORDER(for_this);
		SET_LINE_STARTP(out, point);
	}
	else if (bearing > 270 && bearing < 360) {
		// draw from the top left corner
		const SDL_Point point = RECT_TOP_LEFT(for_this);
		SET_LINE_STARTP(out, point);
	}
	else {
		// ???
	}


	const int sdlangle = BEARING_TO_SDLANGLE(bearing);

	// calculate end point basing on angle

	int help_x = round(cos((double)sdlangle * (M_PI / 180.0)) * LINE_LN);
	int help_y = round(sin((double)sdlangle * (M_PI / 180.0)) * LINE_LN);

	out.x2 = out.x1 + help_x;
	out.y2 = out.y1 + help_y;

	return out;
}

int main (int argc, char *argv[])
{
	svgDrawing *ptSvg;
	const int initres = SDL_Init (SDL_INIT_VIDEO);

	if (initres < 0) {
		const char *errormsg = SDL_GetError ();
		printf ("error: %s", errormsg);
		return -1;
	}

	// point 0,0 is in top left corner. Width goes along x-axis towards right. height goes along
	// y-axis towards down
	SDL_Rect rectangle = {.x = 40, .y = 40, .w = 15, .h = 15};

	SDL_Window *window = SDL_CreateWindow ("TERMINATOR12",
										   SDL_WINDOWPOS_UNDEFINED,
										   SDL_WINDOWPOS_UNDEFINED,
										   1024,
										   768,
										   SDL_WINDOW_SHOWN);

	SDL_Renderer *renderer = SDL_CreateRenderer (window, -1, SDL_RENDERER_ACCELERATED);

	short bearing = 0;

	while (1) {
		SDL_Event e;
		if (SDL_WaitEventTimeout(&e, 10)) {
			if (e.type == SDL_QUIT) {
				break;
			}
		}
		SDL_SetRenderDrawColor (renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor (renderer, 255, 255, 255, 0);
		SDL_RenderDrawRect (renderer, &rectangle);
		const line_coordinates_t line = main_get_bearing_line(&rectangle, (++bearing) % 360);
		SDL_RenderDrawLine (renderer, line.x1, line.y1, line.x2, line.y2);
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
