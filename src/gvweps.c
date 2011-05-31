/* Copyright (C) 1993-1996, Russell Lang.  All rights reserved.
  
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

/* gvweps.c */
/* EPS file manipulation module for Windows GSview */

#include "gvwin.h"


static HGLOBAL get_bitmap_hglobal;

LPBITMAP2
get_bitmap()
{
    get_bitmap_hglobal = (*gsdll.copy_dib)(gsdll.device);
    if (get_bitmap_hglobal == (HGLOBAL)NULL) {
	message_box("not enough memory to copy bitmap", 0);
	return NULL;
    }
    return (LPBITMAP2)GlobalLock(get_bitmap_hglobal);
}

void
release_bitmap()
{
	if (get_bitmap_hglobal) {
	    GlobalUnlock(get_bitmap_hglobal);
	    GlobalFree(get_bitmap_hglobal);
	    get_bitmap_hglobal = 0;
	}
}

long
hugewrite(HFILE hf, const void _huge *hpvBuffer, long cbBuffer)
{
	    return _hwrite(hf, hpvBuffer, cbBuffer);
}

/* convert the display bitmap to a metafile picture */
HMETAFILE
make_metafile(void)
{
HDC hdc;
HMETAFILE hmf;
LPBITMAP2 pbm;
LPBITMAPINFOHEADER pbmih;
BYTE _huge *lpDibBits;
	if ( (pbm = get_bitmap()) == (LPBITMAP2)NULL) {
	    return (HMETAFILE)0;
	}
	pbmih = (LPBITMAPINFOHEADER)pbm;
	lpDibBits = ((BYTE _huge *)pbmih) + pbmih->biSize;
	if (pbmih->biSize == sizeof(BITMAPCOREHEADER))
	    lpDibBits += dib_pal_colors((LPBITMAP2)pbmih) * sizeof(RGBTRIPLE); 
	else
	    lpDibBits += dib_pal_colors((LPBITMAP2)pbmih) * sizeof(RGBQUAD); 
	/* now make a Metafile from it */
	hdc = CreateMetaFile(NULL);
	SetWindowOrg(hdc, 0, 0);
	SetWindowExt(hdc, (int)pbmih->biWidth, (int)pbmih->biHeight);
	StretchDIBits(hdc, 0, 0, (int)pbmih->biWidth, (int)pbmih->biHeight,
	    0, 0, (int)pbmih->biWidth, (int)pbmih->biHeight,
	    (void FAR *)lpDibBits, (LPBITMAPINFO)pbmih,
	    DIB_RGB_COLORS, SRCCOPY);
	hmf = CloseMetaFile(hdc);
	release_bitmap();
	return hmf;
}

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
#ifdef __WIN32__
UINT size;
LPCVOID lpmf;
#else
LPSTR lpmf;
#endif

	if ( (hmf = make_metafile()) == (HMETAFILE)NULL ) {
	    play_sound(SOUND_ERROR);
	    return;
	}

	/* get memory handle to metafile */
#ifdef __WIN32__
	size = GetMetaFileBitsEx(hmf, 0, NULL);
	hglobal = GlobalAlloc(GHND, size);
	lpmf = GlobalLock(hglobal);
	GetMetaFileBitsEx(hmf, size, lpmf);
	GlobalUnlock(hglobal);
#else
	hglobal = GetMetaFileBits(hmf);
#endif

	/* create buffer for PS file copy */
	buffer = malloc(COPY_BUF_SIZE);
	if (buffer == (char *)NULL) {
	    GlobalFree(hglobal);
	    gserror(IDS_BADEPS, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
	    return;
	}

	/* create EPS file */
	epsname[0] = '\0';
	if (!get_filename(epsname, TRUE, FILTER_EPS, 0, IDS_TOPICPREVIEW)) {
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
	fseek(psfile.file, 0, SEEK_END);
	eps_header.ps_length = ftell(psfile.file);
	eps_header.mf_begin = eps_header.ps_begin + eps_header.ps_length;
#ifdef __WIN32__
	eps_header.mf_length = size;
#else
	eps_header.mf_length = GlobalSize(hglobal);
#endif
	eps_header.tiff_begin = 0;
	eps_header.tiff_length = 0;
	eps_header.checksum = -1;
	_lwrite(hfEPS, (void _huge *)&eps_header, sizeof(eps_header));

	/* copy PS file */
	rewind(psfile.file);
	do {
	    count = fread(buffer, 1, COPY_BUF_SIZE, psfile.file);
	    _lwrite(hfEPS, buffer, count);
	} while (count != 0);
	free(buffer);

	/* copy metafile */
	lpmf = GlobalLock(hglobal);
	hugewrite(hfEPS, lpmf, eps_header.mf_length);
	GlobalUnlock(hglobal);
	GlobalFree(hglobal);

	_lclose(hfEPS);
}
