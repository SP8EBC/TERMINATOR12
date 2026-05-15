/*
 * srtm.c
 *
 *  Created on: May 8, 2026
 *      Author: mateusz
 */

#include "srtm.h"

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <logger.h>
#include <regex.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "coordinates.h"
#include "main_application_config.h"

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

//!< How many tiles are stored in single SRTM binary HGT file
#define SIZE_IN_TILES (1201U)

//!< Each 3-arc-second data tile has 1442401 integers representing a 1201×1201 grid
#define EXPECTED_SRTM_SIZE (SIZE_IN_TILES * SIZE_IN_TILES * (sizeof (uint16_t) / sizeof (uint8_t)))

//!< How many SRTM files could be loaded
#define SRTM_FILES_BUFFER_SIZE 4

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

/**
 * @brief Metadata for single SRTM file
 * @note  files are labeled with the coordinate of the southwest cell. For example, the file
 * N20E100.hgt contains data from 20°N to 21°N and from 100°E to 101°E inclusive.
 */
typedef struct srtm_metadata_t {
	char filename[12]; //!< typical filename is NxxEyyy.HGT, see note above
	int16_t latitude;  //!< of a southwest cell
	int16_t lontitude; //!< of a southwest cell
} srtm_metadata_t;

/**
 * @brief type encloses a two dimensional array of data read from SRTM file. It is used mainly
 * to omit a need to use three dimensional array
 */
typedef struct srtm_data_container_t {

	// elevation data is stored as 16 bit integers,
	// height of each cell in meters arranged from west to east and then north to south.

	// elevation data is stored as 16 bit integers,
	// height of each cell in meters arranged from west to east and then north to south.

	uint16_t data[SIZE_IN_TILES][SIZE_IN_TILES];
} srtm_data_container_t;

// clang-format off
/**
 * @brief
 */
typedef enum srtm_to_viewport_corner_t {
	SRTM_VIREPORT_CORNER_NOTHING, //!< SRTM file area is bigger than a viewport -> single match
	SRTM_VIEWPORT_CORNER_NW, //!< North-western corner. Interesting HGT data are on x & y indexes higher than start point
	SRTM_VIEWPORT_CORNER_NE, //!< Nort-eastern corner. Interesting HGT data are on x indexes lower and y indexes higher
	SRTM_VIEWPORT_CORNER_SW, //!< South-western corner. Interesting HGT data are on x indexes higher and y indexes lower
	SRTM_VIEWPORT_CORNER_SE  //!< South-estern corner. Interesting HGT data are on x & y indexes lower than start point
} srtm_to_viewport_corner_t;
// clang-format on

/**
 * @brief Structure resembles single match between displayed viewport and SRTM height file
 */
typedef struct srtm_to_viewport_match_t {
	size_t idx; //!< Index in 'srtm_meta' and 'srtm_data'
	coordinates_t start_point;
	srtm_to_viewport_corner_t corner;
} srtm_to_viewport_match_t;

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

static srtm_metadata_t srtm_meta[SRTM_FILES_BUFFER_SIZE];

static srtm_data_container_t srtm_data[SRTM_FILES_BUFFER_SIZE];

static SDL_Texture *srtm_texture_for_current_viewport;
/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/**
 * Find SRTM hgt files to fill given viewport with topo layer
 * @param viewport current viewport
 * @param zoom current zoom level
 * @param out pointer to an array where results will be stored, the array *must be* at least 4 elems
 * @return how many files have been found within content of @link{srtm_data}
 */
static int srtm_find_for_viewport (coordinates_t viewport, double zoom,
								   srtm_to_viewport_match_t *out)
{
	int how_many_found = 0;

	// pointer to currently processed element
	const srtm_metadata_t *current_elem = 0;

	// HGT data can be stored in any order
	for (size_t i = 0; i < SRTM_FILES_BUFFER_SIZE; i++) {
		current_elem = &srtm_meta[i];

		const int16_t lat = current_elem->latitude;
		const int16_t lon = current_elem->lontitude;

		if (((int16_t)viewport.latitude > lat) && ((int16_t)viewport.latitude < lat + 1))
		{

		}
	}

	return how_many_found;
}

