/*
 * clip.c --    Clipboard and EPS operations for GSVIEW.EXE, 
 *              a graphical interface for MS-Windows Ghostscript
 * Copyright (C) 1993  Russell Lang
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Author: Russell Lang
 * Internet: rjl@monu1.cc.monash.edu.au
 */

#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <shellapi.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dir.h>
#include <io.h>
#define NeedFunctionPrototypes 1
#include "ps.h"
#include "gsview.h"

#include <time.h>

#define COPY_BUF_SIZE 16384u

/* Write data to file - blocks > 64k permitted */
long
hwrite(HFILE hf, const void _huge *hpvBuffer, long cbBuffer)
{
DWORD count;
long written, done;
char _huge *hp;
	if (is_win31)
	    return _hwrite(hf, hpvBuffer, cbBuffer);
	done = 0;
	hp = (char _huge *)hpvBuffer;
	while (cbBuffer > 0) {
	    count = min( min(32768UL, cbBuffer), (DWORD)(65536UL-OFFSETOF(hp)) );
	    written = _lwrite(hf, hp, (UINT)count);
	    if (written == (long)HFILE_ERROR)
		return (long)HFILE_ERROR;
	    done += written;
	    cbBuffer -= written;
	    hp += written;
	}
	return done;
}

/* return number of bytes per line, rounded up to multiple of 4 bytes */
LONG
dib_bytewidth(LPBITMAPINFOHEADER pbmih)
{
 return (((pbmih->biWidth * pbmih->biBitCount + 31) & ~31) >> 3);
}

/* return number of colors in color table */
UINT
dib_pal_colors(LPBITMAPINFOHEADER pbmih)
{
int bits_per_pixel = pbmih->biPlanes * pbmih->biBitCount;
	if (bits_per_pixel != 24) {
	    if (pbmih->biSize == sizeof(BITMAPCOREHEADER)) {
		return 1<<bits_per_pixel;
	    }
	    else {
		return (pbmih->biClrUsed) ? (UINT)(pbmih->biClrUsed) : 1<<bits_per_pixel;
	    }
	}
	return 0;
}

/* copy a DIB from the clipboard to a file */
void
clip_to_file(void)
{
HGLOBAL hglobal;
LPBITMAPINFOHEADER pbmih;
BITMAPFILEHEADER bmfh;
UINT palcolors;
UINT palsize;
DWORD bitmap_size;
BYTE _huge *lpBits;
static char output[MAXSTR];
HFILE hfile;
	if (!OpenClipboard(hwndimg)) {
		play_sound(SOUND_ERROR);
		return;
	}
	if (!IsClipboardFormatAvailable(CF_DIB)) {
		CloseClipboard();
		play_sound(SOUND_ERROR);
		return;
	}
	hglobal = (HGLOBAL)GetClipboardData(CF_DIB);
	pbmih = (LPBITMAPINFOHEADER)GlobalLock(hglobal);
	palcolors = dib_pal_colors(pbmih);
	if (pbmih->biSize == sizeof(BITMAPCOREHEADER))
		palsize = palcolors * sizeof(RGBTRIPLE); 
	else
		palsize = palcolors * sizeof(RGBQUAD);
	bitmap_size = (DWORD)pbmih->biHeight *  dib_bytewidth(pbmih);

	bmfh.bfType = ('M'<<8) | 'B';
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + pbmih->biSize + palsize;
	bmfh.bfSize = bmfh.bfOffBits + bitmap_size;

	if ( getfilename(output, SAVE, FILTER_BMP, NULL, IDS_TOPICEDIT)
	    && ((hfile = _lcreat(output, 0)) != HFILE_ERROR) ) {
		hwrite(hfile, &bmfh, sizeof(BITMAPFILEHEADER));
		hwrite(hfile, pbmih, pbmih->biSize + palsize);
		lpBits =  ((BYTE _huge *)pbmih) + pbmih->biSize + palsize;
		hwrite(hfile, lpBits, bitmap_size);
		_lclose(hfile);
	}
	GlobalUnlock(hglobal);
	CloseClipboard();
}

/* convert bitmap (DIB or DDB) in clipboard to */
/* CF_DIB, CF_BITMAP and CF_PALETTE */
void
clip_convert(void)
{
	if (!OpenClipboard(hwndimg)) {
	    play_sound(SOUND_ERROR);
	    return;
	}
	if (IsClipboardFormatAvailable(CF_DIB)) {
	    if (!IsClipboardFormatAvailable(CF_PALETTE))
		clip_add_palette();
	    if (!IsClipboardFormatAvailable(CF_BITMAP))
	        clip_add_ddb();
	}
	else {
	    if (IsClipboardFormatAvailable(CF_BITMAP)) {
		clip_add_dib();
	        if (!IsClipboardFormatAvailable(CF_PALETTE))
		    clip_add_palette();
	    }
	    else 
		play_sound(SOUND_ERROR);
	}
	CloseClipboard();
}

/* Read DIB from the clipboard, create PALETTE and add to clipboard */
void
clip_add_palette(void)
{
HGLOBAL hglobal;
LPBITMAPINFOHEADER pbmih;
UINT palcolors;
UINT palsize;
int i;
LPLOGPALETTE logpalette;
HPALETTE hpalette;
RGBQUAD FAR *prgbquad;
RGBTRIPLE FAR *prgbtriple;

	if (!IsClipboardFormatAvailable(CF_DIB)) {
		play_sound(SOUND_ERROR);
		return;
	}
	hglobal = (HGLOBAL)GetClipboardData(CF_DIB);
	pbmih = (LPBITMAPINFOHEADER)GlobalLock(hglobal);
	palcolors = dib_pal_colors(pbmih);
	if (pbmih->biSize == sizeof(BITMAPCOREHEADER))
		palsize = palcolors * sizeof(RGBTRIPLE); 
	else
		palsize = palcolors * sizeof(RGBQUAD);
	hpalette = (HPALETTE)NULL;
	if (palsize) {
	    /* create palette to match DIB */
	    logpalette = (LPLOGPALETTE) malloc( sizeof(LOGPALETTE) + 
		palcolors * sizeof(PALETTEENTRY) );
	    if (logpalette == (LPLOGPALETTE)NULL) {
		GlobalUnlock(hglobal);
		play_sound(SOUND_ERROR);
		return;
	    }
	    logpalette->palVersion = 0x300;
	    logpalette->palNumEntries = palcolors;
	    prgbquad = (RGBQUAD FAR *)(((BYTE _huge *)pbmih) + pbmih->biSize);
	    if (pbmih->biSize == sizeof(BITMAPCOREHEADER)) {
		/* OS2 format */
	        prgbtriple = (RGBTRIPLE FAR *)prgbquad;
	        for (i=0; i<palcolors; i++) {
	            logpalette->palPalEntry[i].peFlags = 0;
		    logpalette->palPalEntry[i].peRed   = prgbtriple[i].rgbtRed;
		    logpalette->palPalEntry[i].peGreen = prgbtriple[i].rgbtGreen;
		    logpalette->palPalEntry[i].peBlue  = prgbtriple[i].rgbtBlue;
	        }
	    }
	    else {
		/* Windows Format */
	        for (i=0; i<palcolors; i++) {
	            logpalette->palPalEntry[i].peFlags = 0;
		    logpalette->palPalEntry[i].peRed   = prgbquad[i].rgbRed;
		    logpalette->palPalEntry[i].peGreen = prgbquad[i].rgbGreen;
		    logpalette->palPalEntry[i].peBlue  = prgbquad[i].rgbBlue;
	        }
	    }
	    hpalette = CreatePalette(logpalette);
	    free((void *)logpalette);
	    SetClipboardData(CF_PALETTE, hpalette);
	}
	GlobalUnlock(hglobal);
}


