// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  i2c.h
 *
 *  Copyright (C) 2023  Leigh Brown
 */

#ifndef I2C_H_
#define I2C_H_

// Target address range
#define	I2C_MIN_TARGET_ADDR	0
#define I2C_MAX_TARGET_7BIT	((1<<7)-1)
#define I2C_MAX_TARGET_ADDR	((1<<10)-1)

// eZ80F92 I2C_CTL register bits
#define I2C_CTL_IEN	(1 << 7)
#define I2C_CTL_ENAB	(1 << 6)
#define I2C_CTL_STA	(1 << 5)
#define I2C_CTL_STP	(1 << 4)
#define I2C_CTL_IFLG	(1 << 3)
#define I2C_CTL_AAK	(1 << 2)

// I2C Target address mode
#define I2C_TARGET_READ 0
#define I2C_TARGET_WRITE 1

// I2C interface status
#define I2C_BUS_ERROR			0x00
#define I2C_START			0x08
#define I2C_REP_START			0x10
#define I2C_CT_TARG_ACK			0x18
#define I2C_CT_TARG_NACK		0x20
#define I2C_CT_DATA_ACK			0x28
#define I2C_CT_DATA_NACK		0x30
#define I2C_CT_ARB_LOST			0x38
#define I2C_CR_ARB_LOST			0x38
#define I2C_CR_TARG_ACK			0x40
#define I2C_CR_TARG_NACK		0x48
#define I2C_CR_DATA_ACK			0x50
#define I2C_CR_DATA_NACK		0x58
#define I2C_TR_TARG_ACK			0x60
#define I2C_TR_ARB_LOST_TARG_ACK	0x68
#define I2C_TR_GCALL_ACK		0x70
#define I2C_TR_ARB_LOST_GCALL_ACK	0x78
#define I2C_TR_DATA_ACK			0x80
#define I2C_TR_DATA_NACK		0x88
#define I2C_TR_GCALL_DATA_ACK		0x90
#define I2C_TR_GCALL_DATA_NACK		0x98
#define I2C_TR_STOP			0xA0
#define I2C_TT_TARG_ACK			0xA8
#define I2C_TT_ARB_LOST_TARG_ACK	0xB0
#define I2C_TT_DATA_ACK			0xB8
#define I2C_TT_DATA_NACK		0xC0
#define I2C_TT_LAST_DATA		0xC8
#define I2C_CT_2ND_ACK			0xD0
#define I2C_CT_2ND_NACK			0xD8
#define I2C_NO_INFO			0xF8

// I2C software status
#define I2C_ST_IDLE			0
#define I2C_ST_CTRL_START_SENT		1
#define I2C_ST_CTRL_STOP_SENT		2
#define I2C_ST_CTRL_TARG_SENT		3
#define I2C_ST_CTRL_DATA_SENT		4
#define I2C_ST_CTRL_DATA_REQD		5

// I2C errors
#define I2C_OK				0
#define I2C_ERR_INVALID_STATE		1
#define I2C_ERR_INVALID_TARGET_ADDR	2

int i2c_init(int target_addr, char general_call);
void i2c_ctrl_start();
void i2c_ctrl_stop(unsigned char wait);
unsigned char i2c_ctrl_send_target_addr(int target_addr, char mode);
unsigned char i2c_ctrl_recv_byte(unsigned char *data, char last);
unsigned char i2c_ctrl_send_byte(unsigned char b);
int i2c_ctrl_write(int target_addr, unsigned char *buf, unsigned int len);
int i2c_ctrl_read(int target_addr, unsigned char *buf, unsigned int len);

#endif // I2C_H_

