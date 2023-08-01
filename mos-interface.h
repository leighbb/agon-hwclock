// SPDX-License-Identifier: MIT
/*
 *  mos-interface.h
 *
 *  Copyright (C) 2023  Various
 */

#ifndef MOS_H
#define MOS_H

#include <defines.h>

// File access modes - from mos_api.inc
#define fa_read			0x01
#define fa_write		0x02
#define fa_open_existing	0x00
#define fa_create_new		0x04
#define fa_create_always	0x08
#define fa_open_always		0x10
#define fa_open_append		0x30

// Indexes into sysvar - from mos_api.inc
#define sysvar_time		0x00
#define sysvar_vpd_pflags	0x04
#define sysvar_keycode		0x05
#define sysvar_keymods		0x06
#define sysvar_cursorX		0x07
#define sysvar_cursorY		0x08
#define sysvar_scrchar		0x09
#define sysvar_scrpixel		0x0A
#define sysvar_audioChannel	0x0D
#define sysvar_audioSuccess	0x0E

// VDP specific (for VDU 23,0,n commands)
#define VDP_gp			0x80
#define VDP_keycode		0x81
#define VDP_cursor		0x82
#define VDP_scrchar		0x83
#define VDP_scrpixel		0x84
#define VDP_audio		0x85
#define VDP_mode		0x86
#define VDP_rtc			0x87
#define VDP_keystate		0x88
#define VDP_logicalcoords	0xC0
#define VDP_terminalmode	0xFF

#define VDPP_FLAG_CURSOR	(1 << 0)
#define VDPP_FLAG_SCRCHAR	(1 << 1)
#define VDPP_FLAG_POINT		(1 << 2)
#define VDPP_FLAG_AUDIO		(1 << 3)
#define VDPP_FLAG_MODE		(1 << 4)
#define VDPP_FLAG_RTC		(1 << 5)

typedef struct mos_sysvars {
	unsigned long   clock;
	unsigned char   vdp_protocol_flags;
	unsigned char   keyascii;
	unsigned char   keymods;
	unsigned char   cursorX;
	unsigned char   cursorY;
	unsigned char   scrchar;
	unsigned int    scrpixel;
	unsigned char   audioChannel;
	unsigned char   audioSuccess;
	unsigned short  scrwidth;
	unsigned short  scrheight;
	unsigned char   scrcols;
	unsigned char   scrrows;
	unsigned char   scrcolours;
	unsigned char   scrPixelIndex;
	unsigned char   keycode;
	unsigned char   keydown;
	unsigned char   keycount;
	struct {
		unsigned char   year;
		unsigned char   month;
		unsigned char   day;
		unsigned char   dayOfYear;
		unsigned char   dayOfWeek;
		unsigned char   hour;
		unsigned char   minute;
		unsigned char   second;
	} time;
	unsigned short  keydelay;
	unsigned short  keyrate;
	unsigned char   keyled;
	unsigned char   scrmode;
	int		errno;
	char		coldBoot;
	char		gp;
};

extern int putch(int a);
extern char getch(void);
extern void mos_write(const char *buf, unsigned int len);
extern void waitvblank(void);

extern struct mos_sysvars *mos_sysvars(void);
extern DWORD getsysvar_time(void);
extern int getsysvar_cursorX(void);
extern int getsysvar_cursorY(void);
extern int getsysvar_scrwidth(void);
extern int getsysvar_scrheight(void);

extern UINT8 getsysvar8bit(UINT8 sysvar);
extern UINT16 getsysvar16bit(UINT8 sysvar);
extern UINT24 getsysvar24bit(UINT8 sysvar);


extern UINT8 mos_fopen(char * filename, UINT8 mode); // returns filehandle, or 0 on error
extern UINT8 mos_fclose(UINT8 fh);					 // returns number of still open files
extern char	 mos_fgetc(UINT8 fh);					 // returns character from file
extern UINT24 mos_fread(UINT8 fh, char *buffer, UINT24 btr);
extern UINT24 mos_fwrite(UINT8 fh, char *buffer, UINT24 btw);
extern UINT8 mos_flseek(UINT8 fh, DWORD offset);
extern UINT8 mos_ren(char *filename1, char *filename2);
extern UINT8 mos_del(char *filename);
extern void	 mos_fputc(UINT8 fh, char c);			 // writes character to file
extern UINT8 mos_feof(UINT8 fh);					 // returns 1 if EOF, 0 otherwise

extern void mos_setrtc(const unsigned char *rtcbuf);
extern int mos_getrtc(unsigned char *rtcbuf);

#endif MOS_H
