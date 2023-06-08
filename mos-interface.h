/*
 * Title:			AGON MOS - MOS c header interface
 * Author:			Jeroen Venema
 * Created:			15/10/2022
 * Last Updated:	15/10/2022
 * 
 * Modinfo:
 * 15/10/2022:		Added putch, getch
 * 22/10/2022:		Added waitvblank, mos_f* functions
 */

#ifndef MOS_H
#define MOS_H

#include <defines.h>

// File access modes - from mos_api.inc
#define fa_read				0x01
#define fa_write			0x02
#define fa_open_existing	0x00
#define fa_create_new		0x04
#define fa_create_always	0x08
#define fa_open_always		0x10
#define fa_open_append		0x30

// Indexes into sysvar - from mos_api.inc
#define sysvar_time			0x00
#define sysvar_vpd_pflags	0x04
#define sysvar_keycode		0x05
#define sysvar_keymods		0x06
#define sysvar_cursorX		0x07
#define sysvar_cursorY		0x08
#define sysvar_scrchar		0x09
#define sysvar_scrpixel		0x0A
#define sysvar_audioChannel	0x0D
#define syscar_audioSuccess	0x0E

#define VDDP_FLAG_CURSOR	(1 << 0)
#define VDDP_FLAG_SCRCHAR	(1 << 1)
#define VDDP_FLAG_POINT		(1 << 2)
#define VDDP_FLAG_AUDIO		(1 << 3)
#define VDDP_FLAG_MODE		(1 << 4)
#define VDDP_FLAG_RTC		(1 << 5)

extern int putch(int a);
extern char getch(void);
extern void waitvblank(void);

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

extern int mlt_test(int v);

extern unsigned int mlt_16_8(unsigned short a, unsigned char b);
extern unsigned int mlt_16_8_2(unsigned short a, unsigned char b);

#endif MOS_H
