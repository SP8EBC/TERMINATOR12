/*
 * line_coordinates_t.h
 *
 *  Created on: Feb 1, 2026
 *      Author: mateusz
 */

#ifndef SRC_TYPES_LINE_COORDINATES_T_H_
#define SRC_TYPES_LINE_COORDINATES_T_H_

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

#define SET_LINE_STARTP(line, point)		\
		line.x1 = point.x;					\
		line.y1 = point.y;					\

/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

typedef struct line_coordinates_t {
	int x1; // begin
	int y1;
	int x2; // end
	int y2;
} line_coordinates_t;


#endif /* SRC_TYPES_LINE_COORDINATES_T_H_ */
