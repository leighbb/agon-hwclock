// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  rtc.h
 *
 *  Copyright (C) 2023  Leigh Brown
 */

#ifndef RTC_H_
#define RTC_H_

#include "iso8601.h"

// MOS Epoch
#define EPOCH_YEAR		1980

// The size of the packet to send to the ESP32
#define MOS_RTC_WRITE_LEN	8

// MOD-RTC I2C
#define MOD_RTC_I2C_ADDR	0x51

#define MOD_RTC_REG_CTRL1	0
#define MOD_RTC_REG_CTRL2	1

#define MOD_RTC_REG_SEC		2
#define MOD_RTC_REG_MIN		3
#define MOD_RTC_REG_HOUR	4
#define MOD_RTC_REG_DAY		5
#define MOD_RTC_REG_DOW		6
#define MOD_RTC_REG_MON		7
#define MOD_RTC_REG_YEAR	8

#define MOD_RTC_REG_AL_MIN	9
#define MOD_RTC_REG_AL_HOUR	10
#define MOD_RTC_REG_AL_DAY	11
#define MOD_RTC_REG_AL_DOW	12

#define MOD_RTC_REG_CLK_CTRL	13
#define MOD_RTC_REG_TIMER_CTRL	14
#define MOD_RTC_REG_TIMER	15


// MOD-RTC2 I2C
#define MOD_RTC2_I2C_ADDR	0x68

#define MOD_RTC2_REG_SEC	0
#define MOD_RTC2_REG_MIN	1
#define MOD_RTC2_REG_HOUR	2
#define MOD_RTC2_REG_DOW	3
#define MOD_RTC2_REG_DAY	4
#define MOD_RTC2_REG_MON	5
#define MOD_RTC2_REG_YEAR	6

#define MOD_RTC2_REG_AL1_SEC	7
#define MOD_RTC2_REG_AL1_MIN	8
#define MOD_RTC2_REG_AL1_HOUR	9
#define MOD_RTC2_REG_AL1_DOWDAY	10

#define MOD_RTC2_REG_AL2_MIN	11
#define MOD_RTC2_REG_AL2_HOUR	12
#define MOD_RTC2_REG_AL2_DOWDAY	13

#define MOD_RTC2_REG_CTRL	14
#define MOD_RTC2_REG_CTRL2	15
#define MOD_RTC2_REG_AGING	16
#define MOD_RTC2_REG_TEMP_MSB	17
#define MOD_RTC2_REG_TEMP_LSB	18


int write_modrtc(iso8601_datetime *dt);
int read_modrtc(iso8601_datetime *dt);

int write_modrtc2(iso8601_datetime *dt);
int read_modrtc2(iso8601_datetime *dt);

int write_sysrtc(const iso8601_datetime *dt);
int read_sysrtc(iso8601_datetime *dt);

#endif // RTC_H_
