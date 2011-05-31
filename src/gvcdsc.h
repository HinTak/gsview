/* Copyright (C) 1993, 1994, Russell Lang.  All rights reserved.
  
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

/* gvcdsc.h */

/* header file for DSC scanning */

#define PSLINELENGTH	256

#define ATEND		-1
#define NONE		0

#define PORTRAIT	1
#define LANDSCAPE	2

#define ASCEND		1
#define	DESCEND		2
#define SPECIAL		3

typedef struct tagPSMEDIA {
	char	*name;
	int	width;
	int	height;
	struct tagPSMEDIA *next;
} PSMEDIA;

typedef struct tagPSBBOX {
	int	llx;
	int	lly;
	int	urx;
	int	ury;
	int	valid;
} PSBBOX;

typedef struct tagPSPAGE {
	long	begin;
	long	end;
	char	*label;
	int	ordinal;
	struct tagPSPAGE *next;
} PSPAGE;

/* DOS binary EPS header */
typedef struct tagDOSEPS {
   unsigned char id[4];
   unsigned long ps_begin;
   unsigned long ps_length;
   unsigned long mf_begin;
   unsigned long mf_length;
   unsigned long tiff_begin;
   unsigned long tiff_length;
   unsigned short checksum;
} DOSEPS;

typedef struct tagPSDOC {
	long	begincomments;
	long	endcomments;
	long	beginpreview;
	long	endpreview;
	long	begindefaults;
	long	enddefaults;
	long 	beginprolog;
	long	endprolog;
	long 	beginsetup;
	long	endsetup;
	PSPAGE	*page_list;	/* linked list of pages */
	PSPAGE	*pages;		/* array of pages */
	long	begintrailer;
	long	endtrailer;
	int	epsf;
	int	ctrld;		/* true if DSC except for ^D prefix */
	char	*firstline;
	char	*title;
	char 	*date;
	PSBBOX	bbox;
	int	dscpages;	/* from %%Pages */
	int	numpages;	/* actual number of %%Page */
	int	pageorder;
	int	orientation;
	PSMEDIA	*default_page_media;
	PSMEDIA	*document_media;
	DOSEPS	*doseps;	/* DOS binary EPS header */
	/* these fields are used internally by gvcdsc.c */
	/* and should not be treated as containing valid data */
	/* by other modules */
	FILE 	*f;		/* file being scanned */
	long	position;	/* position before line read */
	long	enddoseps;	/* position of end of ps section in DOSEPS file */
	char 	*line;		/* current line buffer */
	int	linecount;	/* current line number */
} PSDOC;

extern PSMEDIA paper_size[34];

/* global functions in gvcdsc.c */
void dsc_scan_debug(int flag);
PSDOC * dsc_scan_file(FILE *f);
void dsc_scan_clean(PSDOC *psdoc);
char * dsc_copy(FILE *fromfile, FILE *tofile, long begin, long end, char *comment);
DWORD reorder_dword(DWORD val);
WORD reorder_word(WORD val);

/* must be provided by program linking with gvcdsc.c */
void pserror(char *str);

