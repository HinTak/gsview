/*
 * gsview.c -- Main module of GSVIEW.EXE, a graphical interface for 
 *             MS-Windows Ghostscript
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

char szAppName[MAXSTR];			/* application name - for title bar */
const char szClassName[] = "gsview_class";
const char szScratch[] = "gsview";	/* temporary filename prefix */

HWND hwndimg;			/* gsview main window */
HWND hDlgModeless;		/* any modeless dialog box */
HWND hwndtext;			/* gswin text window */
HWND hwndimgchild;		/* gswin image child window */
HINSTANCE phInstance;		/* instance of gsview */
HINSTANCE gswin_hinst;		/* instance of gswin */
int bitmap_scrollx=0;		/* offset from bitmap to origin of child window */
int bitmap_scrolly=0;
int bitmap_width;		/* size of gswin bitmap in pixels */
int bitmap_height;

/* these can be saved in the INI file */
char szGSwin[128];		/* command to invoke gswin */
POINT img_origin;		/* gsview window origin */
POINT img_size;			/* gsview window size */
BOOL quick;			/* use quick opening (don't reload gswin) */
BOOL settings;			/* save settings on exit */
BOOL button_show;		/* show buttons bar */
int media;			/* IDM_LETTER etc. */
int user_width, user_height;	/* User Defined media size */
BOOL epsf_clip;			/* make bitmap size of epsf bounding box */
BOOL epsf_warn;			/* write warning messages if operators incompatible with EPS are used */
BOOL redisplay;			/* redisplay on resize */
int orientation;		/* IDM_PORTRAIT, IDM_LANDSCAPE etc. */
BOOL swap_landscape;		/* swap IDM_LANDSCAPE & IDM_SEASCAPE */
float xdpi, ydpi;		/* resolution of gswin bitmap */
UINT timeout;			/* default timeout period in 1 sec units */
BOOL save_dir;			/* remember current directory for next time */
char device_name[32];		/* printer name */
char device_resolution[32];	/* printer resolution */

struct sound_s sound[NUMSOUND] = {
	{"SoundOutputPage", IDS_SNDPAGE, ""},
	{"SoundNoPage", IDS_SNDNOPAGE, BEEP},
	{"SoundNoNumbering", IDS_SNDNONUMBER, ""},
	{"SoundNotOpen", IDS_SNDNOTOPEN, ""},
	{"SoundError", IDS_SNDERROR, BEEP},
	{"SoundTimeout", IDS_SNDTIMEOUT, ""},
	{"SoundStart", IDS_SNDSTART, ""},
	{"SoundExit", IDS_SNDEXIT, ""},
};


/* initialised in init.c */
BOOL is_win31 = FALSE;		/* To allow selective use of win 3.1 features */
char szHelpName[MAXSTR];	/* buffer for building help filename */
char szHelpTopic[48];		/* topic for OFN_SHOWHELP */
UINT help_message;		/* message sent by OFN_SHOWHELP */
HMENU hmenu;			/* main menu */
HACCEL haccel;			/* menu accelerators */
HCURSOR hcWait;
POINT img_offset;		/* offset to gswin child window */
POINT info_file;		/* position of file information */
POINT info_page;		/* position of page information */
RECT  info_rect;		/* position and size of brief info area */
RECT  info_coord;		/* position and size of coordinate information */
RECT  button_rect;		/* position and size of button area */

BOOL prev_in_child;		/* true if cursor previously in gswin child window */
BOOL waiting = FALSE;		/* true when 'wait' to be displayed in info area */
BOOL page_ready = FALSE;	/* true when gswin has sent an OUTPUT_PAGE and is waiting for NEXT_PAGE */
BOOL saved = FALSE;		/* true if interpreter state currently saved in /gssave */
BOOL epsf_clipped;		/* clipping this page? */
int page_skip = 5;		/* number of pages to skip in IDM_NEXTSKIP or IDM_PREVSKIP */
BOOL debug = FALSE;		/* /D command line option used */
HINSTANCE hlib_mmsystem;	/* DLL containing sndPlaySound function */
FPSPS lpfnSndPlaySound;		/* pointer to sndPlaySound function if loaded */

/* timer used for open, close, display & print timeouts */
BOOL bTimeout;			/* true if timeout occured */
BOOL bTimerSet;			/* true if TIMER running */
#define ID_MYTIMER 1
UINT timeout_count;

/* document manipulation */
struct document *doc;	/* DSC structure.  NULL if not DSC */
int pagenum;		/* current page number */
char dfname[MAXSTR];	/* name of selected document file */
char efname[MAXSTR];	/* name of temporary file containing PS extracted 
                           from DOS EPS file */
FILE *dfile;		/* selected file */
FILE *cfile;		/* command file (pipe) */
BOOL is_ctrld;		/* TRUE if DSC except for ctrl+D at start of file */
int preview;		/* preview type IDS_EPSF, IDS_EPSI, etc. */
struct page_list_s page_list;	/*  page selection for print/extract */

/* local functions */
BOOL draw_button(DRAWITEMSTRUCT FAR *lpdis);
BOOL in_child_client_area(void);
BOOL in_client_area(void);
BOOL in_info_area(void);
void info_paint(HWND);
void gsview_close(void);
int gsview_command(WORD);
BOOL not_open(void);
BOOL not_dsc(void);

