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

/* gvceps.h */
/* Common data structures for EPS manipulation */

/* because Windows and PM define the same bitmap structures  */
/* with different names we use our own common structures */
/* BITMAP1 = Windows BITMAPCORE and PM BITMAPINFO */
/* BITMAP2 = Windows BITMAPINFO and PM BITMAPINFO2 */
/* RGB3 = Windows RGBTRIPLE and PM RGB */
/* RGB4 = Windows RGBQUAD and PM RGB2 */

#ifdef __EMX__
#pragma pack(1)		/* align structures to byte boundaries */
#endif

typedef struct tagRGB3
{
    BYTE    rgbtBlue;
    BYTE    rgbtGreen;
    BYTE    rgbtRed;
} RGB3;
typedef RGB3 GVFAR* LPRGB3;

typedef struct tagRGB4
{
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;
} RGB4;
typedef RGB4 GVFAR* LPRGB4;

typedef struct tagBITMAP1
{
    DWORD   bcSize;
    short   bcWidth;
    short   bcHeight;
    WORD    bcPlanes;
    WORD    bcBitCount;
} BITMAP1;
typedef BITMAP1 GVFAR* LPBITMAP1;

typedef struct tagBITMAP2
{
    DWORD   biSize;
    LONG    biWidth;
    LONG    biHeight;
    WORD    biPlanes;
    WORD    biBitCount;
    DWORD   biCompression;
    DWORD   biSizeImage;
    LONG    biXPelsPerMeter;
    LONG    biYPelsPerMeter;
    DWORD   biClrUsed;
    DWORD   biClrImportant;
} BITMAP2;
typedef BITMAP2 GVFAR* LPBITMAP2;

typedef struct tagBITMAPFILE
{
    WORD    bfType;
    DWORD   bfSize;
    WORD    bfReserved1;
    WORD    bfReserved2;
    DWORD   bfOffBits;
} BITMAPFILE;
typedef BITMAPFILE GVFAR* LPBITMAPFILE;


struct eps_header_s {
    char id[4];
    DWORD ps_begin;
    DWORD ps_length;
    DWORD mf_begin;
    DWORD mf_length;
    DWORD tiff_begin;
    DWORD tiff_length;
    WORD  checksum;
};
#define EPS_HEADER_SIZE 30

#ifdef __EMX__
#pragma pack()
#endif

/* a structure for holding details of a bitmap used for constructing */
/* an EPS preview */
typedef struct tagPREBMAP {
    int  width;
    int  height;
    int  depth;
    int  bytewidth;	/* length of each scan line in bytes */
    BYTE GVHUGE* bits;
    BOOL topleft;
} PREBMAP;


typedef struct tagPSBBOX {
	int	llx;
	int	lly;
	int	urx;
	int	ury;
	int	valid;
} PSBBOX;
extern PSBBOX bbox;

/* in gvpeps.c or gvweps.c */
LPBITMAP2 get_bitmap(void);
void release_bitmap(void);

/* in gvceps.c */
unsigned long dib_bytewidth(LPBITMAP2 pbm);
unsigned int dib_pal_colors(LPBITMAP2 pbm);
int make_eps_tiff(int type, BOOL calc_bbox);
int make_eps_interchange(BOOL calc_bbox);
int make_eps_user(void);
int make_eps_metafile(BOOL calc_bbox);
void extract_doseps(int command);

