/*
 * aircraft.h
 *
 *  Created on: Feb 1, 2026
 *      Author: mateusz
 */

#ifndef SRC_DRAW_AIRCRAFT_H_
#define SRC_DRAW_AIRCRAFT_H_

#include <SDL2/SDL.h>
#include <stddef.h>
#include <stdint.h>

#include "types/aircraft_stv_t.h"

#ifdef WITH_OGN
#include "types/ogn_position_t.h"
#endif

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 *
 * @param aircraft state vector, altitude, coordinates etc.
 * @param aircraft
 * @return rectangle at display coordinates representing
 */
SDL_Rect aircraft_draw_w_bearing_line (SDL_Renderer *renderer, aircraft_stv_t *aircraft);

/**
 *
 * @param renderer
 * @param aircraft state vector, altitude, coordinates etc.
 * @param callsign which must be 7 characters and space padded if a string is shorter.
 */
void aircraft_draw_w_bearing_line_label (SDL_Renderer *renderer, aircraft_stv_t *aircraft,
										 char *callsign);

#ifdef WITH_OGN
/**
 * @brief Inserts or refreshes a slot keyed by callsign. Oldest by rx_epoch_s is evicted
 * if the table is full.
 */
void aircraft_table_upsert (const ogn_position_t *pos);

/**
 * @brief Drops slots older than @c stale_seconds compared to @c now_epoch_s.
 */
void aircraft_table_prune (uint64_t now_epoch_s, uint32_t stale_seconds);

/**
 * @brief How many live slots the table holds right now.
 */
size_t aircraft_table_count (void);

/**
 * @brief Random-access getter (i < count). Returns NULL on out-of-range.
 * Output: a stv suitable for @link{aircraft_draw_w_bearing_line_label} plus the callsign.
 */
const ogn_position_t *aircraft_table_get (size_t i);
#endif

#endif /* SRC_DRAW_AIRCRAFT_H_ */
