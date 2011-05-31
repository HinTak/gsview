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

/* not implemented fully - only 16 and 24 bit displays supported */

#include "gvx.h"


unsigned char *xbmp;

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
    BITMAP2 bmp;
    if (debug & DEBUG_GENERAL)
	gs_addmess("get_bitmap: not implemented fully.  Only 16 & 24 bit displays supported\n");

    XImage *image = NULL;
    long plane_mask = (1L << display.bitcount) -1;
    if (pixmap == NULL)
	return NULL;

    image = XGetImage(dpy, GDK_WINDOW_XWINDOW(pixmap), 
	0, 0, display.width, display.height,
	plane_mask, ZPixmap);
    if (image == NULL)
	return NULL;

    if (debug & DEBUG_GENERAL) {
	gs_addmess("get_bitmap:\n");
	gs_addmessf("  width=%d\n", image->width);
	gs_addmessf("  height=%d\n", image->height);
	gs_addmessf("  xoffset=%d\n", image->xoffset);
	gs_addmessf("  format=%d\n", image->format);
	gs_addmessf("  data=0x%lx\n", (unsigned long)image->data);
	gs_addmessf("  byte_order=%d\n", image->byte_order);
	gs_addmessf("  bitmap_unit=%d\n", image->bitmap_unit);
	gs_addmessf("  bitmap_bit_order=%d\n", image->bitmap_bit_order);
	gs_addmessf("  bitmap_pad=%d\n", image->bitmap_pad);
	gs_addmessf("  depth=%d\n", image->depth);
	gs_addmessf("  bytes_per_line=%d\n", image->bytes_per_line);
	gs_addmessf("  bits_per_pixel=%d\n", image->bits_per_pixel);
	gs_addmessf("  red_mask=0x%lx\n", image->red_mask);
	gs_addmessf("  green_mask=0x%lx\n", image->green_mask);
	gs_addmessf("  blue_mask=0x%lx\n", image->blue_mask);
    }

    /* Convert image to 24 bpp Windows BMP format */
    bmp.biSize = BITMAP2_LENGTH;
    bmp.biWidth = image->width;
    bmp.biHeight = image->height;
    bmp.biPlanes = 1;
    bmp.biBitCount = 24;
    bmp.biCompression = 0;
    bmp.biSizeImage = 0;
    bmp.biXPelsPerMeter = (long)(1000 * option.xdpi / 25.4);
    bmp.biYPelsPerMeter = (long)(1000 * option.ydpi / 25.4);
    bmp.biClrUsed = 0;
    bmp.biClrImportant = 0;
    int bytewidth = ((bmp.biWidth * bmp.biBitCount + 31) & ~31) >> 3;
    long size = BITMAP2_LENGTH + (3 * bytewidth * bmp.biHeight);
    if (debug & DEBUG_GENERAL) {
	gs_addmess("bmp:\n");
	gs_addmessf(" biSize=%ld\n", bmp.biSize);
	gs_addmessf(" biWidth=%ld\n", bmp.biWidth);
	gs_addmessf(" biHeight=%ld\n", bmp.biHeight);
	gs_addmessf(" bytewidth=%d\n", bytewidth);
	gs_addmessf(" size=%ld\n", size);
    }

    if (xbmp != NULL)
	release_bitmap();
    if ((image->depth == 16) || (image->depth == 24)) {
	if ((xbmp = (unsigned char *)malloc(size)) != NULL) {
	    /* write BMP header */
	    PutDWORD(xbmp, bmp.biSize);
	    PutDWORD(xbmp+4, bmp.biWidth);
	    PutDWORD(xbmp+8, bmp.biHeight);
	    PutWORD(xbmp+12, bmp.biPlanes);
	    PutWORD(xbmp+14, bmp.biBitCount);
	    PutDWORD(xbmp+16, bmp.biCompression);
	    PutDWORD(xbmp+20, bmp.biSizeImage);
	    PutDWORD(xbmp+24, bmp.biXPelsPerMeter);
	    PutDWORD(xbmp+28, bmp.biXPelsPerMeter);
	    PutDWORD(xbmp+32, bmp.biClrUsed);
	    PutDWORD(xbmp+36, bmp.biClrImportant);

	    int x, y;
	    unsigned char *s, *d;
	    for (y=0; y<bmp.biHeight; y++) {
		s = (unsigned char *)image->data + y * image->bytes_per_line;
		d = xbmp + BITMAP2_LENGTH + 
			(bytewidth * (bmp.biHeight - 1 - y));
		if (image->depth == 24) {
		    int step = image->bits_per_pixel >> 3;
		    if (image->byte_order == MSBFirst) {
			/* red first */
			for (x=0; x<bmp.biWidth; x++) {
			    d[0] = s[2]; 
			    d[1] = s[1]; 
			    d[2] = s[0]; 
			    s += step;
			    d += 3;
			}
		    }
		    else {
			/* blue first */
			for (x=0; x<bmp.biWidth; x++) {
			    d[0] = s[0]; 
			    d[1] = s[1]; 
			    d[2] = s[2]; 
			    s += step;
			    d += 3;
			}
		    }
		}
		else if (image->depth == 16) {
		    int step = image->bits_per_pixel >> 3;
		    int value;
		    int r, g, b;
		    for (x=0; x<bmp.biWidth; x++) {
			if (image->byte_order == MSBFirst)
			    value = (s[0] << 8) + s[1];
			else
			    value = (s[1] << 8) + s[0];
			r = (value >> 11) & 0x1f;
			g = (value >> 5) & 0x3f;
			b = value & 0x1f;
			d[0] = (b << 3) + (b >> 2);
			d[1] = (g << 2) + (g >> 4);
			d[2] = (r << 3) + (r >> 2);
			s += step;
			d += 3;
		    }
		}
		else {
		    /* panic */
		}
	    }
	}
    }
    XDestroyImage(image);

    return (LPBITMAP2)xbmp;
}

void release_bitmap(void)
{
    free(xbmp);
    xbmp = NULL;
}

