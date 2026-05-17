/*
 * main.c
 *
 *  Created on: Dec 29, 2025
 *      Author: mateusz
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <logger.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <svg.h>
#include <time.h>
#include <unistd.h>

#include "draw/aircraft.h"
#include "draw/airspace.h"
#include "draw/geography.h"
#include "draw/text.h"
#include "heap/heap_airspaces.h"
#include "srtm/srtm.h"
#include "types/aircraft_stv_t.h"
#include "types/airspace_t.h"
#include "types/airspace_t_xmacros.h"

#include "coordinates.h"
#include "main_application_config.h"

#ifdef WITH_OGN
#include "ogn.h"
#include "ogn_config.h"
#endif

// flipped by SIGINT/SIGTERM so the main loop can exit cleanly when the user
// hits Ctrl+C in the launching terminal (SDL only delivers SDL_QUIT for the
// window-close button, not for terminal signals).
static volatile sig_atomic_t shutdown_signalled = 0;

static void on_terminal_signal (int signum)
{
	(void)signum;
	shutdown_signalled = 1;
}

int main (int argc, char *argv[])
{
	const char *skrzyczne = "SKRZYCZNE\0";
	const char *zar = "ZAR\0";

	logger_initConsoleLogger (stdout);
	// INFO by default: previously DEBUG flooded stdout from per-frame call
	// sites (coordinates_move_origin, coordinates_scale_zoom_*, OGN drops),
	// turning the logger mutex + printf into a real CPU sink at 60 FPS.
	logger_setLevel (LogLevel_INFO);

	// catch terminal-side termination (Ctrl+C, `kill <pid>`) so we exit the
	// render loop and run the SDL/OGN teardown path instead of dying mid-frame.
	struct sigaction sa = {0};
	sa.sa_handler = on_terminal_signal;
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = 0; // no SA_RESTART: let blocking syscalls return EINTR so
					 // the OGN worker thread can also notice shutdown.
	sigaction (SIGINT, &sa, NULL);
	sigaction (SIGTERM, &sa, NULL);

	const size_t hardcoded_airspaces = heap_hardcoded_airspaces_count ();

	LOG_INFO ("amount of airspaces hardcoded: %d", hardcoded_airspaces);

	// print all info about all hardcoded airspces
	for (size_t i = 0; i < hardcoded_airspaces; i++) {
		// to currently processed airspace
		const airspace_t *const ptr = heap_hardcoded_airspaces[i];

		LOG_DEBUG ("number_of_vrt: %d", ptr->num_of_vertices);
	}

	svgDrawing *ptSvg = NULL;
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
										   SDL_WINDOW_OPENGL);

	// PRESENTVSYNC lets the GPU driver block SDL_RenderPresent on the display's
	// refresh, which is what actually caps CPU usage. Without it the render
	// thread spins flat-out and the SDL_Delay(16) below is the only governor.
	SDL_Renderer *renderer = SDL_CreateRenderer (window,
												 -1,
												 SDL_RENDERER_ACCELERATED |
													 SDL_RENDERER_PRESENTVSYNC);

	int w = 0, h = 0, ww = 0, hh = 0;

	SDL_GetRendererOutputSize (renderer, &w, &h);
	SDL_GetWindowSizeInPixels (window, &ww, &hh);

	srtm_load_files (" ");

#ifdef WITH_OGN
	// APRS-IS filter is centred on the middle of the *visible* viewport so the
	// aircraft we receive actually fall on-screen. The MAIN_VIEWPORT_DEFAULT_LOCATION
	// macro sits at the top-left of the SDL window (Y=0), so we offset south and
	// east by roughly half the viewport span to reach the centre of what the user
	// actually sees. Span estimate (with default scale): ~0.9° lat × ~1.2° lon.
	if (ogn_init (OGN_FIFO_PATH) == 0) {
		const float filter_lat = 49.904498f - 0.45f;
		const float filter_lon = 18.790042f + 0.60f;
		if (ogn_connect (OGN_DEFAULT_SERVER,
						 OGN_DEFAULT_PORT,
						 OGN_DEFAULT_RADIUS_KM,
						 filter_lat,
						 filter_lon) != 0) {
			LOG_WARN ("ogn_connect failed");
		}
	}
	else {
		LOG_WARN ("ogn_init failed - aircraft layer will be empty");
	}
#endif

	// Zoom feel: base rate is 10% of the current scale per second, ramping
	// linearly up to 3x peak over ZOOM_RAMP_S seconds of continuous hold.
	// The per-key hold timestamps are reset to 0 the instant the key is
	// released, so a quick tap stays at the base rate and only a sustained
	// hold spins up to the maximum.
	const double   ZOOM_BASE_PER_S = 0.10;
	const double   ZOOM_PEAK_X = 3.0;
	const double   ZOOM_RAMP_S = 1.5;
	uint32_t	   zoom_z_hold_start_ms = 0;
	uint32_t	   zoom_x_hold_start_ms = 0;
	uint32_t	   prev_frame_ms = 0;

	bool quit_requested = false;
	while (!quit_requested && !shutdown_signalled) {
		// drain every pending event each frame; if we only consumed one per
		// frame the OS-generated key autorepeat queue would back up and
		// keep firing even after the key has been released.
		SDL_Event e;
		while (SDL_PollEvent (&e)) {
			if (e.type == SDL_QUIT) {
				quit_requested = true;
			}
		}

		// keyboard-state polling rather than per-event handling gives us
		// predictable continuous motion while a key is held, independent of
		// the OS key-repeat delay and rate.
		const Uint8 *kstate = SDL_GetKeyboardState (NULL);

		// frame dt so the zoom rate is wall-clock based and stable across
		// framerates / vsync hiccups.
		const uint32_t now_ms = SDL_GetTicks ();
		const double dt_s = (prev_frame_ms == 0) ? 0.016
												 : (double)(now_ms - prev_frame_ms) / 1000.0;
		prev_frame_ms = now_ms;

		if (kstate[SDL_SCANCODE_Z]) {
			if (zoom_z_hold_start_ms == 0) {
				zoom_z_hold_start_ms = now_ms;
			}
			const double held_s = (double)(now_ms - zoom_z_hold_start_ms) / 1000.0;
			const double t = (held_s >= ZOOM_RAMP_S) ? 1.0 : (held_s / ZOOM_RAMP_S);
			const double factor = 1.0 + (ZOOM_PEAK_X - 1.0) * t;
			const double scale = coordinates_return_current_scale ();
			const double delta = ZOOM_BASE_PER_S * scale * factor * dt_s;
			coordinates_scale_zoom_out (delta);
		}
		else {
			zoom_z_hold_start_ms = 0;
		}
		if (kstate[SDL_SCANCODE_X]) {
			if (zoom_x_hold_start_ms == 0) {
				zoom_x_hold_start_ms = now_ms;
			}
			const double held_s = (double)(now_ms - zoom_x_hold_start_ms) / 1000.0;
			const double t = (held_s >= ZOOM_RAMP_S) ? 1.0 : (held_s / ZOOM_RAMP_S);
			const double factor = 1.0 + (ZOOM_PEAK_X - 1.0) * t;
			const double scale = coordinates_return_current_scale ();
			const double delta = ZOOM_BASE_PER_S * scale * factor * dt_s;
			coordinates_scale_zoom_in (delta);
		}
		else {
			zoom_x_hold_start_ms = 0;
		}
		if (kstate[SDL_SCANCODE_H]) {
			coordinates_move_origin (WEST, 0.002);
		}
		if (kstate[SDL_SCANCODE_J]) {
			coordinates_move_origin (EAST, 0.002);
		}
		if (kstate[SDL_SCANCODE_K]) {
			coordinates_move_origin (NORTH, 0.002);
		}
		if (kstate[SDL_SCANCODE_L]) {
			coordinates_move_origin (SOUTH, 0.002);
		}
		SDL_SetRenderDrawColor (renderer, 0, 0, 0, 0);
		SDL_RenderClear (renderer);

		geography_draw_longitude_lines (renderer, 0.15, LINE_STYLE_DOTTED);
		geography_draw_latitude_lines (renderer, 0.15, LINE_STYLE_DOTTED);

#ifdef WITH_OGN
		// drain any pending position frames from the OGN worker
		ogn_position_t pos;
		int drained = 0;
		while (ogn_read_next (&pos) > 0) {
			aircraft_table_upsert (&pos);
			drained++;
		}
		aircraft_table_prune ((uint64_t)time (NULL), OGN_STALE_SECONDS);
		static int ogn_log_throttle = 0;
		if (++ogn_log_throttle >= 200) {
			LOG_INFO ("ogn: table=%zu reports=%u timeouts=%u",
					  aircraft_table_count (),
					  ogn_get_position_reports_cntr (),
					  ogn_get_timeouts_cntr ());
			ogn_log_throttle = 0;
		}

		// paint each tracked aircraft from the OGN table
		for (size_t i = 0; i < aircraft_table_count (); i++) {
			const ogn_position_t *p = aircraft_table_get (i);
			aircraft_stv_t		  stv;
			stv.lat		 = (float)p->latitude_deg;
			stv.lon		 = (float)p->longitude_deg;
			stv.altitude = (unsigned)((p->altitude_feet < 0) ? 0 : p->altitude_feet);
			stv.bearing	 = (short)(p->ground_track_deg % 360);
			aircraft_draw_w_bearing_line_label (renderer, &stv, (char *)p->callsign);
		}
#endif

		geography_draw_mountain (renderer, 49.6855667f, 19.0312978f, skrzyczne, strlen (skrzyczne));
		geography_draw_mountain (renderer, 49.7872189f, 19.2248306f, zar, strlen (zar));

		for (size_t i = 0; i < heap_hardcoded_airspaces_count (); i++) {
			airspace_draw (renderer, heap_hardcoded_airspaces[i]);
		}
		SDL_RenderPresent (renderer);

		// With SDL_RENDERER_PRESENTVSYNC the call above already blocks on the
		// display refresh, so no extra SDL_Delay is needed -- adding one
		// stacks on top of vsync and roughly halves the framerate.
	}

#ifdef WITH_OGN
	ogn_deinit ();
#endif

	SDL_DestroyRenderer (renderer);
	SDL_DestroyWindow (window);

	// release SDL-owned resources held by drawing/SRTM layers before SDL_Quit tears
	// down the renderer/video subsystem.
	text_shutdown ();
	srtm_shutdown ();

	TTF_Quit ();
	SDL_Quit ();

	if (argc >= 2 && argv[1] != NULL) {
		ptSvg = svgOpenFile (argv[1]);
		if (ptSvg == NULL) {
			printf ("ERROR(%d): %s.\n", svgGetLastError (), svgGetLastErrorDescription ());
		}
	}

	return 0;
}