/* Read DIB from the clipboard, convert to DDB and add to clipboard */
void
clip_add_ddb(void)
{
HGLOBAL hglobal;
LPBITMAPINFOHEADER pbmih;
UINT palcolors;
UINT palsize;
HPALETTE hpalette;
HDC hdc;
HBITMAP hbitmap;

	hglobal = (HGLOBAL)GetClipboardData(CF_DIB);
	pbmih = (LPBITMAPINFOHEADER)GlobalLock(hglobal);
	palcolors = dib_pal_colors(pbmih);
	if (pbmih->biSize == sizeof(BITMAPCOREHEADER))
		palsize = palcolors * sizeof(RGBTRIPLE); 
	else
		palsize = palcolors * sizeof(RGBQUAD);

	hdc = GetDC(hwndimg);
	hpalette = GetClipboardData(CF_PALETTE);
	if (hpalette) {
	    SelectPalette(hdc,hpalette,NULL);
	    RealizePalette(hdc);
	}
	hbitmap = CreateDIBitmap(hdc, pbmih, CBM_INIT,
		((BYTE _huge *)pbmih) + pbmih->biSize + palsize,
		(LPBITMAPINFO)pbmih, DIB_RGB_COLORS);
	ReleaseDC(hwndimg, hdc);
	GlobalUnlock(hglobal);
	SetClipboardData(CF_BITMAP, hbitmap);
}

/* make a DIB from a BITMAP in the clipboard */
/* GetDIBits won't work for 4 plane or 4 bit/pixel bitmaps */
/* clipboard must be open */
HGLOBAL 
make_dib(void)
{
LPBITMAPINFOHEADER pbmih;
LPBITMAPINFO pbmi;
BYTE FAR *lpBits;
HBITMAP hbitmap;
UINT palcolors;
UINT palsize;
UINT byte_width;
DWORD bitmap_size;
HGLOBAL hglobal;
HDC hdc;
HDC hdc_bit;
BITMAP bm;
PALETTEENTRY *pe;
int i;
	hbitmap = GetClipboardData(CF_BITMAP);
	hdc = GetDC((HWND)NULL);
	hdc_bit = CreateCompatibleDC(hdc);
	ReleaseDC((HWND)NULL,hdc);
	GetObject(hbitmap, sizeof(BITMAP), &bm);
	if (bm.bmPlanes == 4) {
	    HBITMAP hbitmap_new, hbitmap_old;
	    HDC hdc_new;
	    /* convert format to 1 plane, 1 bit/pixel */ 
	    bm.bmPlanes = 1;
	    bm.bmBitsPixel = 1;
	    hdc_new = CreateCompatibleDC(hdc_bit);
	    hbitmap_new = CreateBitmap(bm.bmWidth, bm.bmHeight, bm.bmPlanes, bm.bmBitsPixel, NULL);
	    SelectBitmap(hdc_new, hbitmap_new);
	    hbitmap_old = SelectBitmap(hdc_bit, hbitmap);
	    BitBlt(hdc_new, 0, 0, bm.bmWidth, bm.bmHeight, hdc_bit, 0, 0, SRCCOPY);
	    SelectBitmap(hdc_bit, hbitmap_old);
	    DeleteDC(hdc_new);
	    hbitmap = hbitmap_new;
	}
 	byte_width = (((bm.bmWidth * bm.bmBitsPixel + 31) & ~31) >> 3);
	bitmap_size = (DWORD)bm.bmHeight *  byte_width;
	palcolors = 1<<(bm.bmBitsPixel * bm.bmPlanes);
	palsize = palcolors * sizeof(RGBQUAD);
	hglobal = GlobalAlloc(GHND, sizeof(BITMAPINFOHEADER) + palsize + bitmap_size);
	if (hglobal != (HGLOBAL)NULL) {
	    lpBits = GlobalLock(hglobal);
	    pbmi = (LPBITMAPINFO)lpBits;
	    pbmih = (LPBITMAPINFOHEADER)lpBits;
	    lpBits += sizeof(BITMAPINFOHEADER) + palsize;
	    pbmih->biSize = sizeof(BITMAPINFOHEADER);
	    pbmih->biWidth = bm.bmWidth;
	    pbmih->biHeight = bm.bmHeight;
	    pbmih->biPlanes = 1;
	    pbmih->biBitCount = bm.bmBitsPixel * bm.bmPlanes;
	    pbmih->biCompression = BI_RGB;
	    pbmih->biSizeImage = bitmap_size;
	    pbmih->biXPelsPerMeter = (int)(xdpi / 25.4 * 1000);
	    pbmih->biYPelsPerMeter = (int)(ydpi / 25.4 * 1000);
	    pbmih->biClrUsed = palcolors;
	    pbmih->biClrImportant = palcolors;
	    /* create colour table from system palette */
	    pe = malloc(palcolors * sizeof(PALETTEENTRY));
	    if (IsClipboardFormatAvailable(CF_PALETTE)) {
		HPALETTE hpalette = GetClipboardData(CF_PALETTE);
		i = GetObject(hpalette, sizeof(int), pe);
		GetPaletteEntries(hpalette, 0, i, pe);
	    }
	    else
	        GetSystemPaletteEntries(hdc_bit, 0, palcolors, pe);
	    for (i=0; i<palcolors; i++) {
		pbmi->bmiColors[i].rgbRed = pe[i].peRed;
		pbmi->bmiColors[i].rgbGreen = pe[i].peGreen;
		pbmi->bmiColors[i].rgbBlue = pe[i].peBlue;
		pbmi->bmiColors[i].rgbReserved = 0;
	    }
	    free((void *)pe);
	    i = GetDIBits(hdc_bit, hbitmap, 0, (UINT)pbmih->biHeight,
		lpBits, pbmi, DIB_RGB_COLORS);
	    GlobalUnlock(hglobal);
	    if (i == 0) {
		GlobalFree(hglobal);
		hglobal = NULL;
	    }
	}
	DeleteDC(hdc_bit);
	if (hbitmap != GetClipboardData(CF_BITMAP))
		DeleteBitmap(hbitmap);
	return hglobal;
}

