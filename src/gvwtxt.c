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

/* gvwtxt.c */
/* text dialog box for Windows GSview */
#include "gvwin.h"

void gs_addmess_update(HWND hwnd);


/* Text Window for Ghostscript Messages */
/* uses MS-Windows multiline edit field */


#define TWLENGTH 4096
#define TWSCROLL 1024
char twbuf[TWLENGTH];
int twend;

extern HWND hwndmess;

#pragma argsused
BOOL CALLBACK _export
TextDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
        case WM_INITDIALOG:
	    gs_addmess_update(hDlg);
            return(TRUE);
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case IDCANCEL:
		    DestroyWindow(hDlg);
                    return(TRUE);
		case TEXTWIN_COPY:
		    {HGLOBAL hglobal;
		    LPSTR p;
		    DWORD result;
		    int start, end;
		    result = SendDlgItemMessage(hDlg, TEXTWIN_MLE, EM_GETSEL, (WPARAM)0, (LPARAM)0);
		    start = LOWORD(result);
		    end   = HIWORD(result);
		    if (start == end) {
			start = 0;
			end = twend;
		    }
		    hglobal = GlobalAlloc(GHND | GMEM_SHARE, end-start+1);
		    if (hglobal == (HGLOBAL)NULL) {
			MessageBeep(-1);
			return(FALSE);
		    }
		    p = GlobalLock(hglobal);
		    if (p == (LPSTR)NULL) {
			MessageBeep(-1);
			return(FALSE);
		    }
		    lstrcpyn(p, twbuf+start, end-start);
		    GlobalUnlock(hglobal);
		    OpenClipboard(hDlg);
		    EmptyClipboard();
		    SetClipboardData(CF_TEXT, hglobal);
		    CloseClipboard();
		    }
                default:
                    return(FALSE);
            }
        default:
            return(FALSE);
    }
}


#ifndef __WIN32__
DLGPROC lpfnTextDlgProc;
#endif
HWND hwnd_text;

/* create modeless dialog box with multiline edit control */
HWND 
gs_showmess_modeless(void)
{
#ifdef __WIN32__
    hwnd_text = CreateDialogParam(phInstance, MAKEINTRESOURCE(IDD_TEXTWIN), HWND_DESKTOP, TextDlgProc, (LPARAM)NULL);
#else
    lpfnTextDlgProc = (DLGPROC)MakeProcInstance((FARPROC)TextDlgProc, phInstance);
    hwnd_text = CreateDialogParam(phInstance, MAKEINTRESOURCE(IDD_TEXTWIN), HWND_DESKTOP, lpfnTextDlgProc, (LPARAM)NULL);
#endif
    return hwnd_text;
}

void
gs_showmess_destroy(void)
{
    if (hwnd_text && IsWindow(hwnd_text))
	DestroyWindow(hwnd_text);
#ifndef __WIN32__
    FreeProcInstance((FARPROC)lpfnTextDlgProc);
#endif
}

/* Add string for Ghostscript message window */
void
gs_addmess_count(char GVFAR *str, int count)
{
LPSTR p;
int i, lfcount;
    /* we need to add \r after each \n, so count the \n's */
    lfcount = 0;
    p = str;
    for (i=0; i<count; i++) {
	if (*p == '\n')
	    lfcount++;
	p++;
    }
    if (count + lfcount >= TWSCROLL)
	return;		/* too large */
    if (count + lfcount + twend >= TWLENGTH-1) {
	/* scroll buffer */
	twend -= TWSCROLL;
	memmove(twbuf, twbuf+TWSCROLL, twend);
    }
    p = twbuf+twend;
    for (i=0; i<count; i++) {
	if (*str == '\n') {
	    *p++ = '\r';
	}
	*p++ = *str++;
    }
    twend += (count + lfcount);
    *(twbuf+twend) = '\0';
}

void
gs_addmess(char GVFAR *str)
{
    gs_addmess_count(str, lstrlen(str));
}

void
gs_addmess_update(HWND hwnd)
{
HWND hwndtext = GetDlgItem(hwndmess, TEXTWIN_MLE);
    DWORD linecount;
    SendMessage(hwndtext, WM_SETREDRAW, FALSE, 0);
    SetDlgItemText(hwnd, TEXTWIN_MLE, twbuf);
#ifdef __WIN32__
    /* EM_SETSEL, followed by EM_SCROLLCARET doesn't work */
    linecount = SendDlgItemMessage(hwnd, TEXTWIN_MLE, EM_GETLINECOUNT, (WPARAM)0, (LPARAM)0);
    SendDlgItemMessage(hwnd, TEXTWIN_MLE, EM_LINESCROLL, (WPARAM)0, (LPARAM)linecount-18);
#else
    linecount = SendDlgItemMessage(hwnd, TEXTWIN_MLE, EM_GETLINECOUNT, (WPARAM)0, (LPARAM)0);
    SendDlgItemMessage(hwnd, TEXTWIN_MLE, EM_LINESCROLL, (WPARAM)0, MAKELPARAM(linecount-18, 0));
#endif
    SendMessage(hwndtext, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(hwndtext, (LPRECT)NULL, TRUE);
    UpdateWindow(hwndtext);
}
