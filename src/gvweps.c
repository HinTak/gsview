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

/* gvweps.c */
/* EPS file manipulation module for Windows GSview */

#include "gvwin.h"
#include "gvceps.h"


HGLOBAL make_dib(void);  /* in gvwclip.c */

static HGLOBAL get_bitmap_hglobal;
static BOOL get_bitmap_made_dib = FALSE;

LPBITMAP2
get_bitmap()
{
	get_bitmap_hglobal = 0;
	get_bitmap_made_dib = FALSE;
	if (!OpenClipboard(hwndimg))
	    return NULL;
	if (IsClipboardFormatAvailable(CF_DIB))
	    get_bitmap_hglobal = GetClipboardData(CF_DIB);
	else if (IsClipboardFormatAvailable(CF_BITMAP)) {
	    /* untested */
	    get_bitmap_hglobal = make_dib(); /* convert to DIB format */
	    if (get_bitmap_hglobal == (HGLOBAL)NULL) {
	        CloseClipboard();
		return NULL;
	    }
	    get_bitmap_made_dib = TRUE;
	}
	else {
	    CloseClipboard();
	    return NULL;
	}
	return (LPBITMAP2)GlobalLock(get_bitmap_hglobal);
}

void
release_bitmap()
{
	GlobalUnlock(get_bitmap_hglobal);
	if (get_bitmap_made_dib)
	    GlobalFree(get_bitmap_hglobal);
	CloseClipboard();
}

#ifdef __WIN32__
long
hugewrite(HFILE hf, const void _huge *hpvBuffer, long cbBuffer)
{
	    return _hwrite(hf, hpvBuffer, cbBuffer);
}
#else
/* Write data to file - blocks > 64k permitted */
long
hugewrite(HFILE hf, const void _huge *hpvBuffer, long cbBuffer)
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
#endif

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
	if (!get_filename(epsname, TRUE, FILTER_EPS, 0, IDS_TOPICEDIT)) {
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
