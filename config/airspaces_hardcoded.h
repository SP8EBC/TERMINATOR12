/*
 * airspaces_hardcoded.h
 *
 *  Created on: Apr 26, 2026
 *      Author: mateusz
 */

#ifndef CONFIG_AIRSPACES_HARDCODED_H_
#define CONFIG_AIRSPACES_HARDCODED_H_

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

// clang-format off
/**
 * @brief hardcoded list of airspaces to show
 */
#define AIRSPACES_HARDCODED_LIST(ENTRY) \
	ENTRY (0, EPBA, {49.8666667, 18.7666}, {49.8666667, 19.0833}, {49.65, 19.0666}, {49.65, 18.8666}, {49.86, 18.76666})	\
	ENTRY (4000, EPBX, {49.6855667f, 19.0312978f})	\

	// clang-format on

#endif /* CONFIG_AIRSPACES_HARDCODED_H_ */