/**
 * Loads binary data from single HGT file
 * @param srtm_filename path and filename to binary HGT file
 * @param index at which data and metadata should be stored in @link{srtm_meta} and
 * @link{srtm_data}
 */
static void srtm_load (char *srtm_filename, size_t index)
{
	char temp[4] = {0u}; // to manipulate lat/lon from a filename

	// open this file
	FILE *srtm = fopen (srtm_filename, "rb");

	if (srtm == NULL) {
		perror (NULL);
		LOG_ERROR ("cannot open SRTM binary file: %s", srtm_filename);
	}
	else {
		fseek (srtm, 0L, SEEK_END);
		long int size = ftell (srtm);

		// SRTM files (3 arcsecond resolution) have *always* the same size
		if (size == EXPECTED_SRTM_SIZE) {

			fseek (srtm, 0L, SEEK_SET);

			//			for (unsigned i = 0; i < SIZE_IN_TILES; i++) {
			//				for (unsigned j = 0; j < SIZE_IN_TILES; j++) {
			//					uint16_t alti = srtm_data[i][j];
			//					if (alti > 990 && alti < 1100) {
			//						LOG_DEBUG ("i: %d, j: %d, alti: %d", i, j, alti);
			//					}
			//				}
			//			}

			size_t read_result =
				fread (&srtm_data[index], sizeof (uint8_t), EXPECTED_SRTM_SIZE, srtm);

			fclose (srtm);

			assert (read_result == EXPECTED_SRTM_SIZE);
		}
		// assume N / E as for now

		// extract latitude
		memcpy (temp, srtm_filename + 1, 2);
		srtm_meta[index].latitude = (short int)atoi (temp);

		// extract longitude
		memcpy (temp, srtm_filename + 4, 3);
		srtm_meta[index].lontitude = (short int)atoi (temp);

		LOG_INFO ("file: %s, latitude: %d, longitude: %d",
				  srtm_filename,
				  srtm_meta[index].latitude,
				  srtm_meta[index].lontitude);
	}
}

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 * @note calling this function more than once will overwrite all previously loaded HGT data
 * @param assets_dir_path
 */
void srtm_load_files (const char *assets_dir_path)
{
	(void)assets_dir_path;

	size_t idx = (size_t)0;

	struct dirent *de = NULL; // Pointer for directory entry

	regex_t hgt; // regex to look for HGT file

	// create a regular expression to filter *.HGT files
	const int regcomp_result = regcomp (&hgt, "\\.HGT", REG_ICASE);

	if (regcomp_result != 0) {
		LOG_ERROR ("regcomp_result: %d", regcomp_result);
	}

	// opendir() returns a pointer of DIR type.
	DIR *dr = opendir (".");

	// go through a content of current directory
	while ((de = readdir (dr)) != NULL) {
		if (regexec (&hgt, de->d_name, 0, NULL, 0) == 0) {
			// print a filename of HGT file found.
			LOG_DEBUG ("matched %s\n", de->d_name);
			srtm_load (de->d_name, idx++);
			if (idx >= SRTM_FILES_BUFFER_SIZE) {
				break;
			}
		}
	}

	if (idx == 0) {
		LOG_ERROR ("SRTM height data file doesn't found!");
		return;
	}

	closedir (dr);
}

void srtm_render_for_viewport (SDL_Renderer *renderer, uint16_t min_altitude, uint16_t max_altitude,
							   uint16_t minor_step_metres, uint16_t major_step_increments)
{

	(void)renderer;
	(void)min_altitude;
	(void)max_altitude;
	(void)minor_step_metres;
	(void)major_step_increments;

	srtm_to_viewport_match_t matching[4] = {0u};

	coordinates_t viewport = coordinates_return_current_viewport ();

	double zoom_level = coordinates_return_current_scale ();

	srtm_find_for_viewport (viewport, zoom_level, matching);

	// create new texture to render terrain layer into
	srtm_texture_for_current_viewport = SDL_CreateTexture (renderer,
														   MAIN_DEFAULT_TEXTURE_FORMAT,
														   SDL_TEXTUREACCESS_STATIC,
														   MAIN_WIDTH,
														   MAIN_HEIGHT);
}
