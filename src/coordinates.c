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
// static double coordinates_degrees_longitude_per_pixel;

// static double coordinates_degrees_latitude_per_pixel;

static double coordinates_scale = 10;

static double coordinates_output_scale = 1000.0;

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

static double deg2rad (double deg)
{
	return deg * M_PI / 180.0;
}

static double rad2deg (double rad)
{
	return rad * 180.0 / M_PI;
}

static double normalize_lon_deg (double lon_deg)
{
	while (lon_deg > 180.0)
		lon_deg -= 360.0;
	while (lon_deg < -180.0)
		lon_deg += 360.0;
	return lon_deg;
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
coordinates_t coordinates_mercator_project (double scale, double longitude_deg, double latitude_deg)
{
	const double DEG_TO_RAD = M_PI / 180.0;
	const double MAX_LAT = 85.05112878; // practical Mercator limit

	coordinates_t p;

	if (latitude_deg > MAX_LAT)
		latitude_deg = MAX_LAT;
	if (latitude_deg < -MAX_LAT)
		latitude_deg = -MAX_LAT;

	double lon = longitude_deg * DEG_TO_RAD;
	double lat = latitude_deg * DEG_TO_RAD;

	p.longitude = (scale * lon);
	p.latitude = (scale * log (tan (M_PI / 4.0 + lat / 2.0)));

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
coordinates_t coordinates_mercator_inverse (double scale, double x, double y)
{
	const double RAD_TO_DEG = 180.0 / M_PI;

	coordinates_t ll;

	double lon = x / scale;
	double lat = 2.0 * atan (exp (y / scale)) - M_PI / 2.0;

	ll.longitude = lon * RAD_TO_DEG;
	ll.latitude = lat * RAD_TO_DEG;

	return ll;
}

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
double coordinates_wsg84_distance (double lat1, double lon1, double lat2, double lon2)
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
										  double distance_m, double *lat2_deg, double *lon2_deg)
{
	if (!lat2_deg || !lon2_deg)
		return false;

	/* WGS-84 ellipsoid constants:
	 * a = semi-major axis (equatorial radius)
	 * f = flattening
	 * b = semi-minor axis (polar radius)
	 */
	const double a = 6378137.0;
	const double f = 1.0 / 298.257223563;
	const double b = a * (1.0 - f);

	/* Convert input geographic coordinates and heading to radians
	 * because C trig functions use radians.
	 */
	const double phi1 = deg2rad (lat1_deg);		 /* geodetic latitude of start point */
	const double lambda1 = deg2rad (lon1_deg);	 /* longitude of start point */
	const double alpha1 = deg2rad (bearing_deg); /* initial forward azimuth at start */

	/* Trig values of the initial azimuth.
	 * Precomputed once because they are reused several times.
	 */
	const double sinAlpha1 = sin (alpha1); /* sin(initial bearing) */
	const double cosAlpha1 = cos (alpha1); /* cos(initial bearing) */

	/* Reduced latitude U1.
	 * Vincenty works on the auxiliary sphere rather than directly on the ellipsoid.
	 * U1 is the "parametric" or "reduced" latitude corresponding to phi1.
	 *
	 * tanU1 = (1-f) * tan(phi1)
	 */
	const double tanU1 = (1.0 - f) * tan (phi1); /* tangent of reduced latitude at start */
	const double cosU1 = 1.0 / sqrt (1.0 + tanU1 * tanU1); /* cosine of reduced latitude U1 */
	const double sinU1 = tanU1 * cosU1;					   /* sine of reduced latitude U1 */

	/* sigma1 is the angular distance on the auxiliary sphere
	 * from the equator to the start point, measured along the geodesic.
	 */
	const double sigma1 = atan2 (tanU1, cosAlpha1);

	/* sinAlpha is the azimuth of the geodesic at the equator
	 * on the auxiliary sphere; it stays constant along the geodesic.
	 */
	const double sinAlpha = cosU1 * sinAlpha1;

	/* cosSqAlpha = cos(alpha)^2
	 * This appears repeatedly in Vincenty's series expansions.
	 */
	const double cosSqAlpha = 1.0 - sinAlpha * sinAlpha;

	/* uSq is the "ellipsoid shape" parameter for this geodesic.
	 * It depends on Earth flattening and on the azimuth of the path.
	 * Larger uSq means a stronger ellipsoidal correction is needed.
	 */
	const double uSq = cosSqAlpha * (a * a - b * b) / (b * b);

	/* A and B are series coefficients used to correct the spherical
	 * arc-length solution so it matches the ellipsoidal geodesic.
	 */
	const double A = 1.0 + uSq / 16384.0 * (4096.0 + uSq * (-768.0 + uSq * (320.0 - 175.0 * uSq)));
	const double B = uSq / 1024.0 * (256.0 + uSq * (-128.0 + uSq * (74.0 - 47.0 * uSq)));

	/* sigma is the current estimate of the angular distance on the
	 * auxiliary sphere from the start point to the destination point.
	 *
	 * First guess: distance / (b*A)
	 */
	double sigma = distance_m / (b * A);

	/* sigma_prev stores the previous iteration's sigma so we can test convergence. */
	double sigma_prev;

	/* These are updated each iteration:
	 * sinSigma    = sin(sigma)
	 * cosSigma    = cos(sigma)
	 * cos2SigmaM  = cos(2*sigma_m), where sigma_m is the angular distance
	 *               from the equator to the midpoint of the geodesic segment
	 *               on the auxiliary sphere
	 * deltaSigma  = correction term applied to sigma
	 */
	double sinSigma, cosSigma, cos2SigmaM, deltaSigma;

	/* Iteration counter to avoid infinite loops if convergence fails. */
	int iter = 0;

	do {
		/* cos(2*sigma_m), where:
		 * 2*sigma_m = 2*sigma1 + sigma
		 *
		 * This midpoint-related term appears in Vincenty's correction formula.
		 */
		cos2SigmaM = cos (2.0 * sigma1 + sigma);

		/* Trig values of current sigma estimate. */
		sinSigma = sin (sigma);
		cosSigma = cos (sigma);

		/* Ellipsoidal correction to sigma.
		 * This is the key term that refines the auxiliary-sphere arc length
		 * into the ellipsoidal geodesic solution.
		 */
		deltaSigma = B * sinSigma *
					 (cos2SigmaM + B / 4.0 *
									   (cosSigma * (-1.0 + 2.0 * cos2SigmaM * cos2SigmaM) -
										B / 6.0 * cos2SigmaM * (-3.0 + 4.0 * sinSigma * sinSigma) *
											(-3.0 + 4.0 * cos2SigmaM * cos2SigmaM)));

		/* Save previous sigma before updating. */
		sigma_prev = sigma;

		/* New refined sigma estimate. */
		sigma = distance_m / (b * A) + deltaSigma;

	} while (fabs (sigma - sigma_prev) > 1e-12 && ++iter < 200);

	if (iter >= 200)
		return false;

	/* tmp is an intermediate quantity used in the final latitude formula
	 * and in the reverse-azimuth expression.
	 */
	const double tmp = sinU1 * sinSigma - cosU1 * cosSigma * cosAlpha1;

	/* phi2 is the destination geodetic latitude on the ellipsoid. */
	const double phi2 = atan2 (sinU1 * cosSigma + cosU1 * sinSigma * cosAlpha1,
							   (1.0 - f) * sqrt (sinAlpha * sinAlpha + tmp * tmp));

	/* lambda is the difference in longitude on the auxiliary sphere
	 * before applying the ellipsoidal longitude correction.
	 */
	const double lambda =
		atan2 (sinSigma * sinAlpha1, cosU1 * cosSigma - sinU1 * sinSigma * cosAlpha1);

	/* C is another series coefficient used in the longitude correction term. */
	const double C = f / 16.0 * cosSqAlpha * (4.0 + f * (4.0 - 3.0 * cosSqAlpha));

	/* L is the corrected ellipsoidal longitude difference from start to destination.
	 * This is what must be added to lambda1 to get the destination longitude.
	 */
	const double L =
		lambda -
		(1.0 - C) * f * sinAlpha *
			(sigma +
			 C * sinSigma * (cos2SigmaM + C * cosSigma * (-1.0 + 2.0 * cos2SigmaM * cos2SigmaM)));

	/* Absolute longitude of destination point in radians. */
	const double lambda2 = lambda1 + L;

	/* Convert final answers back to degrees. */
	*lat2_deg = rad2deg (phi2);
	*lon2_deg = normalize_lon_deg (rad2deg (lambda2));

	return true;
}

/**
 * Converts longitude and latitude to the xy coordinates on the screen
 * @param longitude
 * @param latitude
 * @return a copy of SDL_Point structure set to representing xy screen coordinates
 */
SDL_Point coordinates_get_point_from_lonlat (double longitude, double latitude)
{
	(void)coordinates_viewport_limit;
	(void)coordinates_viewport_current;

	const coordinates_t mercator =
		coordinates_mercator_project (coordinates_scale, longitude, latitude);

	const coordinates_t mercator_viewport_origin =
		coordinates_mercator_project (coordinates_scale,
									  coordinates_viewport_current.longitude,
									  coordinates_viewport_current.latitude);

	const double lon = mercator.longitude - mercator_viewport_origin.longitude;
	const double lat = mercator.latitude - mercator_viewport_origin.latitude;

	SDL_Point out = {.x = (int)(lon * coordinates_output_scale),
					 .y = -(int)(lat * coordinates_output_scale)};

	// (0,0)MAIN_HEIGHT point is located in top left corner of a SDL render window
	// while coordinates 0.0 N / 0.0E are well... in the middle?
	// for sure degrees latitude increases from the bottom towards the top
	// out.y = out.y + MAIN_HEIGHT;

	return out;
}

/**
 * @brief Zooms in by adding 'by_this' to the current value of 'coordinates_scale', which is a scale
 * factor used in equations to project WSG-84 coordinates to web-mercator.
 * @note if after adding, the value of 'coordinates_scale' will become zero, it is set to 'by_this'
 * @param by_this a value to increase coordinates_scale
 */
void coordinates_scale_zoom_in (double by_this)
{
	coordinates_scale = coordinates_scale + by_this;

	if (coordinates_scale == 0.0) {
		coordinates_scale = by_this;
	}
}

/**
 * @brief Zooms out by subtracting 'by_this' from the current value of 'coordinates_scale', which is
 * a scale factor used in equations to project WSG-84 coordinates to web-mercator.
 * @note if after subtraction, the value of 'coordinates_scale' will become zero, it is set to
 * negative 'by_this'
 * @param by_this a value to increase coordinates_scale
 */
void coordinates_scale_zoom_out (double by_this)
{
	coordinates_scale = coordinates_scale - by_this;

	if (coordinates_scale == 0.0) {
		coordinates_scale = -by_this;
	}
}

/**
 * @brief Zooms in by adding 'by_this' to the current value of 'coordinates_output_scale', which
 * is a scale factor applied to web-mercator xy *after* projecting from WSG-84 coordinates
 * to web-mercator.
 * @note if after adding, the value of 'coordinates_output_scale' will become zero, it is set to
 * 'by_this'
 * @param by_this a value to increase coordinates_scale
 */
void coordinates_output_scale_zoom_in (double by_this)
{
	coordinates_output_scale = coordinates_output_scale + by_this;

	if (coordinates_output_scale == 0.0) {
		coordinates_output_scale = by_this;
	}
}

/**
 * @brief Zooms out by subtracting 'by_this' from the current value of 'coordinates_output_scale',
 * which is a scale factor applied to web-mercator xy *after* projecting from WSG-84 coordinates to
 * web-mercator.
 * @note if after subtracting, the value of 'coordinates_output_scale' will become zero, it is set
 * to 'by_this'
 * @param by_this a value to increase coordinates_scale
 */
void coordinates_output_scale_zoom_out (double by_this)
{
	coordinates_output_scale = coordinates_output_scale - by_this;

	if (coordinates_output_scale == 0.0) {
		coordinates_output_scale = -by_this;
	}
}

/**
 * @brief Moves origin point, which simply results in moving map left/right/up/down
 * @param towards_there direction we want to move to
 * @param by_this degrees of latitude or longitude
 */
void coordinates_move_origin (coordinate_direction_t towards_there, double by_this)
{
	coordinates_t new = {0u};

	// clang-format off
	switch (towards_there) {
		case WEST: {
			new.longitude = coordinates_viewport_current.longitude - by_this;
			new.latitude = coordinates_viewport_current.latitude;
			break;
		}
		case NORTH: {
			new.longitude = coordinates_viewport_current.longitude;
			new.latitude = coordinates_viewport_current.latitude + by_this;
			break;
		}
		case EAST: {
			new.longitude = coordinates_viewport_current.longitude + by_this;
			new.latitude = coordinates_viewport_current.latitude;
			break;
		}
		case SOUTH: {
			new.longitude = coordinates_viewport_current.longitude;
			new.latitude = coordinates_viewport_current.latitude - by_this;
			break;
		}
	}
	// clang-format on

	// check if a limit was reached
	if (new.latitude < coordinates_viewport_limit.latitude &&
			new.longitude > coordinates_viewport_limit.longitude)
	{
		coordinates_viewport_current = new;
	}
}

coordinates_t coordinates_return_current_viewport (void)
{
	return coordinates_viewport_current;
}
