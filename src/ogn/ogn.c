/*
 * ogn.c
 *
 *  Created on: May 17, 2026
 *      Author: mateusz
 */

#include "ogn.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <logger.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "ogn_aprs.h"
#include "ogn_config.h"
#include "ogn_messages.pb-c.h"

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

//!< maximum protobuf payload size we accept on either side of the FIFO
#define OGN_FRAME_MAX (4096U)

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

typedef struct ogn_connect_args_t {
	char	server[64];
	uint16_t port;
	uint8_t radius_km;
	float	latitude;
	float	longitude;
} ogn_connect_args_t;

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

static char ogn_fifo_path[256];
static int ogn_fifo_read_fd = -1;
static int ogn_fifo_write_fd = -1;	//!< only used by the worker

static pthread_t ogn_worker_thread;
static bool ogn_worker_running = false;
static atomic_int ogn_shutdown_requested = 0;
static _Atomic ogn_status_t ogn_status = OGN_NOT_INITIALIZED;

static atomic_uint ogn_timeouts_cntr = 0;
static atomic_uint ogn_reports_cntr = 0;

static ogn_connect_args_t ogn_connect_args;

//!< partial frame accumulator on the read side (FIFO is non-blocking, frames may straddle reads)
static uint8_t ogn_read_buf[OGN_FRAME_MAX + 4];
static size_t ogn_read_buf_len = 0;

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/**
 * @brief Encodes a position_t into a protobuf frame and writes it to the FIFO with a
 * 4-byte little-endian length prefix. Best-effort: drops the frame on a full FIFO
 * rather than blocking the worker.
 */
static void ogn_worker_emit_frame (const ogn_position_t *pos)
{
	Terminator12__Ogn__PositionReport msg = TERMINATOR12__OGN__POSITION_REPORT__INIT;
	msg.callsign			= (char *)pos->callsign;
	msg.rx_epoch_s			= pos->rx_epoch_s;
	msg.msg_epoch_s			= pos->msg_epoch_s;
	msg.latitude_deg		= pos->latitude_deg;
	msg.longitude_deg		= pos->longitude_deg;
	msg.altitude_feet		= pos->altitude_feet;
	msg.ground_track_deg	= pos->ground_track_deg;
	msg.ground_speed_kt		= pos->ground_speed_kt;
	msg.vertical_speed_fpm	= pos->vertical_speed_fpm;

	size_t packed_size = terminator12__ogn__position_report__get_packed_size (&msg);
	if (packed_size == 0 || packed_size > OGN_FRAME_MAX) {
		LOG_WARN ("ogn: oversize protobuf frame (%zu bytes), dropping", packed_size);
		return;
	}

	uint8_t buf[4 + OGN_FRAME_MAX];
	buf[0] = (uint8_t)(packed_size & 0xFF);
	buf[1] = (uint8_t)((packed_size >> 8) & 0xFF);
	buf[2] = (uint8_t)((packed_size >> 16) & 0xFF);
	buf[3] = (uint8_t)((packed_size >> 24) & 0xFF);
	terminator12__ogn__position_report__pack (&msg, buf + 4);

	ssize_t w = write (ogn_fifo_write_fd, buf, 4 + packed_size);
	if (w < 0) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			LOG_WARN ("ogn: FIFO write error: %s", strerror (errno));
		}
	}
}

/**
 * @brief Best-effort TCP connect. Returns a socket fd or -1 on failure.
 */
