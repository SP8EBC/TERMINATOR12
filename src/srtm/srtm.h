/*
 * srtm.h
 *
 *  Created on: May 8, 2026
 *      Author: mateusz
 */

#ifndef SRC_SRTM_SRTM_H_
#define SRC_SRTM_SRTM_H_

#include <SDL2/SDL.h>


void srtm_load_files (const char * assets_dir_path);

void srtm_render_for_viewport(SDL_Renderer *renderer, uint16_t min_altitude, uint16_t max_altitude, uint16_t minor_step_metres, uint16_t major_step_increments);

/**
 * @brief Releases any SDL texture currently held by the SRTM layer. Must be called
 * before SDL_Quit().
 */
void srtm_shutdown(void);

#endif /* SRC_SRTM_SRTM_H_ */