int PASCAL 
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int cmdShow)
{
	MSG msg;

	/* copy the hInstance into a variable so it can be used */
	phInstance = hInstance;

	LoadString(hInstance, IDS_TITLE, szAppName, sizeof(szAppName));
	if (hPrevInstance) {
	    /* don't run more than one copy */
	    /* because we can't run more than one Ghostscript */
	    gsview_init0(lpszCmdLine);
	    return FALSE;
	}

	gsview_init1(lpszCmdLine);
	ShowWindow(hwndimg, cmdShow);
	
	while (GetMessage(&msg, (HWND)NULL, 0, 0)) {
	    if ((hDlgModeless == 0) || !IsDialogMessage(hDlgModeless, &msg)) {
	        if (!TranslateAccelerator(hwndimg, haccel, &msg)) {
		    TranslateMessage(&msg);
		    DispatchMessage(&msg);
	        }
	    }
	}

	play_sound(SOUND_EXIT);
	gsview_close();
 	WinHelp(hwndimg,szHelpName,HELP_QUIT,(DWORD)NULL);
	if (is_win31 && (hlib_mmsystem != (HINSTANCE)NULL))
		FreeLibrary(hlib_mmsystem);
	return 0;
}


/* parent overlapped window */
LRESULT CALLBACK _export
WndImgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
RECT rect;

    if (message == WM_GSVIEW) {
	switch(wParam) {
	    case HWND_TEXT:
		/* lParam = handle to Ghostscript Borland EasyWin window */
		hwndtext = (HWND)lParam;
		break;
	    case HWND_IMGCHILD:
		/* lParam = handle to Ghostscript image child window */
		hwndimgchild = (HWND)lParam;
		if (hwndimgchild && IsWindow(hwndimgchild)) {
		    SetClassCursor(hwndimgchild, LoadCursor((HINSTANCE)NULL, IDC_CROSS));
		    GetClientRect(hwnd, &rect);
		    SetWindowPos(hwndimgchild, (HWND)NULL, rect.left+img_offset.x, rect.top+img_offset.y,
			rect.right-img_offset.x, rect.bottom-img_offset.y, 
			SWP_NOZORDER | SWP_NOACTIVATE);
		}
		break;
	    case GSWIN_CLOSE:
		/* something is closing gswin */
		gswin_hinst = (HINSTANCE)NULL;
		hwndimgchild = (HWND)NULL;
		hwndtext = (HWND)NULL;
		bitmap_scrollx = bitmap_scrolly = 0;
		page_ready = FALSE;
		saved = FALSE;
		pipeclose();
		clear_timer();
		info_wait(FALSE);
		break;
	    case OUTPUT_PAGE:
		/* showpage has just been called */
		clear_timer();
		play_sound(SOUND_PAGE);
		if (IsIconic(hwndimg))    /* useless as an Icon so fix it */
		    ShowWindow(hwndimg, SW_SHOWNORMAL);
		if ( !IsIconic(hwndimg) ) {  /* redraw child window */
		    if (hwndimgchild && IsWindow(hwndimgchild)) {
			/* don't erase background - the bitmap will cover it anyway */
			InvalidateRect(hwndimgchild, (LPRECT)NULL, FALSE);
			UpdateWindow(hwndimgchild);
		    }
		}
		page_ready = TRUE;
		info_wait(FALSE);
		break;
	    case SYNC_OUTPUT:
		/* time to redraw window */
		if ( !IsIconic(hwndimg) ) {  /* redraw child window */
		    if (hwndimgchild && IsWindow(hwndimgchild)) {
			/* don't erase background - the bitmap will cover it anyway */
			InvalidateRect(hwndimgchild, (LPRECT)NULL, FALSE);
			UpdateWindow(hwndimgchild);
		    }
		}
		break;
	    case SCROLL_POSITION:
		/* User scrolled image window.  
		 * lParam = offsets to top left of image window
		 * we use these to display coordinates */
		bitmap_scrollx = LOWORD(lParam);
		bitmap_scrolly = HIWORD(lParam);
		InvalidateRect(hwndimg, &info_coord, FALSE);
		UpdateWindow(hwndimg);
		break;
	    case PIPE_REQUEST:
		piperequest();
		break;
	    default:
		gserror(0, "Unknown Message", MB_ICONEXCLAMATION, -1);
	}
	return 0;
    }
    else if (message == help_message) {
	WinHelp(hwndimg,szHelpName,HELP_KEY,(DWORD)szHelpTopic);
	return 0;
    } else
    switch(message) {
	case WM_CREATE:
		hwndimg = hwnd;
		gsview_create();
		/* Enable Drag Drop */
		if (is_win31)
			DragAcceptFiles(hwnd, TRUE);
		break;
	case WM_DESTROY:
		/* disable Drag Drop */
		if (is_win31)
			DragAcceptFiles(hwnd, FALSE);
		gsview_close();
		PostQuitMessage(0);
		break;
	case WM_ENDSESSION:
		if (wParam)
		    gsview_close();
		return 0;
	case WM_TIMER:
		if (wParam == ID_MYTIMER) {
		    timeout_count--;
		    if (timeout_count <= 0) {
			clear_timer();
			bTimeout = TRUE;
			gserror(IDS_TIMEOUT, NULL, MB_ICONINFORMATION, SOUND_TIMEOUT);
			info_wait(FALSE);
		    }
		}
		break;
	case WM_DROPFILES:
		if (is_win31) {
		    LPSTR szFile;
		    int i, cFiles, length;
		    HDROP hdrop = (HDROP)wParam;
		    cFiles = DragQueryFile(hdrop, 0xffff, (LPSTR)NULL, 0);
		    for (i=0; i<cFiles; i++) {
			length = DragQueryFile(hdrop, i, (LPSTR)NULL, 0);
			szFile = GlobalAllocPtr(GHND, length+1);
			if (szFile) {
				DragQueryFile(hdrop, i, szFile, MAXSTR);
				/* it doesn't work if we call gsview_display directly */
				PostMessage(hwnd, WM_COMMAND, IDM_DROP, (LPARAM)szFile);
			}
		    }
		    DragFinish(hdrop);
		}
		break;
	case WM_INITMENU:
		if (hmenu == (HMENU)wParam) {
		    HMENU hmenuedit = GetSubMenu(hmenu,1);
		    if (hwndimgchild  && IsWindow(hwndimgchild))
			EnableMenuItem(hmenu, IDM_COPYCLIP, MF_ENABLED);
		    else
			EnableMenuItem(hmenu, IDM_COPYCLIP, MF_DISABLED | MF_GRAYED);
		    if (OpenClipboard(hwnd)) {
			if (IsClipboardFormatAvailable(CF_DIB))
			    EnableMenuItem(hmenu, IDM_PASTETO, MF_ENABLED);
			else
			    EnableMenuItem(hmenu, IDM_PASTETO, MF_DISABLED | MF_GRAYED);
			if (IsClipboardFormatAvailable(CF_DIB) || 
			    IsClipboardFormatAvailable(CF_BITMAP)) 
			    EnableMenuItem(hmenu, IDM_CONVERT, MF_ENABLED);
			else
			    EnableMenuItem(hmenu, IDM_CONVERT, MF_DISABLED | MF_GRAYED);
			/* Make EPS sub menu */
			if ((IsClipboardFormatAvailable(CF_DIB) ||
			     IsClipboardFormatAvailable(CF_BITMAP)) 
			    && (doc != (struct document *)NULL) && doc->epsf)
			    EnableMenuItem(hmenuedit, 5, MF_BYPOSITION | MF_ENABLED);
			else
			    EnableMenuItem(hmenuedit, 5, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
			/* Extract EPS sub menu */
			if ( (preview == IDS_EPST) || (preview == IDS_EPSW) )
			    EnableMenuItem(hmenuedit, 6, MF_BYPOSITION | MF_ENABLED);
			else
			    EnableMenuItem(hmenuedit, 6, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
			CloseClipboard();
		    }
		    return 0;
		}
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDM_DROP) {
		    HGLOBAL hglobal;
		    char buf[MAXSTR];
		    if (lstrlen((LPSTR)lParam) < sizeof(buf))
			lstrcpy(buf, (LPSTR)lParam);
		    else
			buf[0] = '\0';
		    hglobal = (HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(lParam)));
		    GlobalUnlock(hglobal);
		    GlobalFree(hglobal);
		    if (strnicmp(buf,"/P ",3)==0) {
		        gsview_selectfile(buf+3);
		        gsview_print(FALSE);
		    }
		    else if (strnicmp(buf,"/F ",3)==0) {
		        gsview_selectfile(buf+3);
		        gsview_print(TRUE);
		    }
		    else
		        gsview_displayfile(buf);
		}
		else
		     gsview_command(LOWORD(wParam));
		return 0;
	case WM_KEYDOWN:
	case WM_KEYUP:
		/* pass on key presses so that child window scroll bars work */
		if (hwndimgchild && IsWindow(hwndimgchild)) {
			SendMessage(hwndimgchild, message, wParam, lParam);
			return 0;
		}
		break;
	case WM_SIZE:
		/* make child window fill client area */
		if (wParam != SIZE_MINIMIZED  && hwndimgchild !=(HWND)NULL && IsWindow(hwndimgchild))
		    SetWindowPos(hwndimgchild, (HWND)NULL, img_offset.x, img_offset.y,
			LOWORD(lParam)-img_offset.x, HIWORD(lParam)-img_offset.y, 
			SWP_NOZORDER | SWP_NOACTIVATE);
		/* save window size for INIFILE */
		if (wParam == SIZE_RESTORED) {
			GetWindowRect(hwnd,&rect);
			img_size.x = rect.right-rect.left;
			img_size.y = rect.bottom-rect.top;
		}
		return 0;
	case WM_MOVE:
		/* save window position for INIFILE */
		if (!IsIconic(hwnd) && !IsZoomed(hwnd)) {
			GetWindowRect(hwnd,&rect);
			img_origin.x = rect.left;
			img_origin.y = rect.top;
		}
		return 0;
	case WM_SETCURSOR:
		/* if waiting, display hourglass cursor over our window */
		if (waiting) {
		    if (hwndimgchild && IsWindow(hwndimgchild)) {
			if (in_child_client_area() || in_info_area() || (LOWORD(lParam)==HTMENU)) {
			    SetCursor(hcWait);
			    return TRUE;
		        }
		    }
		    else {
			    SetCursor(hcWait);
			    return TRUE;
		    }
		}
		/* track cursor and display coordinates if in child window */
		if (hwndimgchild && IsWindow(hwndimgchild)) {
		    if (in_child_client_area() || prev_in_child) {
			/* update coordinate info */
			InvalidateRect(hwndimg, &info_coord, FALSE);
			UpdateWindow(hwndimg);
		    }
		    prev_in_child = in_child_client_area();
		}
		break;
	case WM_PARENTNOTIFY:
		if (hDlgModeless && (wParam == WM_LBUTTONDOWN))
		    if (in_child_client_area())
		        SendMessage(hDlgModeless, WM_COMMAND, BB_CLICK, lParam);
		break;
	case WM_PAINT:
		info_paint(hwnd);
		return 0;
	case WM_MEASUREITEM:
		return 1;
	case WM_DRAWITEM:
		return draw_button((DRAWITEMSTRUCT FAR *)lParam);
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

/* return TRUE if button drawn */
BOOL
draw_button(DRAWITEMSTRUCT FAR *lpdis)
{
HBRUSH hbrush;
HPEN hpen_highlight, hpen_shadow, hpen_old;
HDC hdc = lpdis->hDC;
RECT rect;
HICON hicon;
HBITMAP hbitmap_old, hbitmap;
BITMAP bm;
int i;
char buf[20];
	rect = lpdis->rcItem;
	if (lpdis->CtlType != ODT_BUTTON)
	    return FALSE;
	switch (lpdis->itemAction) {
	    case ODA_DRAWENTIRE:
		if ((hbitmap = LoadBitmap(phInstance,MAKEINTRESOURCE(lpdis->CtlID)))
		  != (HBITMAP)NULL) {
		    HDC hdcsrc = CreateCompatibleDC(hdc);
		    hbitmap_old = SelectObject(hdcsrc,hbitmap);
		    GetObject(hbitmap, sizeof(BITMAP),&bm);
		    if ( (rect.right-rect.left > bm.bmWidth) ||
			 (rect.bottom-rect.top > bm.bmHeight) ) {
		        hbrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
		        FillRect(hdc, &rect, hbrush);
		        DeleteBrush(hbrush);
		    }
		    BitBlt(hdc, (rect.left+rect.right-bm.bmWidth)/2,
		       (rect.top+rect.bottom-bm.bmHeight)/2,
		       bm.bmWidth,bm.bmHeight,hdcsrc,0,0,SRCCOPY);
		    SelectObject(hdcsrc,hbitmap_old);
		    DeleteObject(hbitmap);
		    DeleteDC(hdcsrc);
		}
		else {
		    hbrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
		    FillRect(hdc, &rect, hbrush);
		    DeleteBrush(hbrush);
		    if ((i = LoadString(phInstance, lpdis->CtlID, buf, sizeof(buf)))
		        != 0) {
			DWORD dw = GetTextExtent(hdc, buf, i);
			SetBkMode(hdc, TRANSPARENT);
			TextOut(hdc, (rect.left+rect.right-LOWORD(dw))/2,
			    (rect.top+rect.bottom-HIWORD(dw))/2, buf, i);
		    }
		    else if ( (hicon = LoadIcon(phInstance, MAKEINTRESOURCE(lpdis->CtlID)))
		        != (HICON)NULL )  {
		        DrawIcon(hdc, (rect.left+rect.right-32)/2, 
		            (rect.top+rect.bottom-32)/2, hicon);
		        DestroyIcon(hicon);
		    }
		}
		hpen_old = SelectPen(hdc, GetStockObject(BLACK_PEN));
		MoveTo(hdc, rect.left, rect.top);
		LineTo(hdc, rect.right-1, rect.top);
		LineTo(hdc, rect.right-1, rect.bottom-1);
		LineTo(hdc, rect.left, rect.bottom-1);
		LineTo(hdc, rect.left, rect.top-1);
		SelectPen(hdc, hpen_old);
		/* fall thru */
	    case ODA_FOCUS:
	    case ODA_SELECT:
		if (lpdis->itemState & ODS_SELECTED) {
		    hpen_highlight = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
		    hpen_shadow = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
		}
		else {
		    hpen_highlight = CreatePen(PS_SOLID, 1, is_win31 ? GetSysColor(COLOR_BTNHIGHLIGHT) : RGB(255,255,255));
		    hpen_shadow = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
		}
		hpen_old = SelectPen(hdc, hpen_highlight);
		MoveTo(hdc, rect.left+1, rect.bottom-3);
		LineTo(hdc, rect.left+1, rect.top+1);
		LineTo(hdc, rect.right-2, rect.top+1);
		MoveTo(hdc, rect.right-3, rect.top+2);
		LineTo(hdc, rect.left+2, rect.top+2);
		LineTo(hdc, rect.left+2, rect.bottom-4);
		SelectPen(hdc, hpen_shadow);
		MoveTo(hdc, rect.left+1, rect.bottom-2);
		LineTo(hdc, rect.right-2, rect.bottom-2);
		LineTo(hdc, rect.right-2, rect.top+1);
		MoveTo(hdc, rect.right-3, rect.top+2);
		LineTo(hdc, rect.right-3, rect.bottom-3);
		LineTo(hdc, rect.left+2, rect.bottom-3);
		SelectPen(hdc, hpen_old);
		DeleteObject(hpen_highlight);
		DeleteObject(hpen_shadow);
	        return TRUE;
	}
	return FALSE;
}

/* returns true if cursor in client area of Ghostscript image window */
BOOL
in_child_client_area()
{
RECT rect;
POINT pt;
HWND hwnd;
        GetCursorPos(&pt);
	hwnd = WindowFromPoint(pt);
	if ((hwnd != hwndimg) && !IsChild(hwndimg,hwnd))
		return 0;
        GetClientRect(hwndimgchild, &rect);
        ScreenToClient(hwndimgchild, &pt);
        return PtInRect(&rect, pt);
}

/* returns true if cursor in client area of Ghostview window */
BOOL
in_client_area()
{
RECT rect;
POINT pt;
HWND hwnd;
        GetCursorPos(&pt);
	hwnd = WindowFromPoint(pt);
	if ((hwnd != hwndimg) && !IsChild(hwndimg,hwnd))
		return 0;
        GetClientRect(hwndimg, &rect);
        ScreenToClient(hwndimg, &pt);
        return PtInRect(&rect, pt);
}

/* returns true if cursor in info area or button area of Ghostview windows */
BOOL
in_info_area()
{
RECT rect;
POINT pt;
HWND hwnd;
        GetCursorPos(&pt);
	hwnd = WindowFromPoint(pt);
	if ((hwnd != hwndimg) && !IsChild(hwndimg,hwnd))
		return 0;
        ScreenToClient(hwndimg, &pt);

        GetClientRect(hwndimg, &rect);
	rect.bottom = img_offset.y;
	if (PtInRect(&rect, pt))
		return TRUE;
        GetClientRect(hwndimg, &rect);
	rect.right = img_offset.x;
        return PtInRect(&rect, pt);
}

BOOL
get_cursorpos(int *x, int *y)
{
RECT rect;
POINT pt;
    if (hwndimgchild && IsWindow(hwndimgchild)) {
	GetClientRect(hwndimgchild, &rect);
	GetCursorPos(&pt);
	ScreenToClient(hwndimgchild, &pt);
	if (PtInRect(&rect, pt)) {
	    *x = (int)((bitmap_scrollx+pt.x)*72.0/xdpi 
		+ (epsf_clipped ? doc->boundingbox[LLX] : 0));
	    *y = (int)(((bitmap_height-1)-(bitmap_scrolly+pt.y))*72.0/ydpi
		+ (epsf_clipped ? doc->boundingbox[LLY] : 0));
	    return TRUE;
	}
    }
    return FALSE;
}

/* paint brief info area */
void
info_paint(HWND hwnd)
{
HDC hdc;
PAINTSTRUCT ps;
RECT rect;
int i;
char buf[MAXSTR];
char fmt[MAXSTR];
int x, y;
	hdc = BeginPaint(hwnd, &ps);
	SetBkMode(hdc, TRANSPARENT);
	if (info_rect.bottom) {
	    GetClientRect(hwnd, &rect);
	    rect.top = 0;
	    rect.left = info_rect.left;
	    rect.bottom = info_rect.bottom;
	    FillRect(hdc, &rect, GetStockObject(LTGRAY_BRUSH));
	    SelectPen(hdc, GetStockObject(BLACK_PEN));
	    MoveTo(hdc, rect.left, rect.bottom);
	    LineTo(hdc, rect.right, rect.bottom);
	}
	if (button_rect.right) {
	    GetClientRect(hwnd, &rect);
	    rect.top = button_rect.top;
	    rect.left = button_rect.left;
	    rect.right = button_rect.right;
	    FillRect(hdc, &rect, GetStockObject(LTGRAY_BRUSH));
	    SelectPen(hdc, GetStockObject(BLACK_PEN));
	    MoveTo(hdc, rect.right, rect.top);
	    LineTo(hdc, rect.right, rect.bottom);
	}
	/* write file information */
	if (dfname[0] != '\0') {
	    i = LoadString(phInstance, IDS_FILE, buf, sizeof(buf));
	    GetFileTitle(dfname, buf+i, sizeof(buf)-i);
	    TextOut(hdc, info_file.x, info_file.y, buf, strlen(buf));
	    if (waiting) {
		i = LoadString(phInstance, IDS_WAIT, buf, sizeof(buf));
		TextOut(hdc, info_page.x, info_page.y, buf, strlen(buf));
	    }
	    else {
	      if (doc!=(struct document *)NULL) {
		int n = map_page(pagenum - 1);
		i = LoadString(phInstance, IDS_PAGEINFO, fmt, sizeof(fmt));
		if (doc->pages)
		    sprintf(buf, fmt, doc->pages[n].label ? doc->pages[n].label : " ",pagenum,  doc->numpages);
		else
		    sprintf(buf, fmt, " " ,pagenum,  doc->numpages);
		TextOut(hdc, info_page.x, info_page.y, buf, strlen(buf));
	      }
	      else {
		if (is_pipe_done())
		    i = LoadString(phInstance, IDS_NOMORE, buf, sizeof(buf));
		else {
		    i = LoadString(phInstance, IDS_PAGE, buf, sizeof(buf));
		    sprintf(buf+i, "%d", pagenum);
		}
		TextOut(hdc, info_page.x, info_page.y, buf, strlen(buf));
	      }
	      /* show coordinate */
	      if (get_cursorpos(&x, &y)) {
		    sprintf(buf,"%d, %d", x, y);
		    SetTextAlign(hdc, TA_RIGHT);
		    TextOut(hdc, info_coord.right-1, info_coord.top, buf, strlen(buf));
	      }
	    }
	}
	else {
	    i = LoadString(phInstance, IDS_NOFILE, buf, sizeof(buf));
	    TextOut(hdc, info_file.x, info_file.y, buf, strlen(buf));
	    if (waiting) {
		i = LoadString(phInstance, IDS_WAIT, buf, sizeof(buf));
		TextOut(hdc, info_page.x, info_page.y, buf, strlen(buf));
	    }
	}
	EndPaint(hwnd, &ps);
}

/* subclass button WndProc to give focus back to parent window */
LRESULT CALLBACK _export
MenuButtonProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
	    case WM_LBUTTONUP:
		{
		RECT rect;
		POINT pt;
		GetWindowRect(hwnd, &rect);
		GetCursorPos(&pt);
		if (PtInRect(&rect, pt))
			SendMessage(GetParent(hwnd), WM_COMMAND, GetWindowID(hwnd), 0L);
		SetFocus(GetParent(hwnd));
		}
		break;
	}
	return CallWindowProc(lpfnButtonWndProc, hwnd, message, wParam, lParam);
}


