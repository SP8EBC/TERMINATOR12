/*
 * airspace_t.h
 *
 *  Created on: Apr 26, 2026
 *      Author: mateusz
 */

#ifndef SRC_TYPES_AIRSPACE_T_H_
#define SRC_TYPES_AIRSPACE_T_H_

#include <stddef.h>

#include "coordinates_t.h"

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

typedef struct airspace_t {
	coordinates_t *vertices;
	size_t num_of_vertices;
	double radius;
	const char * name;
}airspace_t;


#endif /* SRC_TYPES_AIRSPACE_T_H_ */