static int ogn_worker_open_socket (const char *server, uint16_t port)
{
	struct addrinfo hints;
	memset (&hints, 0, sizeof (hints));
	hints.ai_family   = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	char portstr[8];
	snprintf (portstr, sizeof (portstr), "%u", (unsigned)port);

	struct addrinfo *res = NULL;
	const int gai = getaddrinfo (server, portstr, &hints, &res);
	if (gai != 0 || res == NULL) {
		LOG_ERROR ("ogn: getaddrinfo(%s): %s", server, gai_strerror (gai));
		return -1;
	}

	int fd = socket (res->ai_family, res->ai_socktype, res->ai_protocol);
	if (fd < 0) {
		LOG_ERROR ("ogn: socket(): %s", strerror (errno));
		freeaddrinfo (res);
		return -1;
	}

	if (connect (fd, res->ai_addr, res->ai_addrlen) < 0) {
		LOG_ERROR ("ogn: connect(%s:%u): %s", server, (unsigned)port, strerror (errno));
		close (fd);
		freeaddrinfo (res);
		return -1;
	}
	freeaddrinfo (res);

	// rcv timeout so we can poll the shutdown flag periodically
	struct timeval tv;
	tv.tv_sec  = OGN_HEARTBEAT_TIMEOUT_S;
	tv.tv_usec = 0;
	(void)setsockopt (fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof (tv));

	return fd;
}

/**
 * @brief Sends the APRS-IS login line including the geographic filter.
 */
static bool ogn_worker_send_login (int fd, const ogn_connect_args_t *args)
{
	char login[256];
	int n = snprintf (login,
					  sizeof (login),
					  "user N0CALL pass -1 vers TERMINATOR12 0.1 filter r/%.4f/%.4f/%u\r\n",
					  args->latitude,
					  args->longitude,
					  (unsigned)args->radius_km);
	if (n <= 0 || n >= (int)sizeof (login)) {
		return false;
	}
	ssize_t w = send (fd, login, (size_t)n, MSG_NOSIGNAL);
	return (w == n);
}

/**
 * @brief Drains pending bytes from the socket, splits on '\n', dispatches each line.
 * Returns false on socket error (caller will close + reconnect).
 */
static bool ogn_worker_pump_socket (int fd, char *buf, size_t *buf_len)
{
	ssize_t r = recv (fd, buf + *buf_len, OGN_APRS_LINE_MAX - *buf_len - 1, 0);
	if (r == 0) {
		LOG_WARN ("ogn: peer closed connection");
		return false;
	}
	if (r < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			// recv timed out: treat as a missed heartbeat
			atomic_fetch_add (&ogn_timeouts_cntr, 1);
			LOG_WARN ("ogn: heartbeat timeout");
			return false;
		}
		LOG_WARN ("ogn: recv: %s", strerror (errno));
		return false;
	}

	*buf_len += (size_t)r;
	buf[*buf_len] = '\0';

	// scan for complete lines
	size_t start = 0;
	for (size_t i = 0; i < *buf_len; i++) {
		if (buf[i] == '\n') {
			size_t line_len = i - start;
			// strip trailing CR
			while (line_len > 0 && buf[start + line_len - 1] == '\r') {
				line_len--;
			}
			buf[start + line_len] = '\0';

			if (line_len > 0) {
				if (buf[start] == '#') {
					LOG_DEBUG ("ogn rx# %.*s", (int)line_len, buf + start);
				}
				else {
					ogn_position_t pos;
					if (ogn_aprs_parse_line (buf + start, line_len, &pos)) {
						LOG_DEBUG ("ogn parsed %s lat=%.4f lon=%.4f alt=%d ft trk=%u spd=%u",
								   pos.callsign,
								   pos.latitude_deg,
								   pos.longitude_deg,
								   pos.altitude_feet,
								   pos.ground_track_deg,
								   pos.ground_speed_kt);
						ogn_worker_emit_frame (&pos);
						atomic_fetch_add (&ogn_reports_cntr, 1);
					}
					else {
						LOG_DEBUG ("ogn drop %.*s", (int)line_len, buf + start);
					}
				}
			}
			start = i + 1;
		}
	}

	// shift any partial tail to the front
	if (start > 0) {
		size_t remaining = *buf_len - start;
		memmove (buf, buf + start, remaining);
		*buf_len = remaining;
	}

	// protect against a too-long line: drop the buffer if no newline ever arrives
	if (*buf_len >= OGN_APRS_LINE_MAX - 1) {
		LOG_WARN ("ogn: oversized line, dropping buffered bytes");
		*buf_len = 0;
	}

	return true;
}

/**
 * @brief Worker thread entry. Reconnects on socket error until the shutdown flag is set.
 */