void
play_sound(int num)
{
	if (strlen(sound[num].file)==0)
		return;
	if (!is_win31 || (strcmp(sound[num].file,BEEP)==0)) {
		MessageBeep(-1);
		return;
	}
	if (is_win31) {
		if (lpfnSndPlaySound != (FPSPS)NULL) 
   		    lpfnSndPlaySound(sound[num].file, SND_SYNC);
		else
		    MessageBeep(-1);
		return;
	}
}

BOOL
set_timer(UINT period)
{
	timeout_count = period;
	bTimeout = FALSE;
	if (SetTimer(hwndimg, ID_MYTIMER, 1000, NULL) != 0) {
		bTimerSet = TRUE;
		return TRUE;
	}

	bTimerSet = FALSE;
	gserror(IDS_NOTIMER, NULL, MB_ICONINFORMATION, SOUND_TIMEOUT);
	return FALSE;
}

void
clear_timer()
{
	if (bTimerSet)
		KillTimer(hwndimg, ID_MYTIMER);
	bTimerSet = FALSE;
	bTimeout = FALSE;
	EnableWindow(hwndimg, TRUE);
}

/* display or remove 'wait' message */
void
info_wait(BOOL wait)
{
HWND hwnd;
POINT pt;
	waiting = wait;
	InvalidateRect(hwndimg, (LPRECT)&info_rect, FALSE);
	UpdateWindow(hwndimg);

	if (waiting) {
            GetCursorPos(&pt);
	    hwnd = WindowFromPoint(pt);
	    if ((hwnd == hwndimg) || IsChild(hwndimg,hwnd))
		SetCursor(hcWait);
	}
	else {
	    /* set cursor to that of active window */
	    hwnd = GetFocus();
	    if ( (hwndimgchild && IsWindow(hwndimgchild))
	      && ((hwnd == hwndimg) || (hwnd == hwndimgchild)) ) {
		if (in_child_client_area()) {
			SetCursor(GetClassCursor(hwndimgchild));
			return;
		}
	    }
	    SetCursor(GetClassCursor(hwnd));
	}
}


