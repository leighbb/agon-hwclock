// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  bcd.c
 *
 *  Copyright (C) 2023  Leigh Brown
 */

int binary_to_bcd(int num)
{
	int bcd = 0;
	int shift = 0;

	while (num > 0) {
		bcd |= num % 10 << shift;
		shift += 4;
		num /= 10;
	}

	return bcd;
}

int bcd_to_binary(int bcd)
{
	int binary = 0;
	int multiplier = 1;

	while (bcd > 0) {
		binary += (bcd & 15) * multiplier;
		multiplier *= 10;
		bcd >>= 4;
	}

	return binary;
}
