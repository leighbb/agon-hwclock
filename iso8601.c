// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  iso8601.c
 *
 *  Copyright (C) 2023  Leigh Brown
 */

#include <ez80.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "iso8601.h"

static int validate_iso8601(const char *str)
{
	// Format (case-insensitive):
	// YYYY '-' MM '-' DD 'T' hh ':' mm ':' ss

	static const char validation[ISO8601_DT_LEN] =
	{ 'd', 'd', 'd', 'd', '-', 'd', 'd', '-', 'd', 'd',
	  't',      'd', 'd', ':', 'd', 'd', ':', 'd', 'd' };
	int i;

	if (strlen(str) != ISO8601_DT_LEN)
		return -1;

	for (i = 0; i < ISO8601_DT_LEN; ++i) {
		char c = validation[i];
		if (c == 'd') {
			if (!isdigit(str[i]))
				return -1;
		}
		else {
			if (tolower(str[i]) != c)
				return -1;
		}
	}

	return 0;
}

static int extract_digits(const char *str, int len)
{
	int result = 0;
	int i;

	for (i = 0; i < len; ++i) {
		result *= 10;
		result += str[i] - '0';
	}

	return result;
}

int str_to_iso8601(const char *str, iso8601_datetime *dt)
{
	/*
	 * Format:
	 *	YYYY '-' MM '-' DD 'T' hh ':' mm ':' ss
	 */
	if (validate_iso8601(str) < 0) {
		printf("VALIDATION FAILED\r\n");
		return -1;
	}

	dt->year = extract_digits(&str[0],  4);
	dt->mon  = extract_digits(&str[5],  2);
	dt->day  = extract_digits(&str[8],  2);
	dt->hour = extract_digits(&str[11], 2);
	dt->min  = extract_digits(&str[14], 2);
	dt->sec  = extract_digits(&str[17], 2);

	return 0;
}

int iso8601_to_str(const iso8601_datetime *dt, char *buf, int len)
{
	/*
	 * Format:
	 *	YYYY '-' MM '-' DD 'T' hh ':' mm ':' ss
	 */
	if (len < ISO8601_DT_LEN + 1)
		return -1;

	sprintf(buf, "%04d-%02d-%02dT%02d:%02d:%02d",
		dt->year, dt->mon, dt->day,
		dt->hour, dt->min, dt->sec);

	return 0;
}

int iso8601_display(const iso8601_datetime *dt)
{
	char buf[ISO8601_DT_LEN + 1];

	if (iso8601_to_str(dt, buf, sizeof buf) == -1)
		return -1;

	printf("%s\r\n", buf);
	return 0;
}
