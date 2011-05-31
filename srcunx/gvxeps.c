/* Copyright (C) 2000, Ghostgum Software Pty Ltd.  All rights reserved.
  
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

/* gvxeps.cpp */

#include "gvx.h"

static unsigned char *get_bitmap_ptr;
void PutDWORD(unsigned char *buf, DWORD dw);
void PutWORD(unsigned char *buf, WORD w);

void PutDWORD(unsigned char *buf, DWORD dw)
{
    buf[0] = dw & 0xff;
    buf[1] = (dw >> 8) & 0xff;
    buf[2] = (dw >> 16) & 0xff;
    buf[3] = (dw >> 24) & 0xff;
}

void PutWORD(unsigned char *buf, WORD w)
{
    buf[0] = w & 0xff;
    buf[1] = (w >> 8) & 0xff;
}


LPBITMAP2 get_bitmap(void)
{
    unsigned char *pb;
    int bytewidth = ((image.width * 24 + 31) & ~31) >> 3;
    long size = BITMAP2_LENGTH + (3 * bytewidth * image.height);
    int y;
    unsigned char *s, *d;
    int color = image.format & DISPLAY_COLORS_MASK;

    release_bitmap();
    pb = (unsigned char *)malloc(size);
    if (pb == NULL)
	return NULL;

    /* Write BMP header */
    PutDWORD(pb, BITMAP2_LENGTH);
    PutDWORD(pb+4, image.width);
    PutDWORD(pb+8, image.height);
    PutWORD(pb+12, 1);
    PutWORD(pb+14, 24);
    PutDWORD(pb+16, 0);
    PutDWORD(pb+20, 0);
    PutDWORD(pb+24, (long)(1000 * option.xdpi / 25.4));
    PutDWORD(pb+28, (long)(1000 * option.ydpi / 25.4));
    PutDWORD(pb+32, 0);
    PutDWORD(pb+36, 0);

    /* convert raster */
    for (y = 0; y<image.height; y++) {
	s = image.image + y * image.raster;
	d = pb + BITMAP2_LENGTH + bytewidth * (image.height-1-y);
	if ((color == DISPLAY_COLORS_NATIVE) ||
	    (color == DISPLAY_COLORS_RGB) ||
	    (color == DISPLAY_COLORS_CMYK))
	   image_to_24BGR(&image, d, s);
	else
	    memset(d, 0xff, bytewidth);
    }

    get_bitmap_ptr = pb;
    return (LPBITMAP2)get_bitmap_ptr;
}

void release_bitmap(void)
{
    if (get_bitmap_ptr)
	free(get_bitmap_ptr);
    get_bitmap_ptr = NULL;
}

