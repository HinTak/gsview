/* Copyright (C) 1993-1997, Russell Lang.  All rights reserved.
  
  This file is part of GSview.
  
  This program is distributed with NO WARRANTY OF ANY KIND.  No author
  or distributor accepts any responsibility for the consequences of using it,
  or for whether it serves any particular purpose or works at all, unless he
  or she says so in writing.  Refer to the GSview Free Public Licence 
  (the "Licence") for full details.
  
  Every copy of GSview must include a copy of the Licence, normally in a 
  plain ASCII text file named LICENCE.  The Licence grants you the right 
  to copy, modify and redistribute GSview, but only under certain conditions 
  described in the Licence.  Among other things, the Licence requires that 
  the copyright notice and this notice be preserved on all copies.
*/

/* epstool.h */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#ifdef __EMX__
#define GSCOMMAND "gsos2"
#define BYTE unsigned char	/* 8 bits unsigned */
#define WORD  unsigned short	/* 16 bits unsigned */
#define DWORD unsigned int	/* 32 bits unsigned */
#define UINT  unsigned int	/* native unsigned */
#define LONG  int		/* 32 bits signed */
#define BOOL  int
#define GVFAR 
#define GVHUGE 
#define READBIN  "rb"
#define WRITEBIN "wb"
#define MAXSTR 256
#define DIRSEP '\\'
#define EOLSTR "\r\n"
#include <io.h>
#include <fcntl.h>
#endif

#ifdef __WIN32__
#undef MSDOS
#include <windows.h>
#define SetWindowOrg(hdc, x, y) SetWindowOrgEx(hdc, x, y, (LPPOINT)NULL)
#define	SetWindowExt(hdc, x, y) SetWindowExtEx(hdc, x, y, (LPSIZE)NULL)
#define GSCOMMAND "gswin32c"
#define GVFAR
#define GVHUGE
#define BYTE unsigned char
#define WORD  unsigned short
#define DWORD unsigned long
#define UINT  unsigned int
#define LONG  long
#define BOOL  int
#define READBIN  "rb"
#define WRITEBIN "wb"
#define MAXSTR 256
#define DIRSEP '\\'
#define EOLSTR "\r\n"
#include <alloc.h>
#include <dir.h>
#include <io.h>
#include <fcntl.h>
#endif

#ifdef MSDOS
#define GSCOMMAND "gs386"
#define GVFAR far
#define GVHUGE huge
#define BYTE unsigned char
#define WORD  unsigned short
#define DWORD unsigned long
#define UINT  unsigned int
#define LONG  long
#define BOOL  int
#define READBIN  "rb"
#define WRITEBIN "wb"
#define MAXSTR 126
#define DIRSEP '\\'
#define EOLSTR "\r\n"
#include <alloc.h>
#include <dir.h>
#include <io.h>
#include <fcntl.h>
#endif

#if defined(UNIX) || defined(__UNIX) || defined(__unix)
#define GSCOMMAND "gs"
#define GVFAR 
#define GVHUGE
#define BYTE unsigned char
#define WORD  unsigned short
#define DWORD unsigned int
#define UINT  unsigned int
#define LONG  int
#define BOOL  int
#define READBIN  "r"
#define WRITEBIN "w"
#define MAXSTR 256
#define DIRSEP '/'
#define EOLSTR "\n"
#if defined(__NeXT__)
extern char *getwd (char *pathname);
#define gs_getcwd(s,n) getwd(s)
#endif
/* char *gs_getcwd(char *, int); */
#define stricmp(s1, s2) strcasecmp(s1, s2)
#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_END 2
#endif
#endif

#include "ps.h"
#include "gvceps.h"

#define TRUE 1
#define FALSE 0

#ifndef min
#define min(x,y)  ( (x) < (y) ? (x) : (y) )
#endif
#ifndef max
#define max(x,y)  ( (x) > (y) ? (x) : (y) )
#endif

extern BOOL debug;
extern char oname[MAXSTR];
extern char upname[MAXSTR];
extern char szScratch[];
extern char szAppName[];

typedef struct document PSDOC;

typedef struct tagPSFILE {
	BOOL	ignore_dsc;	/* true if DSC to be ignored */
	PSDOC	*doc;		/* DSC structure.  NULL if not DSC */
	int 	pagenum;	/* current page number */
	char 	name[MAXSTR];	/* name of selected document file */
	FILE 	*file;		/* selected file */
	int 	preview;	/* preview type IDS_EPSF, IDS_EPSI, etc. */
} PSFILE;

typedef struct tagOPTION {
	float	xdpi;
	float	ydpi;
} OPTION;

FILE * gp_open_scratch_file(const char *prefix, char *fname, const char *mode);
void gserror(UINT id, char *str, UINT icon, int sound);
void pserror(char *str);
char * gs_getcwd(char *dirname, int size);
void play_sound(int i);
char * psfile_name(PSFILE *psf);

/* temporary kludges */
extern PSDOC *doc;
extern PSFILE psfile;
extern OPTION option;
#ifndef MB_ICONEXCLAMATION
#define MB_ICONEXCLAMATION  1
#endif
#define SOUND_ERROR 1
#define IDM_EXTRACTPS  1
#define IDM_EXTRACTPRE 2
#define IDM_MAKEEPST4  3
#define IDM_MAKEEPST6U 4
#define IDM_MAKEEPST6P 5
#define IDM_MAKEEPSW   6
#define IDS_TOPICEDIT 1
#define COPY_BUF_SIZE 4096

/* error messages */
#define IDS_NOPREVIEW 1
#define IDS_EPSUSERINVALID 2
