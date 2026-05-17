/*
 * ogn_aprs.c
 *
 *  Created on: May 17, 2026
 *      Author: mateusz
 */

#include "ogn_aprs.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/**
 * @brief Converts an APRS-encoded latitude (DDMM.mmH) into decimal degrees. The hemisphere
 * character is N or S. Returns false if the buffer doesn't look right.
 */
static bool ogn_aprs_parse_lat (const char *p, double *out)
{
	// e.g. "5123.45N"
	if (p[4] != '.' || (p[7] != 'N' && p[7] != 'S')) {
		return false;
	}
	char	deg_buf[3] = {p[0], p[1], '\0'};
	char	min_buf[6] = {p[2], p[3], p[4], p[5], p[6], '\0'};
	double	deg = (double)atoi (deg_buf);
	double	min = atof (min_buf);
	double	val = deg + (min / 60.0);
	if (p[7] == 'S') {
		val = -val;
	}
	*out = val;
	return true;
}

/**
 * @brief Same as @link{ogn_aprs_parse_lat} for longitude (DDDMM.mmH, hemisphere E or W).
 */
static bool ogn_aprs_parse_lon (const char *p, double *out)
{
	// e.g. "00123.45E"
	if (p[5] != '.' || (p[8] != 'E' && p[8] != 'W')) {
		return false;
	}
	char	deg_buf[4] = {p[0], p[1], p[2], '\0'};
	char	min_buf[6] = {p[3], p[4], p[5], p[6], p[7], '\0'};
	double	deg = (double)atoi (deg_buf);
	double	min = atof (min_buf);
	double	val = deg + (min / 60.0);
	if (p[8] == 'W') {
		val = -val;
	}
	*out = val;
	return true;
}

/**
 * @brief Combines today's UTC date with the HHMMSS field carried by APRS to produce an epoch.
 */
static uint64_t ogn_aprs_make_epoch (uint8_t hh, uint8_t mm, uint8_t ss, time_t now)
{
	struct tm tm_utc;
	gmtime_r (&now, &tm_utc);
	tm_utc.tm_hour = hh;
	tm_utc.tm_min	= mm;
	tm_utc.tm_sec	= ss;
	// timegm() interprets the struct as UTC; on glibc we have it directly.
	return (uint64_t)timegm (&tm_utc);
}

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

bool ogn_aprs_parse_line (const char *line, size_t line_len, ogn_position_t *out)
{
	if (line == NULL || out == NULL || line_len == 0) {
		return false;
	}

	// comment / heartbeat lines (server keepalive, login response, etc.)
	if (line[0] == '#') {
		return false;
	}

	// callsign ends at the first '>'
	const char *gt = memchr (line, '>', line_len);
	if (gt == NULL) {
		return false;
	}
	size_t cs_len = (size_t)(gt - line);
	if (cs_len == 0 || cs_len >= OGN_CALLSIGN_MAX) {
		return false;
	}

	// payload starts after ':'
	const char *colon = memchr (gt, ':', line_len - (size_t)(gt - line));
	if (colon == NULL) {
		return false;
	}
	const char *payload = colon + 1;
	size_t		 payload_len = line_len - (size_t)(payload - line);

	// expected position-report layout:
	//   /HHMMSSh<lat>/<lon><sym><track>/<speed>/A=<altitude>
	// minimum size: 1 + 6 + 1 + 8 + 1 + 9 + 1 + 3 + 1 + 3 + 3 + 6 = 43 chars
	if (payload_len < 43 || payload[0] != '/' || payload[7] != 'h') {
		return false;
	}

	// HHMMSS
	for (int i = 1; i <= 6; i++) {
		if (!isdigit ((unsigned char)payload[i])) {
			return false;
		}
	}
	uint8_t hh = (uint8_t)((payload[1] - '0') * 10 + (payload[2] - '0'));
	uint8_t mm = (uint8_t)((payload[3] - '0') * 10 + (payload[4] - '0'));
	uint8_t ss = (uint8_t)((payload[5] - '0') * 10 + (payload[6] - '0'));
	if (hh > 23 || mm > 59 || ss > 59) {
		return false;
	}

	// lat starts at offset 8, separator '/' at 16, lon starts at 17, sym at 26
	double lat_deg = 0.0, lon_deg = 0.0;
	if (!ogn_aprs_parse_lat (payload + 8, &lat_deg)) {
		return false;
	}
	if (payload[16] != '/') {
		return false;
	}
	if (!ogn_aprs_parse_lon (payload + 17, &lon_deg)) {
		return false;
	}

	// after the symbol char at offset 26, optional CSE/SPD then "/A=NNNNNN"
	// keep things forgiving: scan the remainder for /A= and for trk/spd via sscanf.
	uint32_t track = 0, speed = 0;
	int32_t	 altitude = 0;
	int32_t	 vs_fpm = 0;

	const char *rest = payload + 27;
	size_t		 rest_len = (payload_len > 27) ? payload_len - 27 : 0;

	// Track / speed: sscanf only consumes up to whitespace, safe on bounded payload
	(void)sscanf (rest, "%3u/%3u", &track, &speed);

	const char *alt_marker = memmem (rest, rest_len, "/A=", 3);
	if (alt_marker != NULL) {
		(void)sscanf (alt_marker + 3, "%6d", &altitude);
	}

	const char *fpm_marker = memmem (rest, rest_len, "fpm", 3);
	if (fpm_marker != NULL && (size_t)(fpm_marker - rest) >= 4) {
		// step back to the start of the +/-NNN token
		const char *q = fpm_marker - 1;
		while (q > rest && (isdigit ((unsigned char)*q) || *q == '+' || *q == '-')) {
			q--;
		}
		(void)sscanf (q + 1, "%d", &vs_fpm);
	}

	// populate the out struct only once everything parsed
	memset (out, 0, sizeof (*out));
	memcpy (out->callsign, line, cs_len);
	out->callsign[cs_len] = '\0';

	time_t now = time (NULL);
	out->rx_epoch_s = (uint64_t)now;
	out->msg_epoch_s = ogn_aprs_make_epoch (hh, mm, ss, now);
	out->latitude_deg = lat_deg;
	out->longitude_deg = lon_deg;
	out->altitude_feet = altitude;
	out->ground_track_deg = track % 360;
	out->ground_speed_kt = speed;
	out->vertical_speed_fpm = vs_fpm;

	return true;
}
