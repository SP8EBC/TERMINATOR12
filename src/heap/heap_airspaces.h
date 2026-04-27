/*
 * heap_airspaces.h
 *
 *  Created on: Apr 26, 2026
 *      Author: mateusz
 */

#ifndef SRC_HEAP_HEAP_AIRSPACES_H_
#define SRC_HEAP_HEAP_AIRSPACES_H_

#include "types/airspace_t.h"


/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/**
 * @brief an array of pointers to all hardcoded airspaces
 */
extern const airspace_t *heap_hardcoded_airspaces[];

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 *
 * @return amount of airspaces hardcoded in definition AIRSPACES_HARDCODED_LIST in "airspaces_hardcoded.h"
 */
size_t heap_hardcoded_airspaces_count();


#endif /* SRC_HEAP_HEAP_AIRSPACES_H_ */