static void *ogn_worker_main (void *arg)
{
	(void)arg;
	char	line_buf[OGN_APRS_LINE_MAX];
	size_t	line_buf_len = 0;

	while (!atomic_load (&ogn_shutdown_requested)) {
		atomic_store (&ogn_status, OGN_DISCONNECTED);
		int fd = ogn_worker_open_socket (ogn_connect_args.server, ogn_connect_args.port);
		if (fd < 0) {
			sleep (5);
			continue;
		}
		if (!ogn_worker_send_login (fd, &ogn_connect_args)) {
			LOG_WARN ("ogn: failed to send login");
			close (fd);
			sleep (5);
			continue;
		}
		LOG_INFO ("ogn: connected to %s:%u filter r/%.4f/%.4f/%u",
				  ogn_connect_args.server,
				  (unsigned)ogn_connect_args.port,
				  (double)ogn_connect_args.latitude,
				  (double)ogn_connect_args.longitude,
				  (unsigned)ogn_connect_args.radius_km);
		atomic_store (&ogn_status, OGN_CONNECTED);
		line_buf_len = 0;

		while (!atomic_load (&ogn_shutdown_requested)) {
			if (!ogn_worker_pump_socket (fd, line_buf, &line_buf_len)) {
				break;
			}
		}
		close (fd);
	}
	atomic_store (&ogn_status, OGN_DISCONNECTED);
	return NULL;
}

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

int ogn_init (const char *path_to_fifo)
{
	if (path_to_fifo == NULL) {
		return -1;
	}
	strncpy (ogn_fifo_path, path_to_fifo, sizeof (ogn_fifo_path) - 1);
	ogn_fifo_path[sizeof (ogn_fifo_path) - 1] = '\0';

	// (re)create the FIFO; ignore EEXIST so multiple runs Just Work.
	if (mkfifo (ogn_fifo_path, 0600) < 0 && errno != EEXIST) {
		LOG_ERROR ("ogn: mkfifo(%s): %s", ogn_fifo_path, strerror (errno));
		return -1;
	}

	// open the read end non-blocking first so the worker's write open doesn't block.
	ogn_fifo_read_fd = open (ogn_fifo_path, O_RDONLY | O_NONBLOCK);
	if (ogn_fifo_read_fd < 0) {
		LOG_ERROR ("ogn: open(read %s): %s", ogn_fifo_path, strerror (errno));
		return -1;
	}
	ogn_fifo_write_fd = open (ogn_fifo_path, O_WRONLY | O_NONBLOCK);
	if (ogn_fifo_write_fd < 0) {
		LOG_ERROR ("ogn: open(write %s): %s", ogn_fifo_path, strerror (errno));
		close (ogn_fifo_read_fd);
		ogn_fifo_read_fd = -1;
		return -1;
	}

	atomic_store (&ogn_status, OGN_INITIALIZED);
	return 0;
}

int ogn_connect (const char *glidernet_server, uint16_t port, uint8_t radius_km, float latitude,
				 float longitude)
{
	if (ogn_check_status () == OGN_NOT_INITIALIZED) {
		return -1;
	}
	if (ogn_worker_running) {
		return 0; // already running
	}

	memset (&ogn_connect_args, 0, sizeof (ogn_connect_args));
	strncpy (ogn_connect_args.server,
			 glidernet_server,
			 sizeof (ogn_connect_args.server) - 1);
	ogn_connect_args.port	   = port;
	ogn_connect_args.radius_km = radius_km;
	ogn_connect_args.latitude  = latitude;
	ogn_connect_args.longitude = longitude;

	atomic_store (&ogn_shutdown_requested, 0);
	if (pthread_create (&ogn_worker_thread, NULL, ogn_worker_main, NULL) != 0) {
		LOG_ERROR ("ogn: pthread_create failed: %s", strerror (errno));
		return -1;
	}
	ogn_worker_running = true;
	return 0;
}

ogn_status_t ogn_check_status (void)
{
	return atomic_load (&ogn_status);
}

