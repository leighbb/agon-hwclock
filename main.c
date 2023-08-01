// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  main.c
 *
 *  Copyright (C) 2023  Leigh Brown
 */

#include <ez80.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <ERRNO.H>

#include "strings.h"
#include "i2c.h"
#include "rtc.h"

#include "mos-interface.h"

char debug;
char device;

int show_modrtc()
{
	iso8601_datetime dt;
	int res;

	if ((device == 1 ? read_modrtc(&dt) : read_modrtc2(&dt)) == -1)
		return -1;

	iso8601_display(&dt);

	return 0;
}

int show_sysrtc()
{
	iso8601_datetime dt;

	if (read_sysrtc(&dt) == -1) {
		printf("Unable to read date and time from system RTC\r\n");
		return -1;
	}

	iso8601_display(&dt);

	return 0;
}


// Set the System Clock from the Hardware Clock
int hctosys(void)
{
	iso8601_datetime dt;

	if ((device == 1 ? read_modrtc(&dt) : read_modrtc2(&dt)) == -1) {
		printf("Unable to read date and time from MOD-RTC\r\n");
		return -1;
	}

	if (write_sysrtc(&dt) == -1) {
		printf("Unable to write date and time to system\r\n");
		return -1;
	}

	return 0;
}

// Set the Hardware Clock from the System Clock
int systohc(void)
{
	iso8601_datetime dt;

	if (read_sysrtc(&dt) == -1) {
		printf("Unable to read date and time from system\r\n");
		return -1;
	}

	if ((device == 1 ? write_modrtc(&dt) : write_modrtc2(&dt)) == -1) {
		printf("Unable to write date and time to MOD-RTC\r\n");
		return -1;
	}

	return 0;
}

static int set_modrtc(const char *datestr)
{
	iso8601_datetime dt;

	if (str_to_iso8601(datestr, &dt) < 0) {
		printf("Invalid ISO8601 date and time: '%s'\r\n", datestr);
		return -1;
	}

	if ((device == 1 ? write_modrtc(&dt) : write_modrtc2(&dt)) == -1) {
		printf("Unable to write date and time to MOD-RTC\r\n");
		return -1;
	}

	return 0;
}

static int set_sysrtc(const char *datestr)
{
	iso8601_datetime dt;

	if (str_to_iso8601(datestr, &dt) < 0) {
		printf("Invalid ISO8601 date and time: '%s'\r\n", datestr);
		return -1;
	}

	if (write_sysrtc(&dt) == -1) {
		printf("Unable to write date and time to system\r\n");
		return -1;
	}

	return 0;
}

void usage(const char *prgname)
{
	printf("Usage: %s [ -debug ] [ -1 | -2 ] < command >\r\n"
	       "or     %s -help\r\n", prgname, prgname);
}

void help(const char *prgname)
{
	usage(prgname);
	printf( "\r\n"
		"\t-debug   Enable RTC debugging\r\n"
		"\r\n"
		"\t-1       Select MOD-RTC\r\n"
		"\t-2       Select MOD-RTC2\r\n"
		"\r\n"
		"\t-systohc Set the System Clock from the Hardware Clock\r\n"
		"\t-hctosys Set the Hardware Clock from the System Clock\r\n"
		"\r\n"
		"\t-showhc  Show the date and time of the Hardware Clock\r\n"
		"\t-showsys Show the date and time of the System Clock\r\n"
		"\r\n"
		"\t-sethc   Set the Hardware Clock\r\n"
		"\t-setsys  set the System Time\r\n"
		"\r\n"
		"\tExample: %s -1 -sethc 2022-04-07T08:30:00\r\n"
		"\r\n", prgname);
}

typedef enum {
	opt_nothing = 0,
	opt_systohc,
	opt_hctosys,
	opt_showhc,
	opt_showsys,
	opt_sethc,
	opt_setsys,
	opt_modrtc,
	opt_modrtc2,
	opt_help,
	opt_debug
} hwclock_opt;

typedef struct  hwclock_arg {
	char *str;
	hwclock_opt opt;
} hwclock_arg;

#define HWCLOCK_ARGS	10

static const hwclock_arg hwclock_args[HWCLOCK_ARGS] = {
	{ "-systohc",	opt_systohc },
	{ "-hctosys",	opt_hctosys },
	{ "-showhc",	opt_showhc },
	{ "-showsys",	opt_showsys },
	{ "-sethc",	opt_sethc },
	{ "-setsys",	opt_setsys },
	{ "-1",		opt_modrtc },
	{ "-2",		opt_modrtc2 },
	{ "-help",	opt_help },
	{ "-debug",	opt_debug },
};

int main(int argc, const char * argv[])
{
	int i, j;
	hwclock_opt opt;
	hwclock_opt cmd = opt_nothing;
	const char *datestr = NULL;

	debug = 0;
	device = 0;

	if (argc == 1) {
		usage(argv[0]);
		return 0;
	}

	for (i = 1; i < argc; ++i) {
		opt = opt_nothing;
		for (j = 0; j < HWCLOCK_ARGS; ++j) {
			if (strcasecmp(argv[i], hwclock_args[j].str) == 0) {
				opt = hwclock_args[j].opt;
				break;
			}
		}
		switch (opt) {
			case opt_nothing:
				printf("Unknown option: '%s'\r\n", argv[i]);
				usage(argv[0]);
				return 19;

			case opt_modrtc:
				device = 1;
				break;

			case opt_modrtc2:
				device = 2;
				break;

			case opt_systohc:
			case opt_hctosys:
			case opt_showhc:
				if (device == 0) {
					usage(argv[0]);
					return 19;
				}
				// fall-through
			case opt_showsys:
			case opt_help:
				if (cmd == opt_nothing)
					cmd = opt;
				else {
					usage(argv[0]);
					return 19;
				}
				break;

			case opt_sethc:
				if (device == 0) {
					usage(argv[0]);
					return 19;
				}
				// fall-through
			case opt_setsys:
				if (cmd == opt_nothing && argc - i > 1) {
					cmd = opt;
					++i;
					datestr = argv[i];
				}
				else {
					usage(argv[0]);
					return 19;
				}
				break;

			case opt_debug:
				++debug;
				break;
		}
	}

	switch (cmd) {
		case opt_nothing:
			usage(argv[0]);
			break;
		case opt_systohc:
			systohc();
			break;
		case opt_hctosys:
			hctosys();
			break;
		case opt_showhc:
			show_modrtc();
			break;
		case opt_showsys:
			show_sysrtc();
			break;
		case opt_sethc:
			set_modrtc(datestr);
			break;
		case opt_setsys:
			set_sysrtc(datestr);
			break;
		case opt_help:
			help(argv[0]);
			break;
		default:
			printf("Internal error\r\n");
			return 19;
	}

	return 0;
}
