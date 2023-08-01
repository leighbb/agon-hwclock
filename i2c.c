// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  i2c.c
 *
 *  Copyright (C) 2023  Leigh Brown
 */

#include <stdio.h>
#include <ez80.h>

#include "i2c.h"

extern char debug;

static unsigned char i2c_state;

int
i2c_init(int target_addr, char general_call)
{
	unsigned char v, i;

	// Target address is either a 7-bit or 10-bit value
	// TODO: support 10-bit target addresses
	if (target_addr < I2C_MIN_TARGET_ADDR ||
	    target_addr > I2C_MAX_TARGET_7BIT)
		return -1;

	// Disable I2C
	I2C_CTL = 0x00;

	// Perform software reset of I2C
	I2C_SRR = 0xff;

	// Give some time for that to happen
	for (i = 0; i < 100; ++i)
		;

	// Set I2C clock divider (M) to 4 and exponent (N) to 0
	// fSAMP =  18.432MHz		fSCLK / 2 ^ N
	// fSCL  = 368.684kHz		fSCLK / (10 * (M + 1) * 2 ^ N) 
	// NB: This assumes 18.432MHz system clock
	I2C_CCR = 4 << 3 | 0;

	// Set the I2C_SAR and I2C_XSAR registers depending on target address
	v = (unsigned char)target_addr << 1;
	if (general_call)
		v |= 1;
	I2C_SAR = v;
	I2C_XSAR = 0;

	// Enable I2C, leave all other bits unset
	I2C_CTL = I2C_CTL_ENAB;

	// Set the status accordingly
	i2c_state = I2C_ST_IDLE;

	return I2C_OK;
}

void
i2c_ctrl_start(void)
{
	unsigned char ctl;

	// Set START condition and clear any IFLG
	ctl  = I2C_CTL;
	ctl |= I2C_CTL_STA;
	ctl &= ~I2C_CTL_IFLG;
	I2C_CTL = ctl;

	i2c_state = I2C_ST_CTRL_START_SENT;
	if (debug)
		printf("[S]");
}

void
i2c_ctrl_stop(unsigned char wait)
{
	unsigned char ctl;
	unsigned char ctl_before, ctl_after;
	unsigned char sr_before, sr_after, dr_before, dr_after;

	// Set STOP condition and clear any IFLG
	sr_before = I2C_SR;
	dr_before = I2C_DR;
	ctl_before = I2C_CTL;
	if (I2C_CTL & I2C_CTL_IFLG)
		I2C_CTL &= ~I2C_CTL_IFLG;
	I2C_CTL |= I2C_CTL_STP;
	ctl_after = I2C_CTL;
	dr_after = I2C_DR;
	sr_after = I2C_SR;

	i2c_state = I2C_ST_CTRL_STOP_SENT;
	if (debug)
		printf("[P]\r\n");

	if (wait) {
		// Wait for STOP condition to complete then clear IFLG
		while (!(I2C_CTL & I2C_CTL_IFLG))
			;
		I2C_CTL &= ~I2C_CTL_IFLG;
	}
}

unsigned char
i2c_ctrl_send_addr(int target_addr, char mode)
{
	unsigned char ctl, sr, b;

	if (target_addr < I2C_MIN_TARGET_ADDR ||
	    target_addr > I2C_MAX_TARGET_7BIT)
		return I2C_ERR_INVALID_TARGET_ADDR;

	if (i2c_state != I2C_ST_CTRL_START_SENT)
		return I2C_ERR_INVALID_STATE;

	b = (target_addr & 0x7f) << 1;
	if (mode != I2C_TARGET_WRITE)
		b |= 1;

	while (!(I2C_CTL & I2C_CTL_IFLG))
		;

	sr = I2C_SR;
	if (!(sr == I2C_START || sr == I2C_REP_START)) {
		if (debug)
			printf("<%02x!=%02x/%02x>", sr, I2C_START, I2C_REP_START);
		return sr;
	}

	// Set data register to byte to send then clear IFLG
	I2C_DR = b;
	I2C_CTL &= ~ I2C_CTL_IFLG;

	i2c_state = I2C_ST_CTRL_TARG_SENT;
	if (debug)
		printf("[%02x->]", b);
	return I2C_OK;
}

unsigned char
i2c_ctrl_send_data(unsigned char b)
{
	unsigned char ctl, sr;

	if (!(i2c_state == I2C_ST_CTRL_TARG_SENT ||
	      i2c_state == I2C_ST_CTRL_DATA_SENT))
		return I2C_ERR_INVALID_STATE;

	while (!(I2C_CTL & I2C_CTL_IFLG))
		;

	sr = I2C_SR;
	// XXX: If we get NACK then do not send the byte
	if (!(sr == I2C_CT_TARG_ACK ||
	      sr == I2C_CT_DATA_ACK)) {
		if (debug)
			printf("[%02x!=%02x/%02x]", sr, I2C_CT_TARG_ACK, I2C_CT_DATA_ACK);
		return sr;
	}

	// Set data register to byte to send then clear IFLG
	I2C_DR = b;
	I2C_CTL &= ~I2C_CTL_IFLG;

	i2c_state = I2C_ST_CTRL_DATA_SENT;
	if (debug)
		printf("[%02x=>]", b);
	return I2C_OK;
}

