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

/* gvwin.c */
/* Main routines for Windows GSview */
/* by Russell Lang */
#include "gvwin.h"

char szAppName[MAXSTR];			/* application name - for title bar */
char szExePath[MAXSTR];
char szIniFile[MAXSTR];
char szWait[MAXSTR];
char szFindText[MAXSTR];
char previous_filename[MAXSTR];
const char szClassName[] = "gsview_class";
const char szScratch[] = "gsview";	/* temporary filename prefix */

HWND hwndimg;			/* gsview main window */
HWND hDlgModeless;		/* any modeless dialog box */
HWND hwndtext;			/* gswin text window */
HWND hwndimgchild;		/* gswin image child window */
HINSTANCE phInstance;		/* instance of gsview */
int bitmap_scrollx=0;		/* offset from bitmap to origin of child window */
int bitmap_scrolly=0;
PSFILE psfile;		/* Postscript file structure */
PROG gsprog;		/* Ghostscript program structure */
OPTIONS option;		/* GSview options (saved in INI file) */
DISPLAY display;	/* Display parameters */
PSDOC *doc;
struct page_list_s page_list;	/*  page selection for print/extract */


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
char szHelpTopic[MAXSTR];	/* topic for OFN_SHOWHELP */
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
int page_extra;			/* extra pages to skip */
int page_skip = 5;		/* number of pages to skip in IDM_NEXTSKIP or IDM_PREVSKIP */
BOOL changed_version = FALSE;	/* to warn user to update Ghostscript Command */
BOOL zoom = FALSE;		/* true if display zoomed */
BOOL debug = FALSE;		/* /D command line option used */
HINSTANCE hlib_mmsystem;	/* DLL containing sndPlaySound function */
FPSPS lpfnSndPlaySound;		/* pointer to sndPlaySound function if loaded */

/* timer used for open, close, display & print timeouts */
BOOL bTimeout;			/* true if timeout occured */
BOOL bTimerSet;			/* true if TIMER running */
#define ID_MYTIMER 1
UINT timeout_count;

/* document manipulation */

