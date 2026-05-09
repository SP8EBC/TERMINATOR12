/*
 * srtm.c
 *
 *  Created on: May 8, 2026
 *      Author: mateusz
 */

#include <dirent.h>
#include <errno.h>
#include <logger.h>
#include <regex.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define SIZE_IN_TILES		(1201U)

// Each 3-arc-second data tile has 1442401 integers representing a 1201×1201 grid
#define EXPECTED_SRTM_SIZE	SIZE_IN_TILES*SIZE_IN_TILES*(sizeof(uint16_t)/sizeof(uint8_t))

// elevation data is stored as 16 bit integers,
// height of each cell in meters arranged from west to east and then north to south.
static uint16_t srtm_data[SIZE_IN_TILES][SIZE_IN_TILES];

static void srtm_load(FILE* srtm)
{
	size_t read_result = fread(srtm_data, sizeof(uint8_t), EXPECTED_SRTM_SIZE, srtm);

	assert(read_result == EXPECTED_SRTM_SIZE);
}

void srtm_test (void)
{
	struct dirent *de = NULL; // Pointer for directory entry

	regex_t hgt;

	const int regcomp_result = regcomp (&hgt, "\\.HGT", REG_ICASE);

	if (regcomp_result != 0) {
		LOG_ERROR ("regcomp_result: %d", regcomp_result);
	}

	// opendir() returns a pointer of DIR type.
	DIR *dr = opendir (".");

	while ((de = readdir (dr)) != NULL) {
		if (regexec (&hgt, de->d_name, 0, NULL, 0) == 0) {
			LOG_DEBUG ("matched %s\n", de->d_name);
			break;
		}
	}

	if (de == NULL) {
		LOG_ERROR ("SRTM height data file doesn't found!");
		return;
	}

	FILE *srtm = fopen (de->d_name, "rb");

	if (srtm == NULL) {
		perror (NULL);
		LOG_ERROR ("cannot open SRTM binary file: %s", de->d_name);
	}
	else {
		fseek (srtm, 0L, SEEK_END);
		long int size = ftell (srtm);
		LOG_DEBUG ("file size: %d", size);

		if (size == EXPECTED_SRTM_SIZE) {

			fseek (srtm, 0L, SEEK_SET);

			srtm_load(srtm);

			for (unsigned i = 0; i < SIZE_IN_TILES; i++)
			{
				for (unsigned j = 0; j < SIZE_IN_TILES; j++)
				{
					uint16_t alti = srtm_data[i][j];
					if (alti > 990 && alti < 1100)
					{
						LOG_DEBUG("i: %d, j: %d, alti: %d", i, j, alti);
					}
				}
			}
		}
	}

	fclose (srtm);

	closedir (dr);
}
