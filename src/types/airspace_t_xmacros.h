/*
 * airspace_t_xmacros.h
 *
 *  Created on: Apr 26, 2026
 *      Author: mateusz
 */

#ifndef SRC_TYPES_AIRSPACE_T_XMACROS_H_
#define SRC_TYPES_AIRSPACE_T_XMACROS_H_

/// ==================================================================================================
///	GLOBAL X-MACROS - used to instantiate types from "airspace_t.h"
/// ==================================================================================================

/**
 * @brief creates an instance of airspace_t struct for single entry defined in AIRSPACES_HARDCODED_LIST
 */
#define AIRSPACE_CREATE(_radius, _name, ...)													\
	static airspace_t airspace_##_name = {														\
		.vertices = (coordinates_t[]){__VA_ARGS__},												\
		.num_of_vertices = sizeof((coordinates_t[]){__VA_ARGS__}) / sizeof(coordinates_t),		\
		.radius = _radius,																		\
		.name = #_name																			\
};																								\
																								\

/**
 * @brief creates single entry of an array "heap_hardcoded_airspaces" of pointers to all hardcoded airspaces
 */
#define AIRSPACE_ARRAY_OF_POINTERS_ENTRY(_radius, _name, ...)				\
		&airspace_##_name,													\

/**
 * @brief creates single entry of an enum "heap_hardcoded_airspaces_list_t", which is used
 * to count of airspaces hardcoded in "heap_hardcoded_airspaces" by AIRSPACES_HARDCODED_LIST
 */
#define AIRSPACE_ENUM_LIST_ENTRY(_radius, _name, ...)				\
		HEAP_HARDCODED_AIRSPACE_##_name,							\

#endif /* SRC_TYPES_AIRSPACE_T_XMACROS_H_ */