/* local functions */
BOOL draw_button(DRAWITEMSTRUCT FAR *lpdis);
BOOL in_child_client_area(void);
BOOL in_client_area(void);
BOOL in_info_area(void);
void info_paint(HWND, HDC);
void cursorpos_paint(HDC hdc);
void gsview_close(void);


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
		gsprog.hinst = (HINSTANCE)NULL;
		gsprog.valid = FALSE;
		hwndimgchild = (HWND)NULL;
		hwndtext = (HWND)NULL;
		bitmap_scrollx = bitmap_scrolly = 0;
		display.page = FALSE;
		display.saved = FALSE;
		page_extra = 0;
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
		display.page = TRUE;
		info_wait(FALSE);
		if (page_extra) {
		    PostMessage(hwndimg, WM_COMMAND, IDM_SKIP, (LPARAM)0);
		}
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
		load_string(IDS_WAITDRAW, szWait, sizeof(szWait));
		info_wait(TRUE);
		piperequest();
		break;
	    case BEGIN:
	    case END:
		/* do nothing at present */
		/* we can figure out when we have reached EOF by inspecting */
		/* the imitation pipe */
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
	    	    HGLOBAL hglobal;
		    int i, cFiles, length;
		    HDROP hdrop = (HDROP)wParam;
		    cFiles = DragQueryFile(hdrop, 0xffff, (LPSTR)NULL, 0);
		    for (i=0; i<cFiles; i++) {
			length = DragQueryFile(hdrop, i, (LPSTR)NULL, 0);
	    		hglobal = GlobalAlloc(GHND | GMEM_SHARE, length+1);
			if (hglobal) {
				szFile = GlobalLock(hglobal);
				DragQueryFile(hdrop, i, szFile, MAXSTR);
	        		GlobalUnlock(hglobal);
				/* it doesn't work if we call gsview_display directly */
				PostMessage(hwnd, WM_COMMAND, IDM_DROP, (LPARAM)hglobal);
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
			    && (doc != (PSDOC *)NULL) && doc->epsf)
			    EnableMenuItem(hmenuedit, 5, MF_BYPOSITION | MF_ENABLED);
			else
			    EnableMenuItem(hmenuedit, 5, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
			/* Extract EPS sub menu */
			if ( (psfile.preview == IDS_EPST) || (psfile.preview == IDS_EPSW) )
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
		    LPSTR szFile;
		    char cmd[MAXSTR];
		    szFile = GlobalLock((HGLOBAL)lParam);
		    if (szFile && (lstrlen(szFile) < sizeof(cmd)))
			lstrcpy(cmd, szFile);
		    else
			cmd[0] = '\0';
#ifdef NOTUSED
#ifdef __WIN32__
		    hglobal = GlobalHandle((LPCVOID)lParam);
#else
		    hglobal = (HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(lParam)));
#endif
#endif
		    GlobalUnlock((HGLOBAL)lParam);
		    GlobalFree((HGLOBAL)lParam);
		    if ((cmd[0] == '-') || (cmd[0] == '/')) {
			switch (toupper(cmd[1])) {
			    case 'P':
		              gsview_selectfile(cmd+2);
		              gsview_print(FALSE);
			      break;
			    case 'F':
		              gsview_selectfile(cmd+2);
		              gsview_print(TRUE);
			      break;
			    case 'S':
			      if (cmd[2] != ' ') {
				char *fname;
				/* skip over port name */
				for (fname=cmd+2; *fname && *fname!=' '; fname++)
				  /* nothing */ ;
			        /* skip blanks until file name */
			        if (*fname) {
			          *fname++ = '\0'; /* place null after port name */
			          for (; *fname==' '; fname++)
			            /* nothing */ ;
			        }
			        if (*fname) {
			            /* found both filename and port */
			            gsview_spool(fname, cmd+2);
			            break;
			        }
			      }
		              gsview_spool(cmd+2, (char *)NULL);
			      break;
			    default:
			      gserror(IDS_BADCLI, cmd, MB_ICONEXCLAMATION, SOUND_ERROR);
			}
		    }
		    else
		        gsview_displayfile(cmd);
		}
		else {
		    if (GetNotification(wParam,lParam) != BN_DOUBLECLICKED) {
			if (hDlgModeless) {
			    play_sound(SOUND_ERROR);
			    return 0;	/* obtaining Bounding Box so ignore commands */
			}
			if (waiting) {
			    switch(LOWORD(wParam)) {
	    		        case IDM_INFO:
	    		        case IDM_SAVEDIR:
	    		        case IDM_SETTINGS:
	    		        case IDM_SAVESETTINGS:
	    		        case IDM_SOUNDS:
	    		        case IDM_HELPCONTENT:
	    		        case IDM_HELPSEARCH:
	    		        case IDM_ABOUT:
				case IDM_EXIT:
				    /* these are safe to use when busy */
				    break;
			        default:
			            play_sound(SOUND_ERROR);
			            return 0;	/* Command not permitted now */
			    }
			}
		        gsview_command(LOWORD(wParam));
		    }
		}
		/* if command resulted in an update to the display, do it */
		if (display.do_endfile || display.do_resize || display.do_display) {
		    if (!gs_open())
			return FALSE;
		    dfreopen();
		    do_output();
		    fflush(gsprog.input);
		    dfclose();
		    pipeflush();
		    display.busy = FALSE;
		    display.abort = FALSE;
		    display.do_endfile = FALSE;
		    display.do_resize = FALSE;
		    display.do_display = FALSE;
		}
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
			option.img_size.x = rect.right-rect.left;
			option.img_size.y = rect.bottom-rect.top;
		}
		return 0;
	case WM_MOVE:
		/* save window position for INIFILE */
		if (!IsIconic(hwnd) && !IsZoomed(hwnd)) {
			GetWindowRect(hwnd,&rect);
			option.img_origin.x = rect.left;
			option.img_origin.y = rect.top;
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
			HDC hdc;
			hdc = GetDC(hwnd);
			cursorpos_paint(hdc);
			ReleaseDC(hwnd, hdc);
		    }
		    prev_in_child = in_child_client_area();
		}
		break;
	case WM_PARENTNOTIFY:
		if (hDlgModeless && (wParam == WM_LBUTTONDOWN))
		    if (in_child_client_area())
		        SendMessage(hDlgModeless, WM_COMMAND, BB_CLICK, lParam);
		if (wParam == WM_RBUTTONDOWN) {
		    float x, y;
		    RECT rect;
		    GetWindowRect(hwndimgchild,&rect);
		    if (get_cursorpos(&x, &y)) {
		        zoom = !zoom;
		        display.zoom_xoffset = x;
		        display.zoom_yoffset = y;
			x = (bitmap_scrollx + (rect.right - rect.left)/2)*72.0/option.xdpi;
			y = ((display.height-1) - (bitmap_scrolly + (rect.bottom - rect.top)/2))*72.0/option.ydpi;
			transform_cursorpos(&x, &y);
			x *= option.xdpi/72.0;
			y *= option.ydpi/72.0;
			display.zoom_xoffset -= (int)(x*72.0/option.zoom_xdpi);
			display.zoom_yoffset -= (int)(y*72.0/option.zoom_ydpi);
		    }
		    else {
		        zoom = FALSE;
		    }
		    PostMessage(hwndimg, WM_COMMAND, IDM_ZOOM, (LPARAM)0);
		}
		break;
	case WM_PAINT:
		{
		HDC hdc;
		PAINTSTRUCT ps;
		RECT chrect;
		hdc = BeginPaint(hwnd, &ps);
		/* draw info area at top */
		info_paint(hwnd, hdc);
		/* draw button background */
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
		/* fill area outside Ghostscript image client */
		if (!option.fit_page && hwndimgchild && IsWindow(hwndimgchild)) {
		    GetClientRect(hwnd, &rect);
		    GetWindowRect(hwndimgchild, &chrect);
		    rect.top = info_rect.bottom+1;
		    rect.left = img_offset.x + chrect.right - chrect.left;
		    FillRect(hdc, &rect, GetStockObject(LTGRAY_BRUSH));
		    rect.left = button_rect.right+1;
		    rect.top = img_offset.y + chrect.bottom - chrect.top;
		    FillRect(hdc, &rect, GetStockObject(LTGRAY_BRUSH));
		}
		EndPaint(hwnd, &ps);
		}
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
#ifdef __WIN32__
			SIZE sz;
			GetTextExtentPoint(hdc, buf, i, &sz);
			SetBkMode(hdc, TRANSPARENT);
			TextOut(hdc, (rect.left+rect.right-sz.cx)/2,
			    (rect.top+rect.bottom-sz.cy)/2, buf, i);