unsigned char
i2c_ctrl_req_byte(char last)
{
	int ctl, sr, i;

	if (i2c_state != I2C_ST_CTRL_TARG_SENT) {
		if (debug)
			printf("{%02x!=%02x}", i2c_state, I2C_ST_CTRL_TARG_SENT);
		return I2C_ERR_INVALID_STATE;
	}

	// Wait for I2C interrupt flag to be set to indicate target address
	// has been sent (or error)
	while (!(I2C_CTL & I2C_CTL_IFLG))
		;

	// Check status register
	sr = I2C_SR;
	if (sr != I2C_CR_TARG_ACK) {
		if (debug)
			printf("<%02x!=%02x>", sr, I2C_CR_TARG_ACK);
		return sr;
	}

	// Set ACK/NACK depending on whether requesting last byte, then
	// clear IFLG bit
	ctl = I2C_CTL;
	if (last)
		ctl &= ~I2C_CTL_AAK;
	else
		ctl |= I2C_CTL_AAK;
	ctl &= ~I2C_CTL_IFLG;
	I2C_CTL = ctl;

	i2c_state = I2C_ST_CTRL_DATA_REQD;
	if (debug)
		printf("[-]");
	return I2C_OK;
}

unsigned char
i2c_ctrl_recv_byte(unsigned char *data, char last)
{
	int ctl, sr, i;

	if (i2c_state != I2C_ST_CTRL_DATA_REQD)
		return I2C_ERR_INVALID_STATE;

	// Wait for I2C interrupt flag to be set
	while (!(I2C_CTL & I2C_CTL_IFLG))
		;

	// Check status register
	sr = I2C_SR;
	if (sr == I2C_CR_DATA_ACK || sr == I2C_CR_DATA_NACK) {
		unsigned char b = I2C_DR;
		if (debug)
			printf("[<=%02x]", b);
		*data = b;
	}
	else
		if (debug)
			printf("<!%02x>", sr);
	
	// Set ACK/NACK depending on whether requesting last byte, then
	// clear IFLG bit (not relevant for last byte but doesn't harm)
	ctl = I2C_CTL;
	if (last)
		ctl &= ~I2C_CTL_AAK;
	else
		ctl |= I2C_CTL_AAK;
	ctl &= ~I2C_CTL_IFLG;
	I2C_CTL = ctl;

	// no change in i2c_state
	if (sr == I2C_CR_DATA_ACK)
		return I2C_OK;
	else
		return sr;
}

/*
 * i2c_ctrl_write - write some bytes to an I2C target
 */

int
i2c_ctrl_write(int target_addr, unsigned char *buf, unsigned int len)
{
	unsigned char sr;
	int i;

	// TODO: support 10-bit addresses
	if (target_addr < I2C_MIN_TARGET_ADDR ||
	    target_addr > I2C_MAX_TARGET_7BIT)
		return -1;

	i2c_ctrl_start();

	sr = i2c_ctrl_send_addr(target_addr, I2C_TARGET_WRITE);
	if (sr != I2C_OK)
		return -sr;
	
	i = 0;
	while (i < len) {
		sr = i2c_ctrl_send_data(buf[i]);
		if (sr != I2C_OK)
			break;

		++i;
	}

	if (sr == I2C_CT_DATA_NACK)
		return i;
	else if (sr != I2C_OK)
		return -sr;

	// Wait for last data byte to transfer
	while (!(I2C_CTL & I2C_CTL_IFLG))
		;

	sr = I2C_SR;
	// XXX: disabled as did not work: I2C_CTL &= ~I2C_CTL_IFLG;
	if (sr == I2C_CT_DATA_ACK || sr == I2C_CT_DATA_NACK)
		return i+1;
	else {
		if (debug)
			printf("<%02x!=%02x/%02x>", sr, I2C_CT_DATA_ACK,
							I2C_CT_DATA_NACK);
		return -sr;
	}
}

/*
 * i2c_ctrl_read - read some bytes from an I2C target
 * TODO: Does not support 10-bit target addresses currently
 */

int
i2c_ctrl_read(int target_addr, unsigned char *buf, unsigned int len)
{
	unsigned char sr;
	int i;

	// TODO: support 10-bit addresses
	if (target_addr < I2C_MIN_TARGET_ADDR ||
	    target_addr > I2C_MAX_TARGET_7BIT)
		return -1;

	i2c_ctrl_start();

	sr = i2c_ctrl_send_addr(target_addr, I2C_TARGET_READ);
	if (sr != I2C_OK)
		return -(int)sr;

	// Request first byte
	i = 0;
	sr = i2c_ctrl_req_byte(i == (len - 1) ? 1 : 0);
	if (sr != I2C_OK)
		return -(int)sr;

	// Receive bytes
	while (i < len) {
		sr = i2c_ctrl_recv_byte(&buf[i], i == (len - 1) ? 1 : 0);
		++i;
		if (sr != I2C_OK)
			break;
	}

	if (sr == I2C_CR_DATA_NACK || sr == I2C_OK)
		return i;
	else
		return -(int)sr;
}
