/*
 * main_apllication_config.h
 *
 *  Created on: Apr 9, 2026
 *      Author: mateusz
 */

#ifndef CONFIG_MAIN_APPLICATION_CONFIG_H_
#define CONFIG_MAIN_APPLICATION_CONFIG_H_

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

/**
 *  * \param w the width of the window, in screen coordinates
 * \param h the height of the window, in screen coordinates
 */
#define MAIN_WIDTH	(1024)
#define MAIN_HEIGHT (768)

/**
 * @brief top left GPS-WSG84 coordinate, which defines a place to which the viewport could be
 * moved / zoomed
 * @note assumes latitude N & longitude E
 */
#define MAIN_VIEWPORT_LIMIT(ENTRY) ENTRY (50.0473358, 18.70000)

/**
 * @brief top left GPS-WSG84 coordinate of default location
 * @note assumes latitude N & longitude E
 */
#define MAIN_VIEWPORT_DEFAULT_LOCATION(ENTRY) ENTRY (49.8044983, 18.8900422)

/**
 * @brief float which is a longitude size of a viewport expressed as degrees, which is
 * added or subtracted from the longitude of @link{MAIN_VIEWPORT_LIMIT}
 */
#define MAIN_VIEWPORT_LIMIT_LON_SIZE (1.2)

#define MAIN_VIEWPORT_LIMIT_LAT_SIZE (-0.7)

#endif /* CONFIG_MAIN_APPLICATION_CONFIG_H_ */
