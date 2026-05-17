/*
 * ogn_aprs.h
 *
 *	Pure parser for APRS-IS position-report lines. No globals, no I/O.
 *	Kept separate from @file{ogn.c} so it can be exercised by unit tests
 *	without involving sockets or pthreads.
 *
 *  Created on: May 17, 2026
 *      Author: mateusz
 */

#ifndef SRC_OGN_OGN_APRS_H_
#define SRC_OGN_OGN_APRS_H_

#include <stdbool.h>
#include <stddef.h>

#include "types/ogn_position_t.h"

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 * @brief Parses one APRS-IS line (already stripped of trailing CR/LF). Comment / heartbeat
 * lines that begin with '#' are rejected via the return value so the caller can update its
 * heartbeat timestamp without trying to dereference @c out.
 *
 * Typical input:
 *   FLR3F2B91>APRS,qAS,LFMD:/123456h5123.45N/00123.45E'180/025/A=001000 +050fpm 1.5rot ...
 *
 * @param line   NUL-terminated APRS line, must be non-NULL.
 * @param line_len  length of @c line without the trailing NUL; used to bound parser scans.
 * @param out    populated on success; left untouched on failure.
 * @return true if @c out was filled with a valid position, false on any parsing problem.
 */
bool ogn_aprs_parse_line (const char *line, size_t line_len, ogn_position_t *out);

#endif /* SRC_OGN_OGN_APRS_H_ */
