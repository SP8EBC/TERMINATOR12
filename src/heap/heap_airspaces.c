/*
 * heap_airspaces.c
 *
 *  Created on: Apr 26, 2026
 *      Author: mateusz
 */

#include "types/airspace_t.h"
#include "types/airspace_t_xmacros.h"

#include "airspaces_hardcoded.h"

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

// clang-format off
/**
 *
 */
typedef enum heap_hardcoded_airspaces_list_t {
	AIRSPACES_HARDCODED_LIST (AIRSPACE_ENUM_LIST_ENTRY)
	HEAP_HARDCODED_AIRSPACE_NUM
} heap_hardcoded_airspaces_list_t;

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

/**
 * @brief Set of local structures, each one resembling single hardcoded structure
 */
AIRSPACES_HARDCODED_LIST (AIRSPACE_CREATE)

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/**
 * @brief an array of pointers to all hardcoded airspaces
 */
const airspace_t *heap_hardcoded_airspaces[] = {
	AIRSPACES_HARDCODED_LIST (AIRSPACE_ARRAY_OF_POINTERS_ENTRY)
};
// clang-format on

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

size_t heap_hardcoded_airspaces_count ()
{
	return (size_t)HEAP_HARDCODED_AIRSPACE_NUM;
}