#else
			DWORD dw = GetTextExtent(hdc, buf, i);
			SetBkMode(hdc, TRANSPARENT);
			TextOut(hdc, (rect.left+rect.right-LOWORD(dw))/2,
			    (rect.top+rect.bottom-HIWORD(dw))/2, buf, i);
#endif
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

/* returns true if cursor in client area of GSview window */
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

/* returns true if cursor in info area or button area of GSview windows */
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
get_cursorpos(float *x, float *y)
{
RECT rect;
POINT pt;
    if (hwndimgchild && IsWindow(hwndimgchild)) {
	GetClientRect(hwndimgchild, &rect);
	GetCursorPos(&pt);
	ScreenToClient(hwndimgchild, &pt);
	if (PtInRect(&rect, pt)) {
	  if (zoom) {
            /* first figure out number of pixels to zoom origin point */
	    *x = (bitmap_scrollx+pt.x)*72.0/option.xdpi;
	    *y = ((display.height-1) -(bitmap_scrolly+pt.y))*72.0/option.ydpi;
	    transform_cursorpos(x,y);
	    *x = *x * option.xdpi/72;
	    *y = *y * option.ydpi/72;
	    /* now convert to pts and offset it */
	    *x = *x * 72/option.zoom_xdpi + display.zoom_xoffset;
	    *y = *y * 72/option.zoom_ydpi + display.zoom_yoffset;
	  }
	  else {
	    *x = (bitmap_scrollx+pt.x)*72.0/option.xdpi 
		+ (display.epsf_clipped ? doc->bbox.llx : 0);
	    *y = ((display.height-1)-(bitmap_scrolly+pt.y))*72.0/option.ydpi
		+ (display.epsf_clipped ? doc->bbox.lly : 0);
	    transform_cursorpos(x,y);
	  }
	    return TRUE;
	}
    }
    return FALSE;
}

