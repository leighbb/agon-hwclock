// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  rtc.c
 *
 *  Copyright (C) 2023  Leigh Brown
 */

#include <ez80.h>
#include <stdio.h>

#include "i2c.h"
#include "bcd.h"
#include "rtc.h"
#include "mos-interface.h"

/*
static const char *WEEKDAYS[7] =
	{ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
*/

static int isnum(char c)
{
	return c >= '0' && c <= '9';
}

static int dow_from_date(int y, int m, int d)
{
	static const char t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
	y -= m < 3;
	return (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
}

int write_modrtc(iso8601_datetime *dt)
{
	unsigned char buffer[8];
	int wrote;

	buffer[1] = binary_to_bcd(dt->sec);
	buffer[2] = binary_to_bcd(dt->min);
	buffer[3] = binary_to_bcd(dt->hour);
	buffer[4] = binary_to_bcd(dt->day);
	buffer[5] = dow_from_date(dt->year, dt->mon, dt->day);
	buffer[6] = binary_to_bcd(dt->mon);
	buffer[7] = binary_to_bcd(dt->year % 100);

	// Initialise I2C
	i2c_init(0, 0);

	// Set address pointer, the rest of the buffer are the registers
	// values.
	buffer[0] = MOD_RTC_REG_SEC;
	wrote = i2c_ctrl_write(MOD_RTC_I2C_ADDR, buffer, sizeof buffer);
	if (wrote < sizeof buffer) {
		printf("Unable to communicate with MOD-RTC (%d)\r\n", wrote);
		i2c_ctrl_stop(1);
		return 1;
	}

	i2c_ctrl_stop(1);

	return 0;
}

int read_modrtc(iso8601_datetime *dt)
{
	unsigned char addrptr;
	unsigned char buffer[7];
	int wrote, got;

	// Initialise I2C
	i2c_init(0, 0);

	// Set address pointer to "2"
	addrptr = 2;
	wrote = i2c_ctrl_write(MOD_RTC_I2C_ADDR, &addrptr, 1);
	if (wrote < 1) {
		printf("Unable to communicate with MOD-RTC (%d)\r\n", wrote);
		i2c_ctrl_stop(1);
		return 1;
	}

	// Read from registers
	got = i2c_ctrl_read(MOD_RTC_I2C_ADDR, &buffer, sizeof buffer);

	// Set STOP condition to release I2C bus
	i2c_ctrl_stop(1);

	// Exit if didn't get what we needed
	if (got < sizeof buffer) {
		printf("Unable to read time from MOD-RTC (%d)\r\n", got);
		return 1;
	}

	// Convert register values into ISO 8601 date-time structure
	dt->sec  = bcd_to_binary(buffer[0] & 0x7f);
	dt->min  = bcd_to_binary(buffer[1] & 0x7f);
	dt->hour = bcd_to_binary(buffer[2] & 0x3f);
	dt->day  = bcd_to_binary(buffer[3] & 0x3f);
	dt->mon  = bcd_to_binary(buffer[5] & 0x1f);
	// (TODO: use century bit .. buffer[3] & 0x80
	dt->year = 2000 + bcd_to_binary(buffer[6]);
	return 0;
}

int write_modrtc2(iso8601_datetime *dt)
{
	unsigned char buffer[8];
	int wrote;

	buffer[1] = binary_to_bcd(dt->sec);
	buffer[2] = binary_to_bcd(dt->min);
	buffer[3] = binary_to_bcd(dt->hour);
	buffer[4] = dow_from_date(dt->year, dt->mon, dt->day);
	buffer[5] = binary_to_bcd(dt->day);
	buffer[6] = binary_to_bcd(dt->mon);
	buffer[7] = binary_to_bcd(dt->year % 100);

	// Initialise I2C
	i2c_init(0, 0);

	// Set address pointer to "0", the rest of the buffer are the registers
	// values.
	buffer[0] = 0;
	wrote = i2c_ctrl_write(MOD_RTC2_I2C_ADDR, buffer, sizeof buffer);
	if (wrote < sizeof buffer) {
		printf("Unable to communicate with MOD-RTC2 (%d)\r\n", wrote);
		i2c_ctrl_stop(1);
		return 1;
	}

	i2c_ctrl_stop(1);

	return 0;
}

int read_modrtc2(iso8601_datetime *dt)
{
	unsigned char addrptr;
	unsigned char buffer[7];
	int wrote, got;

	// Initialise I2C
	i2c_init(0, 0);

	// Set address pointer to "0"
	addrptr = 0;
	wrote = i2c_ctrl_write(MOD_RTC2_I2C_ADDR, &addrptr, 1);
	if (wrote < 1) {
		printf("Unable to communicate with MOD-RTC2 (%d)\r\n", wrote);
		i2c_ctrl_stop(1);
		return 1;
	}

	// Read from registers
	got = i2c_ctrl_read(MOD_RTC2_I2C_ADDR, &buffer, sizeof buffer);

	// Set STOP condition to release I2C bus
	i2c_ctrl_stop(1);

	// Exit if didn't get what we needed
	if (got < sizeof buffer) {
		printf("Unable to read time from MOD-RTC2 (%d)\r\n", got);
		return 1;
	}

	// Convert register values into ISO 8601 date-time structure
	dt->sec  = bcd_to_binary(buffer[0] & 0x7f);
	dt->min  = bcd_to_binary(buffer[1] & 0x7f);
	dt->hour = bcd_to_binary(buffer[2] & 0x3f);
	dt->day  = bcd_to_binary(buffer[4] & 0x3f);
	dt->mon  = bcd_to_binary(buffer[5] & 0x1f);
	// (TODO: use century bit .. buffer[5] & 0x80
	dt->year = 2000 + bcd_to_binary(buffer[6]);
	return 0;
}

int read_sysrtc(iso8601_datetime *dt)
{
	const char vdp_rtc[4] = { 23, 0, VDP_rtc, 0 };
	struct mos_sysvars *sysvars;

	// There is no MOS system call to retrieve
	// the RTC data in structured form. So, we
	// have to manually access the variables
	// ourselves.
	
	// Get pointer to system variables
	sysvars = mos_sysvars();
	
	// Reset protocol flag for RTC data received
	sysvars->vdp_protocol_flags &= ~ VDPP_FLAG_RTC;

	// Send VDP sequence to request ESP32 RTC data
	mos_write(vdp_rtc, sizeof vdp_rtc);
	
	// Wait for response packet with ESP32 RTC data
	// TODO: timeout?
	while (!(sysvars->vdp_protocol_flags & VDPP_FLAG_RTC))
		;

	// Copy the fields into our datetime structure
	dt->sec  = sysvars->time.second;
	dt->min  = sysvars->time.minute;
	dt->hour = sysvars->time.hour;
	dt->day  = sysvars->time.day;
	dt->mon  = sysvars->time.month + 1; // TRAP: When reading it's 0-11
	dt->year = sysvars->time.year + EPOCH_YEAR;

	return 0;
}

int write_sysrtc(const iso8601_datetime *dt)
{
	unsigned char mosrtc[MOS_RTC_WRITE_LEN];

	mosrtc[0] = dt->year - EPOCH_YEAR;
	mosrtc[1] = dt->mon;
	mosrtc[2] = dt->day; // TRAP: When writing it's 1-12
	mosrtc[3] = dt->hour;
	mosrtc[4] = dt->min;
	mosrtc[5] = dt->sec;

	mos_setrtc(mosrtc);

	return 0;
}