/* Read DDB from the clipboard, convert to DIB and add to clipboard */
void
clip_add_dib(void)
{
HGLOBAL hglobal;
	hglobal = make_dib();
	if (hglobal != (HGLOBAL)NULL)
	        SetClipboardData(CF_DIB, hglobal);
}

/* extract temp PS file from EPS dfile */
/* point dfile at temp PS file */
void extract_eps(void)
{
DWORD pos;
DWORD len;
FILE *f;
UINT count;
char *buffer;
DWORD dtemp;
	rewind(dfile);
	fread(&dtemp, 4, 1, dfile);	/* ignore ID */
	fread(&pos, 4, 1, dfile);	/* PS offset */
	fread(&len, 4, 1, dfile);	/* PS length */
	fread(&dtemp, 4, 1, dfile);	/* Metafile offset */
	if (dtemp != 0L)
	    preview = IDS_EPSW;
	else {
	    fread(&dtemp, 4, 1, dfile);	/* Metafile length */
	    fread(&dtemp, 4, 1, dfile);	/* TIFF offset */
	    if (dtemp != 0L)
	        preview = IDS_EPST;
	}
	fseek(dfile, pos, SEEK_SET);	/* seek to PS section */
	/* get new scratch file */
	if ( (f = gp_open_scratch_file(szScratch, efname, "wb")) == (FILE *)NULL) {
		play_sound(SOUND_ERROR);
		return;
	}
	
	/* create buffer for PS file copy */
	buffer = malloc(COPY_BUF_SIZE);
	if (buffer == (char *)NULL) {
	    play_sound(SOUND_ERROR);
	    fclose(f);
	    return;
	}

        while ( (count = (UINT)min(len,COPY_BUF_SIZE)) != 0 ) {
	    count = fread(buffer, 1, count, dfile);
	    fwrite(buffer, 1, count, f);
	    len -= count;
	}
	free(buffer);
	fclose(f);
	fclose(dfile);
	dfile = fopen(efname, "rb");
}

typedef struct {
    DWORD	key;
    HMETAFILE	hmf;
    RECT	bbox;
    WORD	inch;
    DWORD	reserved;
    WORD	checksum;
} METAFILEHEADER;

