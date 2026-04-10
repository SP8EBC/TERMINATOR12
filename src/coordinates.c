/*
 * coordinates.c
 *
 *  Created on: Feb 2, 2026
 *      Author: mateusz
 */

#include "coordinates.h"

#include "types/coordinates_t.h"

#include "main_application_config.h"

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

#define COORDINATES_GET_VIEWPORT_X(x, y) (x)

#define COORDINATES_GET_VIEWPORT_Y(x, y) (y)

#define COORDINATES_EARTH_RADIUS 6371000.0

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

static const coordinates_t coordinates_viewport_limit = {
	.latitude = MAIN_VIEWPORT_LIMIT (COORDINATES_GET_VIEWPORT_X),
	.longitude = MAIN_VIEWPORT_LIMIT (COORDINATES_GET_VIEWPORT_Y)};

/**
 * @brief these are coordinates of a point in top left corner
 */
static coordinates_t coordinates_viewport_current = {
		.latitude = MAIN_VIEWPORT_DEFAULT_LOCATION (COORDINATES_GET_VIEWPORT_X),
		.longitude = MAIN_VIEWPORT_DEFAULT_LOCATION (COORDINATES_GET_VIEWPORT_Y)};

/**
 * @brief how many decimal degrees of longitude corresponds to one pixel on the screen, basically
 * the zoom level
 * @note One minute of latitude corresponds very nearly to 1 nautical mile, which is about 1.852 km,
 * and this is almost constant everywhere on Earth
 * @warning Longitude‑minute distance depends on latitude, because meridians converge toward the
 * poles. Poland sits roughly between about 49°N and 55°N, so a reasonable average central latitude
 * is around 52°N. Using the approximation: 1′ longitude≈1.852⋅cos⁡(φ) km, where φ is
 * latitude in degrees, at 52°N we have: 1′ longitude ≈ 1.852×0.615≈1.141.852×0.615≈1.14 km.
 */
static double coordinates_degrees_longitude_per_pixel;

static double coordinates_scale = 1.0;

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

double deg2rad (double deg)
{
	return deg * M_PI / 180.0;
}

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
double gps_distance (double lat1, double lon1, double lat2, double lon2)
{
	double dlat = deg2rad (lat2 - lat1);
	double dlon = deg2rad (lon2 - lon1);

	lat1 = deg2rad (lat1);
	lat2 = deg2rad (lat2);

	double a =
		sin (dlat / 2) * sin (dlat / 2) + cos (lat1) * cos (lat2) * sin (dlon / 2) * sin (dlon / 2);

	double c = 2.0 * atan2 (sqrt (a), sqrt (1.0 - a));

	return COORDINATES_EARTH_RADIUS * c;
}

/**
 * @brief Converts WSG84-GPS coordinates to mercator projection coordinates, used for displaying
 * @note Clamping latitude is important because the Mercator projection tends to infinity at the
 * poles, and practical Web Mercator usage limits latitude to about 85.0511 degrees.
 * @param scale
 * @param longitude_deg in decimal degrees (will be converted to radians inside)
 * @param latitude_deg in decimal degrees (will be converted to radians inside)
 * @return
 */
SDL_Point mercator_project (double scale, double longitude_deg, double latitude_deg)
{
	const double DEG_TO_RAD = M_PI / 180.0;
	const double MAX_LAT = 85.05112878; // practical Mercator limit

	SDL_Point p;

	if (latitude_deg > MAX_LAT)
		latitude_deg = MAX_LAT;
	if (latitude_deg < -MAX_LAT)
		latitude_deg = -MAX_LAT;

	double lon = longitude_deg * DEG_TO_RAD;
	double lat = latitude_deg * DEG_TO_RAD;

	p.x = scale * lon;
	p.y = scale * log (tan (M_PI / 4.0 + lat / 2.0));

	return p;
}


/**
 * @brief Convert Mercator X, Y coordinates back to longitude and latitude.
 *
 * Converts Mercator-projected Cartesian coordinates (x, y) back to geographic
 * longitude and latitude in degrees, using the same scale factor that was used
 * for the forward projection.
 *
 * @note This function is the exact inverse of the Mercator projection:
 *       x = scale * longitude_rad, y = scale * log(tan(pi/4 + latitude_rad/2)).
 *       For practical use (e.g., Web Mercator), consider clamping latitude
 *       output to approximately ±85.0511°.
 *
 * @param[in]  scale           Scale factor used in the original Mercator projection.
 *                             Must be positive and non‑zero.
 * @param[in]  x               Mercator X coordinate (in the same units as `scale`).
 * @param[in]  y               Mercator Y coordinate (in the same units as `scale`).
 *
 * @return coordinates_t structure containing:
 *         - longitude: longitude in degrees (-180.0 to 180.0).
 *         - latitude:  latitude in degrees (unbounded in theory,
 *                          but typically clamped to about ±85.0511° for Web Mercator).
 *
 * @see mercator_project()  Forward projection from longitude/latitude to Mercator (x, y).
 */
coordinates_t mercator_inverse(double scale, double x, double y)
{
    const double RAD_TO_DEG = 180.0 / M_PI;

    coordinates_t ll;

    double lon = x / scale;
    double lat = 2.0 * atan(exp(y / scale)) - M_PI / 2.0;

    ll.longitude = lon * RAD_TO_DEG;
    ll.latitude  = lat  * RAD_TO_DEG;

    return ll;
}

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 * Converts longitude and latitude to the xy coordinates on the screen
 * @param longitude
 * @param latitude
 * @return a copy of SDL_Point structure set to representing xy screen coordinates
 */
SDL_Point coordinates_get_point_from_latlon (float longitude, float latitude)
{
	SDL_Point out = {0u};

	out.x = (int)longitude;
	out.y = (int)latitude;

	return out;
}
