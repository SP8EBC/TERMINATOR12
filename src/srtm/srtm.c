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

#include "coordinates.h"
#include "main_application_config.h"

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

#define SIZE_IN_TILES (1201U)

// Each 3-arc-second data tile has 1442401 integers representing a 1201×1201 grid
#define EXPECTED_SRTM_SIZE (SIZE_IN_TILES * SIZE_IN_TILES * (sizeof (uint16_t) / sizeof (uint8_t)))

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

// elevation data is stored as 16 bit integers,
// height of each cell in meters arranged from west to east and then north to south.
static uint16_t srtm_data[SIZE_IN_TILES][SIZE_IN_TILES];

static SDL_Texture *srtm_texture_for_current_viewport;

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

static void srtm_load (FILE *srtm)
{
	size_t read_result = fread (srtm_data, sizeof (uint8_t), EXPECTED_SRTM_SIZE, srtm);

	assert (read_result == EXPECTED_SRTM_SIZE);
}

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

void srtm_test (SDL_Renderer *renderer, uint16_t min_altitude, uint16_t max_altitude,
				uint16_t minor_step_metres, uint16_t major_step_increments)
{
	(void)srtm_texture_for_current_viewport;
	(void)renderer;
	(void)min_altitude;
	(void)max_altitude;
	(void)minor_step_metres;
	(void)major_step_increments;

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
			// print a filename of first HGT file found.
			LOG_DEBUG ("matched %s\n", de->d_name);
			break;
		}
	}

	if (de == NULL) {
		LOG_ERROR ("SRTM height data file doesn't found!");
		return;
	}

	// open this file
	FILE *srtm = fopen (de->d_name, "rb");

	if (srtm == NULL) {
		perror (NULL);
		LOG_ERROR ("cannot open SRTM binary file: %s", de->d_name);
	}
	else {
		fseek (srtm, 0L, SEEK_END);
		long int size = ftell (srtm);

		// check this size
		LOG_DEBUG ("file size: %d", size);

		// SRTM files (3 arcsecond resolution) have *always* the same size
		if (size == EXPECTED_SRTM_SIZE) {

			fseek (srtm, 0L, SEEK_SET);

			srtm_load (srtm);

			for (unsigned i = 0; i < SIZE_IN_TILES; i++) {
				for (unsigned j = 0; j < SIZE_IN_TILES; j++) {
					uint16_t alti = srtm_data[i][j];
					if (alti > 990 && alti < 1100) {
						LOG_DEBUG ("i: %d, j: %d, alti: %d", i, j, alti);
					}
				}
			}
		}
	}

	fclose (srtm);

	closedir (dr);
}

void srtm_render_for_viewport(SDL_Renderer *renderer, uint16_t min_altitude, uint16_t max_altitude, uint16_t minor_step_metres, uint16_t major_step_increments)
{
	// create new texture to render terrain layer into
	srtm_texture_for_current_viewport = SDL_CreateTexture(renderer, MAIN_DEFAULT_TEXTURE_FORMAT, SDL_TEXTUREACCESS_STATIC, MAIN_WIDTH, MAIN_HEIGHT);
}
