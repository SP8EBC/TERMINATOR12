/*
 * aircraft_stv_t.h
 *
 *	Aircraft state vector
 *
 *  Created on: Feb 1, 2026
 *      Author: mateusz
 */

#ifndef SRC_TYPES_AIRCRAFT_STV_T_H_
#define SRC_TYPES_AIRCRAFT_STV_T_H_


/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

typedef struct aircraft_stv_t {
	float lat;
	float lon;
	unsigned altitude;		//!< Feets
	short bearing;
}aircraft_stv_t;


#endif /* SRC_TYPES_AIRCRAFT_STV_T_H_ */