void
cursorpos_paint(HDC hdc)
{
float x, y;
char buf[32];
	SetBkMode(hdc, TRANSPARENT);
        FillRect(hdc, &info_coord, GetStockObject(LTGRAY_BRUSH));
	/* show coordinate */
	if (get_cursorpos(&x, &y)) {
	    switch(option.unit) {
	       case IDM_UNITPT:   
		  sprintf(buf,"%.0f, %.0fpt", x, y);
		  break;
	       case IDM_UNITMM:   
		  sprintf(buf,"%.0f, %.0fmm", x/72*25.4, y/72*25.4);
		  break;
	       case IDM_UNITINCH:   
		  sprintf(buf,"%.1f, %.1fin", x/72, y/72);
		  break;
	    }
	    SetTextAlign(hdc, TA_RIGHT);
	    TextOut(hdc, info_coord.right-1, info_coord.top, buf, strlen(buf));
	}
}

/* paint brief info area */
void
info_paint(HWND hwnd, HDC hdc)
{
RECT rect;
int i;
char buf[MAXSTR];
char fmt[MAXSTR];
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
	/* write file information */
	if (psfile.name[0] != '\0') {
	    i = LoadString(phInstance, IDS_FILE, buf, sizeof(buf));
	    GetFileTitle(psfile.name, buf+i, (WORD)(sizeof(buf)-i));
	    TextOut(hdc, info_file.x, info_file.y, buf, strlen(buf));
	    if (waiting) {
		TextOut(hdc, info_page.x, info_page.y, szWait, strlen(szWait));
	    }
	    else {
	      if (doc!=(PSDOC *)NULL) {
		int n = map_page(psfile.pagenum - 1);
		i = LoadString(phInstance, IDS_PAGEINFO, fmt, sizeof(fmt));
		if (doc->pages)
		    sprintf(buf, fmt, doc->pages[n].label ? doc->pages[n].label : " ",psfile.pagenum,  doc->numpages);
		else
		    sprintf(buf, fmt, " " ,psfile.pagenum,  doc->numpages);
		if (zoom)
		    strcat(buf, "  Zoomed");
		TextOut(hdc, info_page.x, info_page.y, buf, strlen(buf));
	      }
	      else {
		if (is_pipe_done())
		    i = LoadString(phInstance, IDS_NOMORE, buf, sizeof(buf));
		else {
		    i = LoadString(phInstance, IDS_PAGE, buf, sizeof(buf));
		    sprintf(buf+i, "%d", psfile.pagenum);
		}
		TextOut(hdc, info_page.x, info_page.y, buf, strlen(buf));
	      }
	      /* show coordinate */
	      cursorpos_paint(hdc);
	    }
	}
	else {
	    i = LoadString(phInstance, IDS_NOFILE, buf, sizeof(buf));
	    TextOut(hdc, info_file.x, info_file.y, buf, strlen(buf));
	    if (waiting) {
		TextOut(hdc, info_page.x, info_page.y, szWait, strlen(szWait));
	    }
	}
}


HWND hbutton_info;

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
#ifdef NOTUSED
	case WM_SETCURSOR:
		/* track cursor and display button info */
		if (hwnd != hbutton_info) {
		    char buf[MAXSTR];
		    hbutton_info = hwnd;
		    load_string(GetWindowID(hwnd), buf, sizeof(buf));
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
		    TextOut(hdc, info_file.x, info_file.y, buf, strlen(buf));
		    EndPaint(hwnd, &ps);
		}
		InvalidateRect(hwndimg, &info_rect, FALSE);
		UpdateWindow(hwndimg);
		break;
#endif
	}
	return CallWindowProc(lpfnButtonWndProc, hwnd, message, wParam, lParam);
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


/* remove temporary files etc. */
void
gsview_close()
{
	gs_close();
	pipeclose();
	print_cleanup();
	if (page_list.select)
		free(page_list.select);
	page_list.select = NULL;
	if (doc)
		dsc_scan_clean(doc);
	doc = (PSDOC *)NULL;
	if (option.settings)
		write_profile();
	SetCursor(GetClassCursor((HWND)NULL));
	return;
}


void
copy_clipboard()
{
	if (hwndimgchild && IsWindow(hwndimgchild))
		SendMessage(hwndimgchild, WM_GSVIEW, COPY_CLIPBOARD, NULL);
}
