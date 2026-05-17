/*
 * ogn.h
 *
 *	OGN/Glidernet APRS-IS reader. Optional, compiled in when WITH_OGN=1.
 *	A background pthread owns the TCP socket and writes length-prefixed
 *	protobuf frames into a named FIFO; the main thread polls the FIFO
 *	non-blocking via @link{ogn_read_next}.
 *
 *  Created on: May 17, 2026
 *      Author: mateusz
 */

#ifndef SRC_OGN_OGN_H_
#define SRC_OGN_OGN_H_

#include <stdint.h>

#include "types/ogn_position_t.h"

/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

typedef enum ogn_status_t {
	OGN_NOT_INITIALIZED = 0,
	OGN_INITIALIZED,
	OGN_DISCONNECTED,
	OGN_CONNECTED,
	OGN_FAILED
} ogn_status_t;

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 * @brief Initializes OGN module. Creates (or reuses) the FIFO at @c path_to_fifo and
 * opens it for read on the main side. Must be called before @link{ogn_connect}.
 * @return 0 on success, -1 on failure (errno set).
 */
int ogn_init (const char *path_to_fifo);

/**
 * @brief Spawns the worker pthread and asks it to connect to the given APRS-IS server.
 * Filter is built as "r/<latitude>/<longitude>/<radius_km>" per the APRS-IS server spec.
 * @return 0 on success (worker spawned), -1 if module is not initialized or thread create failed.
 */
int ogn_connect (const char *glidernet_server, uint16_t port, uint8_t radius_km, float latitude,
				 float longitude);

/**
 * @brief Returns current worker status. Cheap and lock-free (atomic read).
 */
ogn_status_t ogn_check_status (void);

/**
 * @brief How many heartbeat-timeouts have happened since @link{ogn_connect}.
 */
uint32_t ogn_get_timeouts_cntr (void);

/**
 * @brief How many successfully parsed position reports have been pushed into the FIFO
 * by the worker so far.
 */
uint32_t ogn_get_position_reports_cntr (void);

/**
 * @brief Pulls the next decoded position out of the FIFO. Non-blocking — returns 0 if
 * the FIFO has no complete frame ready. Caller is expected to call this in a loop each
 * frame until it returns 0.
 * @return 1 if @c out was populated, 0 if no data was available, -1 on hard error.
 */
int ogn_read_next (ogn_position_t *out);

/**
 * @brief Signals the worker to disconnect and joins it. Safe to call multiple times.
 * @return 0 on success, -1 if module is not initialized.
 */
int ogn_disconnect (void);

/**
 * @brief Releases all resources. Closes FIFO, unlinks it. Must be the last call into the
 * module. Implicitly performs @link{ogn_disconnect} if the worker is still running.
 */
int ogn_deinit (void);

#endif /* SRC_OGN_OGN_H_ */