uint32_t ogn_get_timeouts_cntr (void)
{
	return atomic_load (&ogn_timeouts_cntr);
}

uint32_t ogn_get_position_reports_cntr (void)
{
	return atomic_load (&ogn_reports_cntr);
}

int ogn_read_next (ogn_position_t *out)
{
	if (out == NULL || ogn_fifo_read_fd < 0) {
		return -1;
	}

	// keep filling the buffer until we either drain the pipe or have a full frame
	for (;;) {
		// is there room left to read more?
		if (ogn_read_buf_len < sizeof (ogn_read_buf)) {
			ssize_t r = read (ogn_fifo_read_fd,
							  ogn_read_buf + ogn_read_buf_len,
							  sizeof (ogn_read_buf) - ogn_read_buf_len);
			if (r > 0) {
				ogn_read_buf_len += (size_t)r;
			}
			else if (r == 0) {
				// no writers connected (worker not running yet) - normal
			}
			else if (errno != EAGAIN && errno != EWOULDBLOCK) {
				LOG_WARN ("ogn: FIFO read: %s", strerror (errno));
				return -1;
			}
		}

		// do we have a complete frame at the head?
		if (ogn_read_buf_len < 4) {
			return 0;
		}
		uint32_t payload_len = (uint32_t)ogn_read_buf[0] |
							   ((uint32_t)ogn_read_buf[1] << 8) |
							   ((uint32_t)ogn_read_buf[2] << 16) |
							   ((uint32_t)ogn_read_buf[3] << 24);
		if (payload_len > OGN_FRAME_MAX) {
			LOG_WARN ("ogn: bogus frame size %u, resyncing", payload_len);
			ogn_read_buf_len = 0;
			return 0;
		}
		if (ogn_read_buf_len < 4 + payload_len) {
			return 0;
		}

		Terminator12__Ogn__PositionReport *msg =
			terminator12__ogn__position_report__unpack (NULL, payload_len, ogn_read_buf + 4);

		// shift the consumed frame out regardless of unpack outcome
		size_t consumed = 4 + payload_len;
		size_t remaining = ogn_read_buf_len - consumed;
		memmove (ogn_read_buf, ogn_read_buf + consumed, remaining);
		ogn_read_buf_len = remaining;

		if (msg == NULL) {
			LOG_WARN ("ogn: protobuf unpack failed");
			continue;
		}

		memset (out, 0, sizeof (*out));
		if (msg->callsign != NULL) {
			strncpy (out->callsign, msg->callsign, OGN_CALLSIGN_MAX - 1);
		}
		out->rx_epoch_s			= msg->rx_epoch_s;
		out->msg_epoch_s		= msg->msg_epoch_s;
		out->latitude_deg		= msg->latitude_deg;
		out->longitude_deg		= msg->longitude_deg;
		out->altitude_feet		= msg->altitude_feet;
		out->ground_track_deg	= msg->ground_track_deg;
		out->ground_speed_kt	= msg->ground_speed_kt;
		out->vertical_speed_fpm	= msg->vertical_speed_fpm;
		terminator12__ogn__position_report__free_unpacked (msg, NULL);
		return 1;
	}
}

int ogn_disconnect (void)
{
	if (!ogn_worker_running) {
		return 0;
	}
	atomic_store (&ogn_shutdown_requested, 1);
	pthread_join (ogn_worker_thread, NULL);
	ogn_worker_running = false;
	atomic_store (&ogn_status, OGN_DISCONNECTED);
	return 0;
}

int ogn_deinit (void)
{
	ogn_disconnect ();
	if (ogn_fifo_write_fd >= 0) {
		close (ogn_fifo_write_fd);
		ogn_fifo_write_fd = -1;
	}
	if (ogn_fifo_read_fd >= 0) {
		close (ogn_fifo_read_fd);
		ogn_fifo_read_fd = -1;
	}
	if (ogn_fifo_path[0] != '\0') {
		(void)unlink (ogn_fifo_path);
		ogn_fifo_path[0] = '\0';
	}
	atomic_store (&ogn_status, OGN_NOT_INITIALIZED);
	return 0;
}
