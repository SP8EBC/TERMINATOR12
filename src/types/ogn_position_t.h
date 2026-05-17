/*
 * ogn_position_t.h
 *
 *  Created on: May 17, 2026
 *      Author: mateusz
 */

#ifndef SRC_TYPES_OGN_POSITION_T_H_
#define SRC_TYPES_OGN_POSITION_T_H_

#include <stdint.h>

/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

//!< callsign field carries the APRS source (max 9 chars + NUL in real-world OGN traffic)
#define OGN_CALLSIGN_MAX 16

/**
 * @brief Plain-data position report fed to the renderer aircraft table.
 * Mirrors the protobuf message but lives entirely in-process so that draw code never
 * has to touch protobuf-c types.
 */
typedef struct ogn_position_t {
	char	 callsign[OGN_CALLSIGN_MAX];
	uint64_t rx_epoch_s;		 //!< wall-clock time of reception (worker thread)
	uint64_t msg_epoch_s;		 //!< wall-clock time encoded in the APRS HHMMSS field
	double	 latitude_deg;
	double	 longitude_deg;
	int32_t	 altitude_feet;
	uint32_t ground_track_deg;	 //!< 0..359
	uint32_t ground_speed_kt;
	int32_t	 vertical_speed_fpm; //!< signed, can be negative (descent)
} ogn_position_t;

#endif /* SRC_TYPES_OGN_POSITION_T_H_ */