/* remove temporary files etc. */
void
gsview_close()
{
	gswin_close();
	pipeclose();
	print_cleanup();
	if ((efname[0] != '\0') && !debug)
		unlink(efname);
	efname[0] = '\0';
	if (page_list.select)
		free(page_list.select);
	page_list.select = NULL;
	if (doc)
		psfree(doc);
	doc = (struct document *)NULL;
	if (settings)
		write_profile();
	SetCursor(GetClassCursor((HWND)NULL));
	return;
}


/* gsview menu commands */
int
gsview_command(WORD command)
{
char prompt[MAXSTR];		/* input dialog box prompt and message box string */
char answer[MAXSTR];		/* input dialog box answer string */
    if (hDlgModeless) {
	play_sound(SOUND_ERROR);
	return 0;	/* obtaining Bounding Box so ignore commands */
    }
    if (waiting) {
	/* if user impatient or gsview confused */
	LoadString(phInstance, IDS_BUSY, prompt, sizeof(prompt));
	if (MessageBox(hwndimg, prompt, szAppName, MB_YESNO | MB_ICONQUESTION) == IDYES) {
	    play_sound(SOUND_ERROR);
	    next_page();
	    info_wait(FALSE);
	}
	return 0;
    }
    switch (command) {
	case IDM_OPEN:
		dfreopen();
		gsview_display();
		dfclose();
		return 0;
	case IDM_CLOSE:
		dfreopen();
		gsview_endfile();
		dfname[0] = '\0';
		dfclose();
		if (page_list.select)
			free(page_list.select);
		page_list.select = NULL;
		if (doc)
			psfree(doc);
		doc = (struct document *)NULL;
	    	if (gswin_hinst != (HINSTANCE)NULL) {
	    	    fprintf(cfile,"erasepage flushpage\r\n");
	            set_timer(timeout);
	            pipeflush();
	    	}
		info_wait(FALSE);
		return 0;
	case IDM_NEXT:
		if (not_open())
		    return 0;
		info_wait(TRUE);
		if (doc==(struct document *)NULL) {
		    if (!gswin_open())
		        return 0;
		    if (is_pipe_done()) {
			play_sound(SOUND_NOPAGE);
			info_wait(FALSE);
		    }
		    else {
			pagenum++;
			next_page();
		    }
		    return 0;
		}
		dfreopen();
		dsc_next(1);
		dfclose();
		return 0;
	case IDM_NEXTSKIP:
		if (not_dsc())
		    return 0;
		dfreopen();
		dsc_next(page_skip);
		dfclose();
		return 0;
	case IDM_REDISPLAY:
		if (not_open())
		    return 0;
		info_wait(TRUE);
		if (doc==(struct document *)NULL) {
		    /* don't know where we are so close and reopen */
		    if (!is_pipe_done())
			gswin_close();
		}
		if (!gswin_open())
		    return 0;
		info_wait(TRUE);
		if (page_ready)
		    next_page(); 
		dfreopen();
		if ((doc==(struct document *)NULL) || (doc->pages==0)) {
			gsview_displayfile(dfname);
			dfclose();
			return 0;
		}
		dsc_dopage();
		dfclose();
		return 0;
	case IDM_PREV:
		if (not_dsc())
			return 0;
		dfreopen();
		dsc_prev(1);
		dfclose();
		return 0;
	case IDM_PREVSKIP:
		if (not_dsc())
			return 0;
		dfreopen();
		dsc_prev(page_skip);
		dfclose();
		return 0;
	case IDM_GOTO:
		if (not_dsc())
			return 0;
		dfreopen();
		if (doc->numpages == 0) {
		    gserror(IDS_NOPAGE, NULL, MB_ICONEXCLAMATION, SOUND_NONUMBER);
		}
		else if (get_page(&pagenum, FALSE)) {
		    if (pagenum > doc->numpages) {
			pagenum = doc->numpages;
			play_sound(SOUND_NOPAGE);
		    }
		    else if (pagenum < 1) {
			pagenum = 1;
			play_sound(SOUND_NOPAGE);
		    }
		    else {
			if (gswin_open()) {
			    info_wait(TRUE);
			    if (page_ready)
			        next_page();
			    dsc_dopage();
			}
		    }
		}
		dfclose();
		return 0;
	case IDM_INFO:
		{
		DLGPROC lpProcInfo;
		lpProcInfo = (DLGPROC)MakeProcInstance((FARPROC)InfoDlgProc, phInstance);
		DialogBoxParam( phInstance, "InfoDlgBox", hwndimg, lpProcInfo, (LPARAM)NULL);
		FreeProcInstance((FARPROC)lpProcInfo);
		}
		return 0;
	case IDM_SELECT:
		gsview_select();
		dfclose();
		return 0;
	case IDM_PRINT:
		if (dfname[0] == '\0')
		    gsview_select();
		dfreopen();
		if (dfname[0] != '\0')
		    gsview_print(FALSE);
		dfclose();
		return 0;
	case IDM_PRINTTOFILE:
		if (dfname[0] == '\0')
			gsview_select();
		dfreopen();
		if (dfname[0] != '\0')
		    gsview_print(TRUE);
		dfclose();
		return 0;
	case IDM_SPOOL:
		gsview_spool();
		return 0;
	case IDM_EXTRACT:
		if (dfname[0] == '\0')
		    gsview_select();
		dfreopen();
		if (dfname[0] != '\0')
		    gsview_extract();
		dfclose();
		return 0;
	case IDM_EXIT:
		PostQuitMessage(0);
		return 0;
	case IDM_COPYCLIP:
		if (hwndimgchild && IsWindow(hwndimgchild))
			SendMessage(hwndimgchild, WM_GSVIEW, COPY_CLIPBOARD, NULL);
		return 0;
	case IDM_PASTETO:
		clip_to_file();
		return 0;
	case IDM_CONVERT:
		clip_convert();
		return 0;
	case IDM_GSCOMMAND:
		LoadString(phInstance, IDS_GSCOMMAND, prompt, sizeof(prompt));
		strcpy(answer, szGSwin);
		LoadString(phInstance, IDS_TOPICGSCMD, szHelpTopic, sizeof(szHelpTopic));
		if (get_string(prompt,answer))
		    strcpy(szGSwin, answer);
		if (szGSwin[0]=='\0')
		    strcpy(szGSwin, DEFAULT_GSCOMMAND);
		return 0;
	case IDM_SAVEDIR:
		save_dir = !save_dir;
		CheckMenuItem(hmenu, IDM_SAVEDIR, MF_BYCOMMAND | 
		    (save_dir ? MF_CHECKED : MF_UNCHECKED));
		return 0;
	case IDM_BUTTONSHOW:
		button_show = !button_show;
		CheckMenuItem(hmenu, IDM_BUTTONSHOW, MF_BYCOMMAND | 
		    (button_show ? MF_CHECKED : MF_UNCHECKED));
		show_buttons();
		return 0;
	case IDM_QUICK:
		quick = !quick;
		CheckMenuItem(hmenu, IDM_QUICK, MF_BYCOMMAND | 
		    (quick ? MF_CHECKED : MF_UNCHECKED));
		return 0;
	case IDM_AUTOREDISPLAY:
		redisplay = !redisplay;
		CheckMenuItem(hmenu, IDM_AUTOREDISPLAY, MF_BYCOMMAND | 
		    (redisplay ? MF_CHECKED : MF_UNCHECKED));
		return 0;
	case IDM_EPSFCLIP:
		epsf_clip = !epsf_clip;
		CheckMenuItem(hmenu, IDM_EPSFCLIP, MF_BYCOMMAND | 
		    (epsf_clip ? MF_CHECKED : MF_UNCHECKED));
		gswin_resize();
		return 0;
	case IDM_EPSFWARN:
		epsf_warn = !epsf_warn;
		CheckMenuItem(hmenu, IDM_EPSFWARN, MF_BYCOMMAND | 
		    (epsf_warn ? MF_CHECKED : MF_UNCHECKED));
		return 0;
	case IDM_PSTOEPS:
		if (dfname[0] == '\0')
		    gsview_display();
		if (dfname[0] != '\0') {
		    dfreopen();
		    ps_to_eps();
		    dfclose();
		}
		return 0;
	case IDM_MAKEEPSI:
		dfreopen();
		make_eps_interchange();
		dfclose();
		return 0;
	case IDM_MAKEEPST4:
	case IDM_MAKEEPST:
		dfreopen();
		make_eps_tiff(command);
		dfclose();
		return 0;
	case IDM_MAKEEPSW:
		dfreopen();
		make_eps_metafile();
		dfclose();
		return 0;
	case IDM_EXTRACTPS:
	case IDM_EXTRACTPRE:
		dfreopen();
		extract_doseps(command);
		dfclose();
		return 0;
	case IDM_SETTINGS:
		write_profile();
		return 0;
	case IDM_SAVESETTINGS:
		if (settings) 
			CheckMenuItem(hmenu, IDM_SAVESETTINGS, MF_BYCOMMAND | MF_UNCHECKED);
		else
			CheckMenuItem(hmenu, IDM_SAVESETTINGS, MF_BYCOMMAND | MF_CHECKED);
		settings = !settings;
		sprintf(prompt, "%d", settings);
		WritePrivateProfileString(INISECTION, "SaveSettings", prompt, INIFILE);
		return 0;
	case IDM_SOUNDS:
		{
		DLGPROC lpProcSound;
		LoadString(phInstance, IDS_TOPICSOUND, szHelpTopic, sizeof(szHelpTopic));
		lpProcSound = (DLGPROC)MakeProcInstance((FARPROC)SoundDlgProc, phInstance);
		DialogBoxParam( phInstance, "SoundDlgBox", hwndimg, lpProcSound, (LPARAM)NULL);
		FreeProcInstance((FARPROC)lpProcSound);
		}
		return 0;
	case IDM_PORTRAIT:
	case IDM_LANDSCAPE:
	case IDM_UPSIDEDOWN:
	case IDM_SEASCAPE:
	case IDM_SWAPLANDSCAPE:
		dfreopen();
		gsview_orientation(command);
		dfclose();
		return 0;
	case IDM_RESOLUTION:
		LoadString(phInstance, IDS_RES, prompt, sizeof(prompt));
		if (xdpi == ydpi)
		    sprintf(answer,"%g", xdpi);
		else 
		    sprintf(answer,"%g %g", xdpi, ydpi);
		LoadString(phInstance, IDS_TOPICMEDIA, szHelpTopic, sizeof(szHelpTopic));
		if (get_string(prompt,answer)) {
		    switch (sscanf(answer,"%f %f", &xdpi, &ydpi)) {
		      case EOF:
		      case 0:
			return 0;
		      case 1:
			ydpi = xdpi;
		      case 2:
			if (xdpi==0.0)
			    xdpi = DEFAULT_RESOLUTION;
			if (ydpi==0.0)
			    ydpi = DEFAULT_RESOLUTION;
			dfreopen();
			gswin_resize();
			dfclose();
		    }
		}
		return 0;
	case IDM_LETTER:
	case IDM_LETTERSMALL:
	case IDM_TABLOID:
	case IDM_LEDGER:
	case IDM_LEGAL:
	case IDM_STATEMENT:
	case IDM_EXECUTIVE:
	case IDM_A3:
	case IDM_A4:
	case IDM_A4SMALL:
	case IDM_A5:
	case IDM_B4:
	case IDM_B5:
	case IDM_FOLIO:
	case IDM_QUARTO:
	case IDM_10X14:
	case IDM_USERSIZE:
		if (command == IDM_USERSIZE)
		    if (!gsview_usersize())
			return 0;
		dfreopen();
		gsview_media(command);
		dfclose();
		return 0;
	case IDM_HELPCONTENT:
		WinHelp(hwndimg,szHelpName,HELP_CONTENTS,(DWORD)NULL);
		return 0;
	case IDM_HELPSEARCH:
		WinHelp(hwndimg,szHelpName,HELP_PARTIALKEY,(DWORD)"");
		return 0;
	case IDM_ABOUT:
		{
		DLGPROC lpProcAbout;
		lpProcAbout = (DLGPROC)MakeProcInstance((FARPROC)AboutDlgProc, phInstance);
		DialogBoxParam( phInstance, "AboutDlgBox", hwndimg, lpProcAbout, (LPARAM)NULL);
		FreeProcInstance((FARPROC)lpProcAbout);
		}
		return 0;
	}
	return 0;
}

/* if no document open, display error message and return true */
BOOL
not_open()
{
	if (dfname[0] != '\0')
	    return FALSE;
	gserror(IDS_NOTOPEN, NULL, MB_ICONEXCLAMATION, SOUND_NOTOPEN);
	return TRUE;
}

/* if not DSC document or not open, display error message and return true */
BOOL
not_dsc()
{
	if (not_open())
	    return TRUE;
	if (doc!=(struct document *)NULL)
	    return FALSE;
	gserror(IDS_NOPAGE, NULL, MB_ICONEXCLAMATION, SOUND_NONUMBER);
	return TRUE;
}

void
gserror(UINT id, char *str, UINT icon, int sound)
{
int i;
char mess[300];
	if (sound >= 0)
	    play_sound(sound);
	i = 0;
	if (id)
	    i = LoadString(phInstance, id, mess, sizeof(mess)-1);
	mess[i] = '\0';
	if (str)
	    strncpy(mess+i, str, sizeof(mess)-i-1);
	MessageBox(hwndimg, mess, szAppName, icon | MB_OK);
}

/* for ps.c errors instead of fprintf(stderr,...)! */
void
pserror(char *str)
{
	MessageBox(hwndimg,str,szAppName, MB_OK | MB_ICONHAND);
}
