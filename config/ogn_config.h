/*
 * ogn_config.h
 *
 *  Created on: May 17, 2026
 *      Author: mateusz
 */

#ifndef CONFIG_OGN_CONFIG_H_
#define CONFIG_OGN_CONFIG_H_

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

//!< APRS-IS server hostname for the OGN/Glidernet network
#define OGN_DEFAULT_SERVER "aprs.glidernet.org"

//!< standard APRS-IS server-port for filtered text streams
#define OGN_DEFAULT_PORT 14580

//!< if no data for this many seconds, the worker considers the link dead and reconnects
#define OGN_HEARTBEAT_TIMEOUT_S 60

//!< named FIFO used to ferry length-prefixed protobuf frames worker -> main
#define OGN_FIFO_PATH "/tmp/terminator12_ogn.fifo"

//!< how many aircraft slots the renderer keeps; oldest is evicted when full
#define OGN_MAX_TRACKED 64U

//!< if an aircraft slot has not been refreshed within this many seconds, it is dropped
#define OGN_STALE_SECONDS 120U

//!< default APRS filter radius in km used in the login line "filter r/<lat>/<lon>/<radius>"
#define OGN_DEFAULT_RADIUS_KM 100U

//!< maximum length of a single APRS line we will accept (well above any real packet)
#define OGN_APRS_LINE_MAX 512U

#endif /* CONFIG_OGN_CONFIG_H_ */
