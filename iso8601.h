// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  iso8601.h
 *
 *  Copyright (C) 2023  Leigh Brown
 */

#ifndef ISO8601_H_
#define ISO8601_H_

#define ISO8601_DT_LEN	19

typedef struct iso8601_datetime
{
	unsigned short	year;
	unsigned char	mon;
	unsigned char	day;
	unsigned char	hour;
	unsigned char	min;
	unsigned char	sec;
} iso8601_datetime;

int dow_from_date(int y, int m, int d);
int str_to_iso8601(const char *str, iso8601_datetime *dt);
int iso8601_to_str(const iso8601_datetime *dt, char *buf, int len);
int iso8601_display(const iso8601_datetime *dt);

#endif // ISO8601_H_