/* extract EPS or TIFF or WMF file from DOS EPS file */
void extract_doseps(WORD command)
{
DWORD pos;
DWORD len;
UINT count;
char *buffer;
FILE* epsfile;
BOOL is_meta = TRUE;
char outname[MAXSTR];
FILE *outfile;
UINT filter;
	if (!preview || (preview == IDS_EPSI)) {
	    gserror(IDS_NOPREVIEW, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
	    return;
	}
	epsfile = fopen(dfname,"rb");
	fread(&pos, 4, 1, epsfile);		/* ignore ID */
	fread(&pos, 4, 1, epsfile);		/* PS offset */
	fread(&len, 4, 1, epsfile);		/* PS length */
	if (command == IDM_EXTRACTPRE) {
	    fread(&pos, 4, 1, epsfile);		/* Metafile offset */
	    fread(&len, 4, 1, epsfile);		/* Metafile length */
	    if (pos == 0L) {
	        fread(&pos, 4, 1, epsfile);	/* TIFF offset */
	        fread(&len, 4, 1, epsfile);	/* TIFF length */
	        is_meta = FALSE;
	    }
	}
	if (pos == 0L) {
	    fclose(epsfile);
	    gserror(IDS_NOPREVIEW, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
	    return;
	}
	fseek(epsfile, pos, SEEK_SET);	/* seek to section to extract */

	/* create postscript or preview file */
	outname[0] = '\0';
	if (command == IDM_EXTRACTPRE) {
	    if (is_meta)
	        filter = FILTER_WMF;
	    else
	        filter = FILTER_TIFF;
	}
	else
	    filter = FILTER_PS;
	if (!getfilename(outname, SAVE, filter, NULL, IDS_TOPICEDIT)) {
	    fclose(epsfile);
	    return;
	}

	outfile = fopen(outname, "wb");
	if (outfile == (FILE *)NULL) {
	    play_sound(SOUND_ERROR);
	    fclose(epsfile);
	    return;
	}
	
	/* create buffer for file copy */
	buffer = malloc(COPY_BUF_SIZE);
	if (buffer == (char *)NULL) {
	    play_sound(SOUND_ERROR);
	    fclose(epsfile);
	    fclose(outfile);
	    return;
	}

	if ((command == IDM_EXTRACTPRE) && is_meta) {
	    /* write placeable Windows Metafile header */
	    METAFILEHEADER mfh;
	    int i;
	    WORD *pw;
	    mfh.key = 0x9ac6cdd7L;
	    mfh.hmf = 0;
	    mfh.bbox.left = 0;
	    mfh.bbox.right = doc->boundingbox[URX] - doc->boundingbox[LLX];
	    mfh.bbox.top = 0;
	    mfh.bbox.bottom = doc->boundingbox[URY] - doc->boundingbox[LLY];
	    mfh.inch = 72;	/* PostScript points */
	    mfh.reserved = 0L;
	    mfh.checksum =  0;
	    pw = (WORD *)&mfh;
	    for (i=0; i<10; i++) {
	    	mfh.checksum ^= *pw++;
	    }
	    fwrite(&mfh, sizeof(mfh), 1, outfile);
	}

        while ( (count = (UINT)min(len,COPY_BUF_SIZE)) != 0 ) {
	    count = fread(buffer, 1, count, epsfile);
	    fwrite(buffer, 1, count, outfile);
	    len -= count;
	}
	free(buffer);
	fclose(epsfile);
	fclose(outfile);
}

/* convert a clipboard bitmap to a metafile picture */
HMETAFILE
make_metafile(void)
{
HDC hdc;
HMETAFILE hmf;
HGLOBAL hglobal;
	if (IsClipboardFormatAvailable(CF_DIB)) {
	    LPBITMAPINFOHEADER pbmih;
	    BYTE _huge *lpDibBits;
	    hglobal = GetClipboardData(CF_DIB);
	    pbmih = (LPBITMAPINFOHEADER)GlobalLock(hglobal);
	    lpDibBits = ((BYTE _huge *)pbmih) + pbmih->biSize;
	    if (pbmih->biSize == sizeof(BITMAPCOREHEADER))
		lpDibBits += dib_pal_colors(pbmih) * sizeof(RGBTRIPLE); 
	    else
		lpDibBits += dib_pal_colors(pbmih) * sizeof(RGBQUAD); 
	    /* now make a Metafile from it */
	    hdc = CreateMetaFile(NULL);
	    SetWindowOrg(hdc, 0, 0);
	    SetWindowExt(hdc, (int)pbmih->biWidth, (int)pbmih->biHeight);
	    StretchDIBits(hdc, 0, 0, (int)pbmih->biWidth, (int)pbmih->biHeight,
	        0, 0, (int)pbmih->biWidth, (int)pbmih->biHeight,
	        (void FAR *)lpDibBits, (LPBITMAPINFO)pbmih,
	        DIB_RGB_COLORS, SRCCOPY);
	    hmf = CloseMetaFile(hdc);
	    GlobalUnlock(hglobal);
	}
	else if (IsClipboardFormatAvailable(CF_BITMAP)) {
	    HBITMAP hbitmap, hbitmap_old;
	    HDC hdc_bit;
	    BITMAP bm;
	    hbitmap = GetClipboardData(CF_BITMAP);
	    hdc = GetDC((HWND)NULL);
	    hdc_bit = CreateCompatibleDC(hdc);
	    ReleaseDC((HWND)NULL,hdc);
	    GetObject(hbitmap, sizeof(BITMAP), &bm);
	    hdc = CreateMetaFile(NULL);
	    SetWindowOrg(hdc, 0, 0);
	    SetWindowExt(hdc, bm.bmWidth, bm.bmHeight);
	    hbitmap_old = SelectBitmap(hdc_bit, hbitmap);
	    StretchBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight,
	    	hdc_bit, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
	    SelectBitmap(hdc_bit, hbitmap_old);
	    DeleteDC(hdc_bit);
	    hmf = CloseMetaFile(hdc);
	}
	else {
	    play_sound(SOUND_ERROR);
	    hmf = NULL;
	}
	return hmf;
}

#ifdef NOT_USED
/* not used */
/* Read bitmap (CF_DIB or CF_BITMAP) from the clipboard */
/* convert to Metafile Picture and add to clipboard */
void
clip_add_metafile(void)
{
LPMETAFILEPICT lpmfp;
HGLOBAL hglobal;
HMETAFILE hmf;
	if ( (hmf = make_metafile()) == (HMETAFILE)NULL ) {
		return;
	}

	hglobal = GlobalAlloc(GHND | GMEM_SHARE, sizeof(METAFILEPICT)); 
	lpmfp = GlobalLock(hglobal);
	lpmfp->mm = MM_ANISOTROPIC;
	if (bitmap_width)
	    lpmfp->xExt = (int)(bitmap_width / xdpi * 2540);
	else 
	    lpmfp->xExt = 21000;	/* A4 */
	if (bitmap_height)
	    lpmfp->yExt = (int)(bitmap_height / ydpi * 2540);
	else
	    lpmfp->yExt = 29700;	/* A4 */
	lpmfp->hMF = hmf;
	GlobalUnlock(hglobal);
	SetClipboardData(CF_METAFILEPICT, hglobal);
	return;
}
#endif

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

/* make a PC EPS file with a Windows Metafile Preview */
/* from a PS file and a clipboard bitmap */
void
make_eps_metafile(void)
{
char epsname[MAXSTR];
HMETAFILE hmf;
HGLOBAL hglobal;
char *buffer;
UINT count;
HFILE hfEPS;
struct eps_header_s eps_header;

	if (!OpenClipboard(hwndimg)) {
	    play_sound(SOUND_ERROR);
	    return;
	}

	if ( (hmf = make_metafile()) == (HMETAFILE)NULL ) {
	    play_sound(SOUND_ERROR);
	    CloseClipboard();
	    return;
	}

	CloseClipboard();

	/* get memory handle to metafile */
	hglobal = GetMetaFileBits(hmf);

	/* create buffer for PS file copy */
	buffer = malloc(COPY_BUF_SIZE);
	if (buffer == (char *)NULL) {
	    GlobalFree(hglobal);
	    gserror(IDS_BADEPS, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
	    return;
	}

	/* create EPS file */
	epsname[0] = '\0';
	if (!getfilename(epsname, SAVE, FILTER_EPS, NULL, IDS_TOPICEDIT)) {
	    GlobalFree(hglobal);
	    return;
	}
	hfEPS = _lcreat(epsname, 0);

	/* write DOS EPS binary header */
	eps_header.id[0] = 0xc5;	/* "EPSF" with bit 7 set */
	eps_header.id[1] = 0xd0;
	eps_header.id[2] = 0xd3;
	eps_header.id[3] = 0xc6;
	eps_header.ps_begin = sizeof(eps_header);
	fseek(dfile, 0, SEEK_END);
	eps_header.ps_length = ftell(dfile);
	eps_header.mf_begin = eps_header.ps_begin + eps_header.ps_length;
	eps_header.mf_length = GlobalSize(hglobal);
	eps_header.tiff_begin = 0;
	eps_header.tiff_length = 0;
	eps_header.checksum = -1;
	_lwrite(hfEPS, (void _huge *)&eps_header, sizeof(eps_header));

	/* copy PS file */
	rewind(dfile);
	do {
	    count = fread(buffer, 1, COPY_BUF_SIZE, dfile);
	    _lwrite(hfEPS, buffer, count);
	} while (count != 0);
	free(buffer);

	/* copy metafile */
	hwrite(hfEPS, GlobalLock(hglobal), eps_header.mf_length);
	GlobalUnlock(hglobal);
	GlobalFree(hglobal);

	_lclose(hfEPS);
}


char hex[16] = "0123456789ABCDEF";
unsigned char isblack[256];

void
scan_colors(LPBITMAPINFOHEADER pbmih)
{
	RGBQUAD FAR *prgbquad;
	RGBTRIPLE FAR *prgbtriple;
	unsigned char rr;
	unsigned char gg;
	unsigned char bb;
	int isOS2 = FALSE;
	int clrtablesize;
	int i;

	prgbquad   = (RGBQUAD FAR *)(((BYTE _huge *)pbmih) + pbmih->biSize);
	prgbtriple = (RGBTRIPLE FAR *)prgbquad;
	if (pbmih->biSize == sizeof(BITMAPCOREHEADER))
	    isOS2 = TRUE;
	/* read in the color table */
	clrtablesize = dib_pal_colors(pbmih);
	for (i = 0; i < clrtablesize; i++) {
		if (isOS2) {
		    bb = prgbtriple[i].rgbtBlue;
		    gg = prgbtriple[i].rgbtGreen;
		    rr = prgbtriple[i].rgbtRed;
		}
		else {
		    bb = prgbquad[i].rgbBlue;
		    gg = prgbquad[i].rgbGreen;
		    rr = prgbquad[i].rgbRed;
		}
		isblack[i] = (rr < 0xff) || (gg < 0xff) || (bb < 0xff);
	}
}

BYTE _huge *
get_dib_bits(LPBITMAPINFOHEADER pbmih)
{
BYTE _huge *lpDibBits;
	lpDibBits = (((BYTE _huge *)pbmih) + pbmih->biSize);
	if (pbmih->biSize == sizeof(BITMAPCOREHEADER))
	    lpDibBits += dib_pal_colors(pbmih) * sizeof(RGBTRIPLE); 
	else
	    lpDibBits += dib_pal_colors(pbmih) * sizeof(RGBQUAD); 
	return lpDibBits;
}

void
get_dib_line(BYTE _huge *line, char *preview, int width, int bitcount)
{
int bwidth = ((width + 7) & ~7) >> 3; /* byte width with 1 bit/pixel */
unsigned char omask;
int oroll;
unsigned char c = 0;
int j;
	memset(preview,0xff,bwidth);
	omask = 0x80;
	oroll = 7;
	if (bitcount == 1) {
	    if (isblack[0])
		for (j = 0; j < bwidth ; j++)
			preview[j] = line[j];
	    else
		for (j = 0; j < bwidth ; j++)
			preview[j] = ~line[j];
	    preview[bwidth-1] |= width & 7 ? (1<<(8-(width&7)))-1 : 0;	/* mask for edge of bitmap */
	}
	else {
	    for (j = 0; j < width; j++) {
		switch (bitcount) {
			case 4:
				c = line[j>>1];
				if (!(j&1))
					c >>= 4;
				c = isblack[ c & 0x0f ];
				break;
			case 8:
				c = isblack[ (int)(line[j]) ];
				break;
			case 24:
				c = (line[j*3] < 0xff) || 
				    (line[j*3+1] < 0xff) || 
				    (line[j*3+2] < 0xff);
				break;
		}
		if (c) 
		    preview[j/8] &= ~omask;
		else
		    preview[j/8] |= omask;
		oroll--;
		omask >>= 1;
		if (oroll < 0) {
		    omask = 0x80;
		    oroll = 7;
		}
	    }
	}
}

#define TIFF_BYTE 1
#define TIFF_ASCII 2
#define TIFF_SHORT 3
#define TIFF_LONG 4
#define TIFF_RATIONAL 5

struct rational_s {
	DWORD numerator;
	DWORD denominator;
};

struct ifd_entry_s {
	WORD tag;
	WORD type;
	DWORD length;
	DWORD value;
};

struct tiff_head_s {
	WORD order;
	WORD version;
	DWORD ifd_offset;
	WORD ifd_length;
};

/* write tiff file from DIB bitmap */
/* hglobal is handle to DIB in memory */
void
write_tiff(FILE *f, HGLOBAL hglobal, BOOL tiff4)
{
char *now;
DWORD *strip;
#define IFD_MAX_ENTRY 12
struct tiff_head_s tiff_head;
WORD ifd_length;
DWORD ifd_next;
struct ifd_entry_s ifd_entry[IFD_MAX_ENTRY];
struct ifd_entry_s *pifd_entry;
struct rational_s rational;
DWORD tiff_end, end;
time_t t;
int i;
char *preview;
LPBITMAPINFOHEADER pbmih;
BYTE _huge *lpDibBits;
BYTE _huge *line;
int width;
int height;
int bitcount;
int bwidth;
BOOL soft_extra = FALSE;
BOOL date_extra = FALSE;

	pbmih = (LPBITMAPINFOHEADER)GlobalLock(hglobal);
	lpDibBits = get_dib_bits(pbmih);

	width = (int)pbmih->biWidth;
	height = (int)pbmih->biHeight;
	bitcount = pbmih->biBitCount;
	/* byte width with 1 bit/pixel, rounded up even word */
	bwidth = ((width + 15) & ~15) >> 3;
	height = (int)pbmih->biHeight;

	tiff_end = sizeof(tiff_head);
	tiff_head.order = 0x4949;
	tiff_head.version = 42;
	tiff_head.ifd_offset = tiff_end;

	tiff_end += sizeof(ifd_length);

	if (tiff4)
	    ifd_length = 10;
	else
	    ifd_length = 12;

	tiff_end += ifd_length * sizeof(struct ifd_entry_s) + sizeof(ifd_next);
	ifd_next = 0;
	pifd_entry = &ifd_entry[0];
	if (tiff4) {
	    pifd_entry->tag = 0xff;	/* SubfileType */
	    pifd_entry->type = TIFF_SHORT;
	}
	else {
	    pifd_entry->tag = 0xfe;	/* NewSubfileType */
	    pifd_entry->type = TIFF_LONG;
	}
	pifd_entry->length = 1;
	pifd_entry->value = 0;

	pifd_entry = &ifd_entry[1];
	pifd_entry->tag = 0x100;	/* ImageWidth */
	if (tiff4)
	    pifd_entry->type = TIFF_SHORT;
	else
	    pifd_entry->type = TIFF_LONG;
	pifd_entry->length = 1;
	pifd_entry->value = pbmih->biWidth;

	pifd_entry = &ifd_entry[2];
	pifd_entry->tag = 0x101;	/* ImageLength */
	if (tiff4)
	    pifd_entry->type = TIFF_SHORT;
	else
	    pifd_entry->type = TIFF_LONG;
	pifd_entry->length = 1;
	pifd_entry->value = height;

	pifd_entry = &ifd_entry[3];
	pifd_entry->tag = 0x103;	/* Compression */
	pifd_entry->type = TIFF_SHORT;
	pifd_entry->length = 1;
	pifd_entry->value = 1;		/* no compression */

	pifd_entry = &ifd_entry[4];
	pifd_entry->tag = 0x106;	/* PhotometricInterpretation */
	pifd_entry->type = TIFF_SHORT;
	pifd_entry->length = 1;
	pifd_entry->value = 1;		/* black is zero */

	pifd_entry = &ifd_entry[5];
	pifd_entry->tag = 0x111;	/* StripOffsets */
	pifd_entry->type = TIFF_LONG;
	pifd_entry->length = height;
	pifd_entry->value = tiff_end;
	tiff_end += (pifd_entry->length * sizeof(DWORD));

	pifd_entry = &ifd_entry[6];
	pifd_entry->tag = 0x116;	/* RowsPerStrip */
	pifd_entry->type = TIFF_LONG;
	pifd_entry->length = 1;
	pifd_entry->value = 1;

	pifd_entry = &ifd_entry[7];
	pifd_entry->tag = 0x117;	/* StripByteCounts */
	pifd_entry->type = TIFF_LONG;
	pifd_entry->length = height;
	pifd_entry->value = tiff_end;
	tiff_end += (pifd_entry->length * sizeof(DWORD));

	pifd_entry = &ifd_entry[8];
	pifd_entry->tag = 0x11a;	/* XResolution */
	pifd_entry->type = TIFF_RATIONAL;
	pifd_entry->length = 1;
	pifd_entry->value = tiff_end;
	tiff_end += sizeof(struct rational_s);

	pifd_entry = &ifd_entry[9];
	pifd_entry->tag = 0x11b;	/* YResolution */
	pifd_entry->type = TIFF_RATIONAL;
	pifd_entry->length = 1;
	pifd_entry->value = tiff_end;
	tiff_end += sizeof(struct rational_s);

	if (!tiff4) {
	    pifd_entry = &ifd_entry[10];
	    pifd_entry->tag = 0x131;	/* Software */
	    pifd_entry->type = TIFF_ASCII;
	    pifd_entry->length = strlen(szAppName) + 1;
	    pifd_entry->value = tiff_end;
	    tiff_end += pifd_entry->length;
	    if (tiff_end & 1) { /* pad to word boundary */
	        soft_extra = TRUE;
	        tiff_end++;
	    }

	    pifd_entry = &ifd_entry[11];
	    pifd_entry->tag = 0x132;	/* DateTime */
	    pifd_entry->type = TIFF_ASCII;
	    t = time(NULL);
	    now = ctime(&t);
	    now[strlen(now)-1] = '\0';	/* remove trailing \n */
	    pifd_entry->length = strlen(now)+1;
	    pifd_entry->value = tiff_end;
	    tiff_end += pifd_entry->length;
	    if (tiff_end & 1) { /* pad to word boundary */
	        date_extra = TRUE;
	        tiff_end++;
	    }
	}

	fwrite(&tiff_head, sizeof(tiff_head), 1, f);
	fwrite(&ifd_length, sizeof(ifd_length), 1, f);
	fwrite(&ifd_entry, ifd_length * sizeof(struct ifd_entry_s), 1, f);
	fwrite(&ifd_next, sizeof(ifd_next), 1, f);
	strip = (DWORD *)malloc(height * sizeof(DWORD));
	end = tiff_end;
	for (i=0; i<height; i++) {
		strip[i] = end;		/* strip offsets */
		end += bwidth;
	}
	fwrite(strip, 1, height * sizeof(DWORD), f);
	for (i=0; i<height; i++)
		strip[i] = bwidth;	/* strip byte counts */
	fwrite(strip, 1, height * sizeof(DWORD), f);
	free((void *)strip);
	rational.numerator = (int)xdpi;
	rational.denominator = 1;
	fwrite(&rational, sizeof(rational), 1, f);
	rational.numerator = (int)ydpi;
	rational.denominator = 1;
	fwrite(&rational, sizeof(rational), 1, f);
	if (!tiff4) {
	    fwrite(szAppName, 1, strlen(szAppName)+1, f);
	    if (soft_extra)
	        fputc('\0',f);
	    fwrite(now, 1, strlen(now)+1, f);
	    if (date_extra)
	        fputc('\0',f);
	}

	scan_colors(pbmih);

	preview = (char *) malloc(bwidth);
	memset(preview,0xff,bwidth);

	line = lpDibBits + (dib_bytewidth(pbmih) * pbmih->biHeight);
	/* process each line of bitmap */
	for (i = 0; i < pbmih->biHeight; i++) {
		line -= dib_bytewidth(pbmih);
		get_dib_line(line, preview, width, bitcount);
		fwrite(preview, 1, bwidth, f);
	}
	free(preview);
	GlobalUnlock(hglobal);
}

/* make a PC EPS file with a TIFF Preview */
/* from a PS file and a clipboard bitmap */
void
make_eps_tiff(WORD type)
{
char epsname[MAXSTR];
HGLOBAL hglobal;
char *buffer;
UINT count;
FILE *epsfile;
FILE *tiff_file;
char tiffname[MAXSTR];
BOOL made_dib = FALSE;
struct eps_header_s eps_header;

	if (!OpenClipboard(hwndimg)) {
	    play_sound(SOUND_ERROR);
	    return;
	}
	if (IsClipboardFormatAvailable(CF_DIB))
	    hglobal = GetClipboardData(CF_DIB);
	else if (IsClipboardFormatAvailable(CF_BITMAP)) {
	    /* untested */
	    hglobal = make_dib(); /* convert to DIB format */
	    if (hglobal == (HGLOBAL)NULL) {
	        play_sound(SOUND_ERROR);
	        CloseClipboard();
	        return;
	    }
	    made_dib = TRUE;
	}
	else {
	    play_sound(SOUND_ERROR);
	    CloseClipboard();
	    return;
	}
	
	if ( (tiff_file = gp_open_scratch_file(szScratch, tiffname, "wb")) == (FILE *)NULL) {
	    play_sound(SOUND_ERROR);
	    if (made_dib)
	        GlobalFree(hglobal);
	    CloseClipboard();
	    return;
	}
	write_tiff(tiff_file, hglobal, (type == IDM_MAKEEPST4));
	fclose(tiff_file);
	CloseClipboard();
	if (made_dib)
	    GlobalFree(hglobal);

	/* create EPS file */
	epsname[0] = '\0';
	if (!getfilename(epsname, SAVE, FILTER_EPS, NULL, IDS_TOPICEDIT)) {
	    unlink(tiffname);
	    return;
	}
	epsfile = fopen(epsname,"wb");

	/* write DOS EPS binary header */
	eps_header.id[0] = 0xc5;
	eps_header.id[1] = 0xd0;
	eps_header.id[2] = 0xd3;
	eps_header.id[3] = 0xc6;
	eps_header.ps_begin = sizeof(eps_header);
	fseek(dfile, 0, SEEK_END);
	eps_header.ps_length = ftell(dfile);
	eps_header.mf_begin = 0;
	eps_header.mf_length = 0;
	eps_header.tiff_begin = eps_header.ps_begin + eps_header.ps_length;
	tiff_file = fopen(tiffname,"rb");
	fseek(tiff_file, 0, SEEK_END);
	eps_header.tiff_length = ftell(tiff_file);;
	eps_header.checksum = -1;

	fwrite(&eps_header, sizeof(eps_header), 1, epsfile);
	rewind(dfile);
	pscopy(dfile, epsfile, doc->beginheader, doc->endtrailer);
	
	/* copy tiff file */
	rewind(tiff_file);
	buffer = malloc(COPY_BUF_SIZE);
	if (buffer == (char *)NULL) {
	    play_sound(SOUND_ERROR);
	    fclose(epsfile);
	    unlink(epsname);
	    fclose(tiff_file);
	    unlink(tiffname);
	    return;
	}
        while ( (count = fread(buffer, 1, COPY_BUF_SIZE, tiff_file)) != 0 )
	    fwrite(buffer, 1, count, epsfile);
	free(buffer);
	fclose(tiff_file);
	unlink(tiffname);
	fclose(epsfile);
}


/* write interchange preview to file f */
/* hglobal is handle to DIB in memory */
void
write_interchange(FILE *f, HGLOBAL hglobal)
{
	int i, j;
	char *preview;
	LPBITMAPINFOHEADER pbmih;
	BYTE _huge *lpDibBits;
	BYTE _huge *line;
	int width;
	int height;
	int bitcount;
	int bwidth;
	int lines_per_scan;
	
	pbmih = (LPBITMAPINFOHEADER)GlobalLock(hglobal);
	lpDibBits = get_dib_bits(pbmih);

	width = (int)pbmih->biWidth;
	height = (int)pbmih->biHeight;
	bitcount = pbmih->biBitCount;
	bwidth = ((width + 7) & ~7) >> 3; /* byte width with 1 bit/pixel */

	scan_colors(pbmih);

	preview = (char *) malloc(bwidth);

	lines_per_scan = ((bwidth-1) / 32) + 1;
	fprintf(f,"%%%%BeginPreview: %u %u 1 %u\r\n",width, height, 
	    height*lines_per_scan);

	line = lpDibBits + (dib_bytewidth(pbmih) * pbmih->biHeight);
	/* process each line of bitmap */
	for (i = 0; i < pbmih->biHeight; i++) {
		line -= dib_bytewidth(pbmih);
		get_dib_line(line, preview, width, bitcount);
		fputs("% ",f);
		for (j=0; j<bwidth; j++) {
		    if (j && ((j & 31) == 0))
		        fputs("\r\n% ",f);
		    fputc(hex[15-((preview[j]>>4)&15)],f);
		    fputc(hex[15-((preview[j])&15)],f);
		}
		fputs("\r\n",f);
	}

	fputs("%%EndImage\r\n%%EndPreview\r\n",f);
	free(preview);
	GlobalUnlock(hglobal);
}

/* make an EPSI file with an Interchange Preview */
/* from a PS file and a clipboard bitmap */
void
make_eps_interchange(void)
{
HGLOBAL hglobal;
char epiname[MAXSTR];
FILE *epifile;
BOOL made_dib = FALSE;
	if (!OpenClipboard(hwndimg)) {
	    play_sound(SOUND_ERROR);
	    return;
	}
	if (IsClipboardFormatAvailable(CF_DIB))
	    hglobal = GetClipboardData(CF_DIB);
	else if (IsClipboardFormatAvailable(CF_BITMAP)) {
	    /* untested */
	    hglobal = make_dib(); /* convert to DIB format */
	    if (hglobal == (HGLOBAL)NULL) {
	        play_sound(SOUND_ERROR);
	        CloseClipboard();
	        return;
	    }
	    made_dib = TRUE;
	}
	else {
	    play_sound(SOUND_ERROR);
	    CloseClipboard();
	    return;
	}

	/* create EPI file */
	epiname[0] = '\0';
	if (!getfilename(epiname, SAVE, FILTER_EPI, NULL, IDS_TOPICEDIT)) {
	    play_sound(SOUND_ERROR);
	    CloseClipboard();
	    return;
	}

	if ((epifile = fopen(epiname,"wb")) == (FILE *)NULL) {
	    play_sound(SOUND_ERROR);
	    if (made_dib)
	        GlobalFree(hglobal);
	    CloseClipboard();
	    return;
	}

	rewind(dfile);
	pscopy(dfile, epifile, doc->beginheader, doc->endheader);
	write_interchange(epifile,hglobal);
	pscopy(dfile, epifile, doc->endheader, doc->endtrailer);
	fclose(epifile);
	if (made_dib)
	    GlobalFree(hglobal);
	CloseClipboard();
}


int bbox[4];
BOOL bbflag;

BOOL CALLBACK _export
BoundingBoxDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
static int bboxindex;
int x, y;
char buf[MAXSTR];
    switch(message) {
	case WM_INITDIALOG:
	    bboxindex = 0;
	    LoadString(phInstance, IDS_BBPROMPT, buf, sizeof(buf));
	    SetDlgItemText(hDlg, BB_PROMPT, buf);
	    return TRUE;
	case WM_COMMAND:
	    switch(LOWORD(wParam)) {
		case BB_CLICK:
		    if (!get_cursorpos(&x, &y)) {
			DestroyWindow(hDlg);
			hDlgModeless = 0;
		    }
		    switch(bboxindex) {
			case LLX:
			case URX:
			    bbox[bboxindex] = x;
			    break;
			case URY:
			    bbflag = TRUE;
			case LLY:
			    bbox[bboxindex] = y;
			    break;
		    }
		    bboxindex++;
		    if (bboxindex <= URY) {
	    	        LoadString(phInstance, IDS_BBPROMPT+bboxindex, buf, sizeof(buf));
	    	        SetDlgItemText(hDlg, BB_PROMPT, buf);
			return FALSE;
		    }
		case WM_CLOSE:
		case IDCANCEL:
		    DestroyWindow(hDlg);
		    hDlgModeless = 0;
		    return TRUE;
	    }
    }
    return FALSE;
}

BOOL
get_bbox(void)
{
DLGPROC lpfnBoundingBoxProc;

	bbflag = FALSE;
	bbox[LLX] = bbox[LLY] = bbox[URX] = bbox[URY] = 0;
	if (!page_ready) {
	    gserror(IDS_EPSNOBBOX, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
	    return FALSE;
	}
	lpfnBoundingBoxProc = (DLGPROC)MakeProcInstance((FARPROC)BoundingBoxDlgProc, phInstance);
	hDlgModeless = CreateDialogParam(phInstance, "BoundingBoxDlgBox", hwndimg, lpfnBoundingBoxProc, (LPARAM)NULL);
	while (hDlgModeless) {
	    do_message();	/* wait for bounding box to be obtained */
	}
	FreeProcInstance((FARPROC)lpfnBoundingBoxProc);
	return bbflag;
}


/* At present only allows bounding box to be specified */
void
ps_to_eps(void)
{
char output[MAXSTR];
FILE *f;
char *buffer;
UINT count;
FILE *infile;
time_t t;
char *now;
char text[PSLINELENGTH];
char *comment;
long here;

	LoadString(phInstance, IDS_EPSREAD, output, sizeof(output));
	if (MessageBox(hwndimg, output, szAppName, MB_YESNO | MB_ICONQUESTION)
	    != IDYES) {
	    LoadString(phInstance, IDS_TOPICPSTOEPS, szHelpTopic, sizeof(szHelpTopic));
	    SendMessage(hwndimg, help_message, 0, 0L);
	    return;
	}

	if (!hwndimgchild || !IsWindow(hwndimgchild)) {
	    gserror(IDS_EPSNOBBOX, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
	    return;
	}

	if ((doc != (struct document *)NULL) && (doc->numpages > 1)) {
	    gserror(IDS_EPSONEPAGE, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
	    return;
	}
	if (doc == (struct document *)NULL) {
	    char mess[MAXSTR];
	    LoadString(phInstance, IDS_EPSQPAGES, mess, sizeof(mess));
	    if (MessageBox(hwndimg, mess, szAppName, MB_YESNO | MB_ICONQUESTION) != IDYES)
		return;
	}

	if (!get_bbox()) {
	    play_sound(SOUND_ERROR);
	    return;
	}

	output[0] = '\0';
	if (!getfilename(output, SAVE, FILTER_PS, NULL, IDS_TOPICPSTOEPS))
	    return;

	if ((f = fopen(output, "wb")) == (FILE *)NULL) {
	    play_sound(SOUND_ERROR);
	    return;
	}

	if (doc == (struct document *)NULL) {
	    info_wait(TRUE);
	    fputs("%!PS-Adobe-3.0 EPSF-3.0\r\n",f);
	    /* if this is not a single page document then gsview has just lied */
	    fprintf(f, "%%%%BoundingBox: %u %u %u %u\r\n",
		bbox[LLX],bbox[LLY],bbox[URX],bbox[URY]);
	    fprintf(f,"%%%%Title: %s\r\n",dfname);
	    fprintf(f,"%%%%Creator: %s from %s\r\n",szAppName,dfname);
	    t = time(NULL);
	    now = ctime(&t);
	    now[strlen(now)-1] = '\0';	/* remove trailing \n */
	    fprintf(f,"%%%%CreationDate: %s\r\n",now);
	    fputs("%%Pages: 1\r\n",f);
	    fputs("%%EndComments\r\n",f);

	    fputs("%%Page: 1 1\r\n",f);
	    fprintf(f,"%%BeginDocument: %s\r\n",dfname);

	    /* create buffer for PS file copy */
	    buffer = malloc(COPY_BUF_SIZE);
	    if (buffer == (char *)NULL) {
	        play_sound(SOUND_ERROR);
	        fclose(f);
		unlink(output);
	        return;
	    }

	    infile = fopen(dfname, "rb");
	    if (infile == (FILE *)NULL) {
	        play_sound(SOUND_ERROR);
	        fclose(f);
		unlink(output);
	        return;
	    }

            while ( (count = fread(buffer, 1, COPY_BUF_SIZE, infile)) != 0 ) {
	        fwrite(buffer, 1, count, f);
	    }
	    free(buffer);
	    fclose(infile);

	    fputs("%%EndDocument\r\n",f);
	    fputs("%%Trailer\r\n",f);
	    fclose(f);
	    info_wait(FALSE);
	}
	else {
	    /* document already has DSC comments */
	    info_wait(TRUE);
	    rewind(dfile);
	    if (is_ctrld)
	        fgetc(dfile);
	    fgets(text, PSLINELENGTH, dfile);
	    if (doc->epsf)
	        fputs(text,f);
	    else
	        fputs("%!PS-Adobe-3.0 EPSF-3.0\r\n",f);
	    if ((doc->boundingbox[LLX] != 0) || (doc->boundingbox[LLY] != 0) || 
	        (doc->boundingbox[URX] != 0) || (doc->boundingbox[URY] != 0)) {
	        if ( (comment = pscopyuntil(dfile, f, -1,
			   doc->endheader, "%%BoundingBox:")) != (char *)NULL ) {
		    free(comment);
	        }
	    }
	    fprintf(f, "%%%%BoundingBox: %u %u %u %u\r\n",
		bbox[LLX],bbox[LLY],bbox[URX],bbox[URY]);
	    here = ftell(dfile);
	    pscopy(dfile, f, here, doc->endtrailer);
	    fclose(f);
	    info_wait(FALSE);
	}
}
