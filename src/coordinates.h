/*
 * coordinates.h
 *
 *  Created on: Feb 2, 2026
 *      Author: mateusz
 */

#ifndef SRC_COORDINATES_H_
#define SRC_COORDINATES_H_

#include <SDL2/SDL.h>
#include <stdbool.h>

#include "./types/coordinates_t.h"

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
 * @brief Calculates the great-circle distance between two GPS coordinates using the Haversine
 * formula.
 *
 * This function computes the shortest distance over the Earth's surface between two points
 * specified by their latitude and longitude coordinates. It uses the Haversine formula,
 * which provides high accuracy for distances up to several thousand kilometers.
 *
 * @param lat1 Latitude of the first point in degrees (positive for North, negative for South).
 * @param lon1 Longitude of the first point in degrees (positive for East, negative for West).
 * @param lat2 Latitude of the second point in degrees (positive for North, negative for South).
 * @param lon2 Longitude of the second point in degrees (positive for East, negative for West).
 *
 * @return Distance between the two points in meters.
 *
 * @note Assumes a spherical Earth model with radius defined by COORDINATES_EARTH_RADIUS.
 * @note The function expects coordinates within valid GPS ranges:
 *       - Latitude: [-90.0, 90.0]
 *       - Longitude: [-180.0, 180.0]
 * @note For antipodal points (exactly opposite sides of Earth), results may have minor precision
 * issues.
 *
 * @see deg2rad() - Required helper function for degree-to-radian conversion.
 * @see COORDINATES_EARTH_RADIUS - Global constant defining Earth's radius in meters.
 */
double coordinates_wsg84_distance (double lat1, double lon1, double lat2, double lon2);

/**
 * @brief Computes destination point from:
 *  @param[in] lat1_deg   start latitude in degrees
 *  @param[in] lon1_deg   start longitude in degrees
 *  @param[in] bearing_deg initial bearing in degrees clockwise from true north
 *  @param[in] distance_m distance in meters
 *   @param[out] lat2_deg  destination latitude in degrees
 *   @param[out] lon2_deg  destination longitude in degrees
 *
 * Returns:
 *   true on success
 */
bool coordinates_wgs84_destination_point (double lat1_deg, double lon1_deg, double bearing_deg,
										  double distance_m, double *lat2_deg, double *lon2_deg);

/**
 * Converts longitude and latitude to the xy coordinates on the screen
 * @param longitude
 * @param latitude
 * @return a copy of SDL_Point structure set to represenging xy screen coordinates
 */
SDL_Point coordinates_get_point_from_lonlat (double longitude, double latitude);

void coordinates_scale_zoom_in (double by_this);

void coordinates_scale_zoom_out (double by_this);

void coordinates_output_scale_zoom_in (double by_this);

void coordinates_output_scale_zoom_out (double by_this);

coordinates_t coordinates_return_current_viewport (void);

#endif /* SRC_COORDINATES_H_ */
