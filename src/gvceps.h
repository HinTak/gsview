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
    BYTE    rgbtBlue PACKED;
    BYTE    rgbtGreen PACKED;
    BYTE    rgbtRed PACKED;
} RGB3;
typedef RGB3 GVFAR* LPRGB3;

typedef struct tagRGB4
{
    BYTE    rgbBlue PACKED;
    BYTE    rgbGreen PACKED;
    BYTE    rgbRed PACKED;
    BYTE    rgbReserved PACKED;
} RGB4;
typedef RGB4 GVFAR* LPRGB4;

typedef struct tagBITMAP1
{
    DWORD   bcSize PACKED;
    short   bcWidth PACKED;
    short   bcHeight PACKED;
    WORD    bcPlanes PACKED;
    WORD    bcBitCount PACKED;
} BITMAP1;
typedef BITMAP1 GVFAR* LPBITMAP1;

typedef struct tagBITMAP2
{
    DWORD   biSize PACKED;
    LONG    biWidth PACKED;
    LONG    biHeight PACKED;
    WORD    biPlanes PACKED;
    WORD    biBitCount PACKED;
    DWORD   biCompression PACKED;
    DWORD   biSizeImage PACKED;
    LONG    biXPelsPerMeter PACKED;
    LONG    biYPelsPerMeter PACKED;
    DWORD   biClrUsed PACKED;
    DWORD   biClrImportant PACKED;
} BITMAP2;
typedef BITMAP2 GVFAR* LPBITMAP2;

typedef struct tagBITMAPFILE
{
    WORD    bfType PACKED;
    DWORD   bfSize PACKED;
    WORD    bfReserved1 PACKED;
    WORD    bfReserved2 PACKED;
    DWORD   bfOffBits PACKED;
} BITMAPFILE;
typedef BITMAPFILE GVFAR* LPBITMAPFILE;


struct eps_header_s {
    char id[4] PACKED;
    DWORD ps_begin PACKED;
    DWORD ps_length PACKED;
    DWORD mf_begin PACKED;
    DWORD mf_length PACKED;
    DWORD tiff_begin PACKED;
    DWORD tiff_length PACKED;
    WORD  checksum PACKED;
};

#ifdef __EMX__
#pragma pack()
#endif

/* in gvpeps.c or gvweps.c */
LPBITMAP2 get_bitmap(void);
void release_bitmap(void);

/* in gvceps.c */
unsigned long dib_bytewidth(LPBITMAP2 pbm);
unsigned int dib_pal_colors(LPBITMAP2 pbm);
int make_eps_tiff(int type, BOOL calc_bbox);
int make_eps_interchange(BOOL calc_bbox);
int make_eps_user(void);
void extract_doseps(int command);
void copy_bbox_header(FILE *f);
