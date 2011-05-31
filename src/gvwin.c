/* Copyright (C) 1993-2000, Ghostgum Software Pty Ltd.  All rights reserved.
  
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
#include "gvwin.h"

char szAppName[MAXSTR] = GSVIEW_PRODUCT;  /* application name - for title bar */
char szExePath[MAXSTR];
char szIniFile[MAXSTR];
char szWait[MAXSTR];
char szFindText[MAXSTR];
char previous_filename[MAXSTR];
char selectname[MAXSTR];
const char szClassName[] = "gsview_class";
const char szImgClassName[] = "gsview_img_class";
const char szScratch[] = "gsview";	/* temporary filename prefix */
char *szSpoolPrefix = "\\\\spool\\";

HWND hwndimg;			/* gsview main window */
HWND hDlgModeless;		/* any modeless dialog box */
HWND hwnd_measure;		/* measure modelss dialog box */
HWND hwnd_fullscreen;		/* full screen popup of child window */
HWND hwnd_image;		/* full screen or image child window */	
HWND hwndimgchild;		/* gswin image child window */
HWND hwndspl;			/* window handle of gsv16spl.exe */
HINSTANCE phInstance;		/* instance of gsview */
HINSTANCE hlanguage;		/* instance of language resources */
PSFILE psfile;		/* Postscript file structure */
PRINTER printer;	/* Ghostscript printer structure */
BOOL win32s_printer_pending = FALSE;
BMAP bitmap;		/* information about display bitmap */
OPTIONS option;		/* GSview options (saved in INI file) */
DISPLAY display;	/* Display parameters */
char last_files[4][MAXSTR];	/* last 4 files used */
int last_files_count;		/* number of files known */
HISTORY history;		/* history of pages displayed */
BOOL fullscreen = FALSE;
HCURSOR hCursorArrow;

struct sound_s sound[NUMSOUND] = {
	{"SoundOutputPage", IDS_SNDPAGE, ""},
	{"SoundNoPage", IDS_SNDNOPAGE, BEEP},
	{"SoundNoNumbering", IDS_SNDNONUMBER, ""},
	{"SoundNotOpen", IDS_SNDNOTOPEN, ""},
	{"SoundError", IDS_SNDERROR, BEEP},
	{"SoundStart", IDS_SNDSTART, ""},
	{"SoundExit", IDS_SNDEXIT, ""},
	{"SoundBusy", IDS_SNDBUSY, BEEP},
};

/* initialised in init.c */
BOOL is_winnt = FALSE;		/* To allow selective use of Windows NT features */
BOOL is_win95 = FALSE;		/* To allow selective use of Windows 95 features */
BOOL is_win32s = FALSE;		/* To allow selective use of Win32s misfeatures */
BOOL is_win4;			/* To allow selective use of Windows 4.0 features */
BOOL multithread = FALSE;
#ifdef __WIN32__
CRITICAL_SECTION crit_sec;	/* for thread synchronization */
#endif
HANDLE hmutex_ps;		/* for protecting psfile and pending */
char szHelpName[MAXSTR];	/* buffer for building help filename */
char szHelpTopic[MAXSTR];	/* topic for OFN_SHOWHELP */
UINT help_message;		/* message sent by OFN_SHOWHELP */
HMENU hmenu;			/* main menu */
HACCEL haccel;			/* menu accelerators */
HCURSOR hcWait;
HCURSOR hcCrossHair;
HCURSOR hcHand;
POINT img_offset;		/* offset to gswin child window */
HFONT info_font;		/* font for info line */
POINT info_file;		/* position of file information */
POINT info_page;		/* position of page information */
RECT  info_rect;		/* position and size of brief info area */
RECT  info_coord;		/* position and size of coordinate information */
RECT  button_rect;		/* position and size of button area */
int on_link;			/* TRUE if we were or are over link */
int on_link_page;		/* page number of link target */
BOOL ignore_sync = FALSE;	/* ignore next GSDLL_SYNC */
BOOL fit_page_enabled = FALSE;	/* next WM_SIZE is allowed to resize window */

BOOL prev_in_child;		/* true if cursor previously in gswin child window */
int page_skip = 5;		/* number of pages to skip in IDM_NEXTSKIP or IDM_PREVSKIP */
BOOL zoom = FALSE;		/* true if display zoomed */
BOOL debug = FALSE;		/* /D command line option used */
HINSTANCE hlib_mmsystem;	/* DLL containing sndPlaySound function */
FPSPS lpfnSndPlaySound;		/* pointer to sndPlaySound function if loaded */
BOOL quitnow = FALSE;		/* Used to cause exit from nested message loops */

int percent_done;		/* percentage of document processed */
int percent_pending;		/* TRUE if WM_GSPERCENT is pending */


#if (WINVER < 0x0400)
/* Windows 4.0 scroll bar extras */
#define SIF_RANGE           0x01
#define SIF_PAGE            0x02
#define SIF_POS             0x04
#define SIF_DISABLENOSCROLL 0x08
#define SIF_TRACKPOS        0x10
#define SIF_ALL             (SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS)
#define SBM_SETSCROLLINFO 0x00E9
#define SBM_GETSCROLLINFO 0x00EA

typedef struct tagSCROLLINFO {
    UINT cbSize;
    UINT fMask;
    int nMin;
    int nMax;
    UINT nPage;
    int nPos;
    int nTrackPos;
} SCROLLINFO;
typedef SCROLLINFO FAR *LPSCROLLINFO;
/*
WINUSERAPI int WINAPI SetScrollInfo(HWND, int, LPSCROLLINFO, BOOL);
WINUSERAPI int WINAPI GetScrollInfo(HWND, int, LPSCROLLINFO);
*/
#endif

typedef int (WINAPI *PFN_SetScrollInfo)(HWND, int, LPSCROLLINFO, BOOL);
PFN_SetScrollInfo pSetScrollInfo;
HMODULE hmodule_user32;

#ifdef __WIN32__
BOOL
load_SetScrollInfo(void)
{
    /* Instead of linking to SetScrollInfo at load time,
     * we instead do it at run time.
     * This allows us to produce an EXE that will run under
     * Windows 3.1 and Windows 4.0
     */
    if (!is_win4)
	return FALSE;
    hmodule_user32 = LoadLibrary("USER32.DLL");
    if (hmodule_user32 < (HINSTANCE)HINSTANCE_ERROR) {
	hmodule_user32 = (HINSTANCE)NULL;
        return FALSE;
    }
    pSetScrollInfo = (PFN_SetScrollInfo) GetProcAddress(hmodule_user32, "SetScrollInfo");
    if (!pSetScrollInfo)
        return FALSE;
    return TRUE;
}

void
free_SetScrollInfo(void) 
{
    if (!hmodule_user32)
	return;
    pSetScrollInfo = (PFN_SetScrollInfo)NULL;
    FreeLibrary(hmodule_user32);
    hmodule_user32 = (HINSTANCE)NULL;
}
#else
BOOL
load_SetScrollInfo(void)
{
    pSetScrollInfo = (PFN_SetScrollInfo)NULL;
    return FALSE;
}

void
free_SetScrollInfo(void) 
{
    pSetScrollInfo = (PFN_SetScrollInfo)NULL;
}
#endif


/* local functions */
BOOL draw_button(DRAWITEMSTRUCT FAR *lpdis);
BOOL in_child_client_area(void);
BOOL in_client_area(void);
BOOL in_info_area(void);
void info_paint(HWND, HDC);
void cursorpos_paint(HDC hdc);
void gsview_close(void);
BOOL query_close(void);
void update_scroll_bars(void);
void gs_thread(void *arg);
void map_pt_to_pixel(float *x, float *y);
void enable_menu_item(int menuid, int itemid, BOOL enabled);
void end_button_help(void);

void highlight_words(HDC hdc, int first, int last);
void highlight_links(HDC hdc);
BOOL text_marking = FALSE;
int text_mark_first = -1;
int text_mark_last = -1;
void info_link(void);


int PASCAL 
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int cmdShow)
{
	MSG msg;
	LPSTR command_line;

	/* copy the hInstance into a variable so it can be used */
	phInstance = hInstance;

#ifdef __WIN32__
	command_line = GetCommandLine();
	while (*command_line && *command_line != ' ') {
	    /* skip over program name */
	    if (*command_line == '\042') {
		/* skip over name, until closing quote */
		command_line++;
		while (*command_line && (*command_line != '\042'))
		    command_line++;
		if (*command_line)
		    command_line++;
	    }
	    else 
		command_line++;
	}
	while (*command_line == ' ')
	    command_line++;	/* skip until first argument */
#else
	command_line = lpszCmdLine;
#endif

	dde_initialise();

	if (hPrevInstance) {
	    /* don't run more than one copy */
	    /* because we can't run more than one Ghostscript */
	    /* Win95 and WinNT always have hPrevInstance == 0 */
	    gsview_init0(command_line);
	    /* dde_execute_line(command_line); */
	    dde_uninitialise();
	    return FALSE;
	}

	if (!gsview_init1(command_line)) {
	    dde_uninitialise();
	    return FALSE;
        }

	/* don't start the DDE server until after we have parsed the 
	 * command line, otherwise we could end up talking to ourselves
	 * instead of a separate copy of GSview
	 */
	dde_enable_server(TRUE);

	load_SetScrollInfo();
#ifdef __WIN32__
	{   STARTUPINFO sti;
	    GetStartupInfo(&sti);
	    ShowWindow(hwndimg, option.img_max && (sti.wShowWindow == SW_SHOWNORMAL) 
		? SW_SHOWMAXIMIZED : SW_SHOWDEFAULT);
	}
#else
	ShowWindow(hwndimg, option.img_max && (cmdShow == SW_SHOWNORMAL) 
		? SW_SHOWMAXIMIZED : cmdShow);
#endif
	info_wait(IDS_NOWAIT);
	if (gsview_changed())
	    PostQuitMessage(0);

#ifdef __WIN32__
	if (multithread) {
	    /* start thread for displaying */
	    display.tid = _beginthread(gs_thread, 131072, NULL);
	}
#endif
	
	while (!(!multithread && quitnow)
		 && GetMessage(&msg, (HWND)NULL, 0, 0)) {
	    if ( ((hDlgModeless == 0) || !IsDialogMessage(hDlgModeless, &msg)) 
	      && ((hwnd_measure == 0) || !IsDialogMessage(hwnd_measure, &msg)) 
	       ) {
	        if (!TranslateAccelerator(hwndimg, haccel, &msg)) {
		    TranslateMessage(&msg);
		    DispatchMessage(&msg);
	        }
	    }
#ifdef __WIN32__
	    if (multithread) {
	        /* release other thread if needed */
	        if (pending.unload || pending.now || pending.next || quitnow)
		    SetEvent(display.event);
	    }
	    else 
#endif
	    {
	        if (pending.now) {
	            if (is_win95 || is_winnt)
		        gs_process();	/* start Ghostscript */
		    else {
		        /* Win32s can't start gs_process while printing */
			/* Check if gvwgs.exe is running */
			HWND hwndprn = FindWindow(NULL, "GSview Print");
			if (IsWindow(hwndprn)) {
			    pending.now = FALSE;
			    gserror(0, "Busy printing.  Win32s can't use Ghostscript for displaying while it is being used for printing. Try again when 'GSview Print' has finished.", MB_ICONEXCLAMATION, SOUND_ERROR);
			}
			if (pending.now)
		            gs_process();	/* start Ghostscript */
		    }
		    update_scroll_bars();
		}
	    }
	    if (
#ifdef __WIN32__
		is_win32s && 
#endif
		win32s_printer_pending) {
		/* Win32s can't load GS DLL twice */
		/* so we must run it while display GS DLL is unloaded */
		start_gvwgs();
		win32s_printer_pending = FALSE;
		/* We can't stop the user attempting to display while */
		/* printing since we don't know when printer finished */
		/* We'll see how much of a problem this causes */
	    }
	}

	dde_uninitialise();

	play_sound(SOUND_EXIT);
	gsview_close();
	DestroyWindow(hwndimg);
	delete_buttons();
	free_SetScrollInfo();
 	WinHelp(hwndimg,szHelpName,HELP_QUIT,(DWORD)NULL);
	if (hlib_mmsystem != (HINSTANCE)NULL)
	    FreeLibrary(hlib_mmsystem);
	if ((hlanguage != (HINSTANCE)NULL) && (hlanguage != phInstance))
	    FreeLibrary(hlanguage);

#ifdef DEBUG_MALLOC
        if (malloc_file)
	    fclose(malloc_file);
#endif
	return 0;
}


/* child image window */
LRESULT CALLBACK _export
WndImgChildProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{	
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rect;
	int nVscrollInc, nHscrollInc;
	static int cxAdjust, cyAdjust;
	static int cxClient, cyClient;
	static int nHscrollPos, nHscrollMax;
	static int nVscrollPos, nVscrollMax;

	switch(message) {
		case WM_CREATE:
			cxClient = cyClient = 0;
			nHscrollPos = nHscrollMax = 0;
			nVscrollPos = nVscrollMax = 0;
			break;
		case WM_DESTROY:
			hwnd_image = hwndimgchild;
			fullscreen = FALSE;
			if ((hwnd == hwnd_fullscreen) && gsdll.device) {
			    ShowWindow(hwnd_image, SW_SHOWNA);
			    update_scroll_bars();
			    InvalidateRect(hwnd_image, (LPRECT)NULL, FALSE);
			    UpdateWindow(hwnd_image);
			    if (debug)
				gs_addmess("Full Screen finished\r\n");
			}
			hwnd_fullscreen = (HWND)NULL;
			break;
		case WM_SIZE:
			if (wParam == SIZE_MINIMIZED)
				return(0);
			cyClient = HIWORD(lParam);
			cxClient = LOWORD(lParam);

#ifdef OLD
			cyAdjust = min(bitmap.height, cyClient) - cyClient;
			cyClient += cyAdjust;
#else
			if (bitmap.height < cyClient) {
			    /* shrink window */
			    cyAdjust = bitmap.height - cyClient;
			}
			else {
			    if (!fullscreen && fit_page_enabled) {
				/* We just got a GSDLL_SIZE and option.fitpage was TRUE */
				/* enlarge window to smaller of bitmap height */
				/* and height if client extended to bottom of screen */
				GetWindowRect(GetParent(hwnd),&rect);
				cyAdjust = min(bitmap.height, 
				    cyClient + GetSystemMetrics(SM_CYFULLSCREEN) + 
				    GetSystemMetrics(SM_CYCAPTION) - rect.bottom)
				    - cyClient;
			    }
			    else
				cyAdjust = 0;
			}
			cyClient += cyAdjust;
#endif

			nVscrollMax = max(0, bitmap.height - cyClient);
			nVscrollPos = min(nVscrollPos, nVscrollMax);

			if (!gsdll.device)
			    nVscrollMax = nVscrollPos = 0;

			if (fullscreen) {
			    /* never display scroll bars */
			    SetScrollRange(hwnd, SB_VERT, 0, 0, TRUE);
			}
			else if (is_win4 && pSetScrollInfo) {
			    SCROLLINFO si;
			    si.cbSize = sizeof(si);
			    si.fMask = SIF_ALL;
			    si.nMin = 0;
			    /* Win32 docs say use nMax = nVscrollMax + (nPage-1) */
			    /* Overview example says use nMax = nVscrollMax */
			    if (nVscrollMax) {
				si.nPage = cyClient;
			        si.nMax = nVscrollMax + (cyClient - 1);
			    }
			    else {
			        si.nMax = si.nPage = 0;
			    }
			    si.nPos = nVscrollPos;
			    si.nTrackPos = 0;
			    pSetScrollInfo(hwnd, SB_VERT, &si, TRUE);
			}
			else 
			{
			    SetScrollRange(hwnd, SB_VERT, 0, nVscrollMax, FALSE);
			    SetScrollPos(hwnd, SB_VERT, nVscrollPos, TRUE);
			}

#ifdef OLD
			cxAdjust = min(bitmap.width,  cxClient) - cxClient;
			cxClient += cxAdjust;
#else
			if (bitmap.width < cxClient) {
			    /* shrink window */
			    cxAdjust = bitmap.width - cxClient;
			}
			else {
			    if (fit_page_enabled) {
				/* We just got a GSDLL_SIZE and option.fitpage was TRUE */
				/* enlarge window to smaller of bitmap width */
				/* and width if client extended to right of screen */
				GetWindowRect(GetParent(hwnd),&rect);
				cxAdjust = min(bitmap.width, 
				    cxClient + GetSystemMetrics(SM_CXFULLSCREEN)-rect.right)
				    - cxClient;
			    }
			    else
				cxAdjust = 0;
			}
			cxClient += cxAdjust;
#endif

			nHscrollMax = max(0, bitmap.width - cxClient);
			nHscrollPos = min(nHscrollPos, nHscrollMax);

			if (!gsdll.device)
			    nHscrollMax = nHscrollPos = 0;

			if (fullscreen) {
			    /* never display scroll bars */
			    SetScrollRange(hwnd, SB_HORZ, 0, 0, TRUE);
			}
			else if (is_win4 && pSetScrollInfo) {
			    SCROLLINFO si;
			    si.cbSize = sizeof(si);
			    si.fMask = SIF_ALL;
			    si.nMin = 0;
			    if (nHscrollMax) {
				si.nPage = cxClient;
			        si.nMax = nHscrollMax + (cxClient - 1);
			    }
			    else {
			        si.nMax = si.nPage = 0;
			    }
			    si.nPos = nHscrollPos;
			    si.nTrackPos = 0;
			    pSetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
			}
			else 
			{
			    SetScrollRange(hwnd, SB_HORZ, 0, nHscrollMax, FALSE);
			    SetScrollPos(hwnd, SB_HORZ, nHscrollPos, TRUE);
			}

			bitmap.scrollx = nHscrollPos;
			bitmap.scrolly = nVscrollPos;

			if (!fullscreen && option.fit_page &&
			    (wParam==SIZE_RESTORED) && !IsZoomed(hwndimg) &&
			    gsdll.device && (cxAdjust!=0 || cyAdjust!=0) ) {
			    GetWindowRect(GetParent(hwnd),&rect);
			    MoveWindow(GetParent(hwnd),rect.left,rect.top,
				rect.right-rect.left+cxAdjust,
				rect.bottom-rect.top+cyAdjust, TRUE);
			    cxAdjust = cyAdjust = 0;
			}
			fit_page_enabled = FALSE;

			/* centre the bitmap if smaller than client window */
			GetClientRect(hwnd, &rect);
			cxClient = rect.right - rect.left;
			cyClient = rect.bottom - rect.top;
			if (bitmap.width < cxClient)
			    display.offset.x = (cxClient - bitmap.width) / 2;
			else
			    display.offset.x = 0;
			if (bitmap.height < cyClient)
			    display.offset.y = (cyClient - bitmap.height) / 2;
			else
			    display.offset.y = 0;

			return(0);
		case WM_VSCROLL:
			switch(LOWORD(wParam)) {
			    case SB_TOP:
				    nVscrollInc = -nVscrollPos;
				    break;
			    case SB_BOTTOM:
				    nVscrollInc = nVscrollMax - nVscrollPos;
				    break;
			    case SB_LINEUP:
				    nVscrollInc = -cyClient/16;
				    break;
			    case SB_LINEDOWN:
				    nVscrollInc = cyClient/16;
				    break;
			    case SB_PAGEUP:
				    nVscrollInc = min(-1,-cyClient);
				    break;
			    case SB_PAGEDOWN:
				    nVscrollInc = max(1,cyClient);
				    break;
			    case SB_THUMBPOSITION:
			    case SB_THUMBTRACK:
#ifdef __WIN32__
				    nVscrollInc = HIWORD(wParam) - nVscrollPos;
#else
				    nVscrollInc = LOWORD(lParam) - nVscrollPos;
#endif
				    break;
			    case SB_FIND:
				    /* non standard */
#ifdef __WIN32__
				    nVscrollInc = (short)HIWORD(wParam);
#else
				    nVscrollInc = (short)LOWORD(lParam);
#endif
				    break;
			    default:
				    nVscrollInc = 0;
			}
			if ((nVscrollInc = max(-nVscrollPos, 
				min(nVscrollInc, nVscrollMax - nVscrollPos)))!=0) {
				nVscrollPos += nVscrollInc;
				ScrollWindow(hwnd,0,-nVscrollInc,NULL,NULL);
				SetScrollPos(hwnd,SB_VERT,nVscrollPos,TRUE);
				bitmap.scrollx = nHscrollPos;
				bitmap.scrolly = nVscrollPos;
				UpdateWindow(hwnd);
			}
			else if (gsdll.state != IDLE) {
			    /* We are at the top or bottom of the 
			     * scroll range.  Change page if 
			     * PageUp or PageDown pressed. */
			    int numpages = 0;
			    request_mutex();
			    if (psfile.doc != (PSDOC *)NULL)
				numpages = psfile.doc->numpages;
			    release_mutex();
			    switch(LOWORD(wParam)) {
			        case SB_PAGEUP:
				  if ((psfile.doc != (PSDOC *)NULL)
					&& (psfile.pagenum != 1)) {
				    PostMessage(hwnd ,WM_VSCROLL, SB_BOTTOM,0L);
				    gsview_command(IDM_PREV);
				  }
				  break;
			        case SB_PAGEDOWN:
				  if ((psfile.doc == (PSDOC *)NULL)
					|| (psfile.pagenum < numpages)) {
				    PostMessage(hwnd ,WM_VSCROLL, SB_TOP,0L);
				    gsview_command(IDM_NEXT);
				  }
				  break;
			    }
			}

			return(0);
		case WM_HSCROLL:
			switch(LOWORD(wParam)) {
			    case SB_TOP:
				    nHscrollInc = -nHscrollPos;
				    break;
			    case SB_BOTTOM:
				    nHscrollInc = nHscrollMax - nHscrollPos;
				    break;
			    case SB_LINEUP:
				    nHscrollInc = -cxClient/16;
				    break;
			    case SB_LINEDOWN:
				    nHscrollInc = cyClient/16;
				    break;
			    case SB_PAGEUP:
				    nHscrollInc = min(-1,-cxClient);
				    break;
			    case SB_PAGEDOWN:
				    nHscrollInc = max(1,cxClient);
				    break;
			    case SB_THUMBPOSITION:
			    case SB_THUMBTRACK:
#ifdef __WIN32__
				    nHscrollInc = HIWORD(wParam) - nHscrollPos;
#else
				    nHscrollInc = LOWORD(lParam) - nHscrollPos;
#endif
				    break;
			    case SB_FIND:
				    /* non standard */
#ifdef __WIN32__
				    nHscrollInc = (short)HIWORD(wParam);
#else
				    nHscrollInc = (short)LOWORD(lParam);
#endif
				    break;
			    default:
				    nHscrollInc = 0;
			}

			if ((nHscrollInc = max(-nHscrollPos, 
				min(nHscrollInc, nHscrollMax - nHscrollPos)))!=0) {
				nHscrollPos += nHscrollInc;
				ScrollWindow(hwnd,-nHscrollInc,0,NULL,NULL);
				SetScrollPos(hwnd,SB_HORZ,nHscrollPos,TRUE);
				bitmap.scrollx = nHscrollPos;
				bitmap.scrolly = nVscrollPos;
				UpdateWindow(hwnd);
			}

			return(0);
		case WM_KEYDOWN:
			end_button_help();
			switch(LOWORD(wParam)) {
			    case VK_HOME:
				SendMessage(hwnd,WM_VSCROLL,SB_TOP,0L);
				break;
			    case VK_END:
				SendMessage(hwnd,WM_VSCROLL,SB_BOTTOM,0L);
				break;
			    case VK_PRIOR:
				SendMessage(hwnd,WM_VSCROLL,SB_PAGEUP,0L);
				break;
			    case VK_NEXT:
				SendMessage(hwnd,WM_VSCROLL,SB_PAGEDOWN,0L);
				break;
			    case VK_UP:
				if (GetKeyState(VK_CONTROL) & 0x8000)
				  SendMessage(hwnd,WM_VSCROLL,SB_PAGEUP,0L);
				else
				  SendMessage(hwnd,WM_VSCROLL,SB_LINEUP,0L);
				break;
			    case VK_DOWN:
				if (GetKeyState(VK_CONTROL) & 0x8000)
				  SendMessage(hwnd,WM_VSCROLL,SB_PAGEDOWN,0L);
				else
				  SendMessage(hwnd,WM_VSCROLL,SB_LINEDOWN,0L);
				break;
			    case VK_LEFT:
				if (GetKeyState(VK_CONTROL) & 0x8000)
				  SendMessage(hwnd,WM_HSCROLL,SB_PAGEUP,0L);
				else
				  SendMessage(hwnd,WM_HSCROLL,SB_LINEUP,0L);
				break;
			    case VK_RIGHT:
				if (GetKeyState(VK_CONTROL) & 0x8000)
				  SendMessage(hwnd,WM_HSCROLL,SB_PAGEDOWN,0L);
				else
				  SendMessage(hwnd,WM_HSCROLL,SB_LINEDOWN,0L);
				break;
			    case VK_ESCAPE:
				if (fullscreen)
				    DestroyWindow(hwnd_fullscreen);
				break;
			}
			return(0);
		case WM_KEYUP:
			end_button_help();
			break;
		case WM_SETCURSOR:
			if (fullscreen && (hwnd == hwnd_fullscreen)) {
			    if (szWait[0] != '\0')
				SetCursor(NULL);
			    else
			        SetCursor(hCursorArrow);
			    return TRUE;
			}
			break;
		case WM_RBUTTONDOWN:
			if (fullscreen && (hwnd == hwnd_fullscreen)) {
			    SendMessage(hwndimg, WM_COMMAND, IDM_PREV, 0L);
			    break;
			}
			break;
		case WM_LBUTTONDOWN:
			if (fullscreen && (hwnd == hwnd_fullscreen)) {
			    SendMessage(hwndimg, WM_COMMAND, IDM_NEXT, 0L);
			    break;
			}
			if (hDlgModeless && in_child_client_area())
			    SendMessage(hDlgModeless, WM_COMMAND, BB_CLICK, lParam);
			else {
			    int iword ;
			    float x, y;
			    PDFLINK link;
			    if (get_cursorpos(&x, &y)) {
				HDC hdc = GetDC(hwnd);
				if ( (iword = word_find((int)x, (int)y)) >= 0 ) {
				    /* remove any current selection */
				    highlight_words(hdc, text_mark_first, text_mark_last);
				    /* mark new selection */
				    text_mark_first = text_mark_last = iword;
				    text_marking = TRUE;
				    highlight_words(hdc, text_mark_first, text_mark_last);
		    		    SetCapture(hwnd);
				}
				else {
				    /* remove selection */
				    highlight_words(hdc, text_mark_first, text_mark_last);
				    text_mark_first = text_mark_last = -1;
				    ReleaseCapture();
				}
				ReleaseDC(hwnd, hdc);
				/* pdfmark link */
				if (is_link(x, y, &link)) {
				    /* found link */
				    if (link.page == 0)
					gserror(IDS_NOLINKTARGET, NULL, 0, SOUND_ERROR);
				    else {
					gsview_unzoom();
					pending.pagenum = link.page;
					history_add(pending.pagenum);
					pending.now = TRUE;
				    }
				}
			    }
			    /* set last point for measuring distances */
			    measure_setpoint(x, y);
			}
			break;
		case WM_MOUSEMOVE:
			if (text_marking) {
			    int iword ;
			    float x, y;
			    while (!in_child_client_area()) {
				RECT rect;
				POINT pt;
				GetCursorPos(&pt);
				GetClientRect(hwnd, &rect);
				ScreenToClient(hwnd, &pt);
				if (pt.x > rect.right)
				    SendMessage(hwnd, WM_HSCROLL,SB_LINEDOWN,0L);
				if (pt.x < rect.left)
				    SendMessage(hwnd, WM_HSCROLL,SB_LINEUP,0L);
				if (pt.y > rect.bottom)
				    SendMessage(hwnd, WM_VSCROLL,SB_LINEDOWN,0L);
				if (pt.y < rect.top)
				    SendMessage(hwnd, WM_VSCROLL,SB_LINEUP,0L);
#ifdef __WIN32__
				Sleep(100);
#endif
			    }
			    if (get_cursorpos(&x, &y)) {
				if ( (iword = word_find((int)x, (int)y)) >= 0 ) {
				    if (iword != text_mark_last) {
					HDC hdc = GetDC(hwnd_image);
					int first, last;
					if ((text_mark_last-text_mark_first >= 0) != (iword-text_mark_first >= 0)) {
					    /* changing direction */
					    /* clear everything */
					    highlight_words(hdc, text_mark_first, text_mark_last);
					    /* reinstate first word */
					    text_mark_last = text_mark_first;
					    highlight_words(hdc, text_mark_first, text_mark_last);
					}
					if (iword != text_mark_last) {
					  if (iword >= text_mark_first) {
					    if (iword > text_mark_last)
						first=text_mark_last+1, last=iword;
					    else
						first=iword+1, last=text_mark_last;
					  }
					  else {
					    if (iword > text_mark_last)
						first=text_mark_last, last=iword-1;
					    else
						first=iword, last=text_mark_last-1;
					  }
					  highlight_words(hdc, first, last);
					  text_mark_last = iword;
					}
					ReleaseDC(hwnd_image, hdc);
				    }
				}
			    }
			}
			break;
		case WM_LBUTTONUP:
			ReleaseCapture();
			text_marking = FALSE;
			break;
		case WM_PAINT:
			if (quitnow) {
			    hdc = BeginPaint(hwnd, &ps);
			    rect = ps.rcPaint;
			    FillRect(hdc, &rect, GetStockObject(LTGRAY_BRUSH));
			    EndPaint(hwnd, &ps);
			    return 0;
			}
			if (gsdll.lock_device && gsdll.device)
			    gsdll.lock_device(gsdll.device, 1);
			request_mutex();
			{
			int wx,wy;
			RECT source, dest, fillrect;
			PSDOC *doc = psfile.doc;
			HBRUSH hbrush = GetStockObject(LTGRAY_BRUSH);
			hdc = BeginPaint(hwnd, &ps);
			rect = ps.rcPaint;
			SetMapMode(hdc, MM_TEXT);
			SetBkMode(hdc,OPAQUE);
			if (hwnd == hwnd_fullscreen)
			    hbrush = GetStockObject(WHITE_BRUSH);
			if (gsdll.draw && gsdll.device && 
				(bitmap.width > 1) && (bitmap.height > 1)) {

			    wx = rect.right-rect.left; /* width */
			    wy = rect.bottom-rect.top;
			    if (rect.left < display.offset.x)
				    source.left = 0;
			    else
				    source.left = rect.left - display.offset.x + nHscrollPos;
			    if (rect.top < display.offset.y)
				    source.top = 0;
			    else
				    source.top = rect.top - display.offset.y + nVscrollPos;
			    if (source.left > bitmap.width)
				    source.left = bitmap.width;
			    if (source.left + wx > bitmap.width)
				    wx = bitmap.width - source.left;
			    source.right = source.left + wx;
			    if (source.top > bitmap.height)
				    source.top = bitmap.height;
			    if (source.top + wy > bitmap.height)
				    wy = bitmap.height - source.top;
			    source.bottom = source.top + wy;

			    if (rect.left < display.offset.x)
				    dest.left = display.offset.x;
			    else
				    dest.left = rect.left;
			    if (rect.top < display.offset.y)
				    dest.top = display.offset.y;
			    else
				    dest.top = rect.top;
			    dest.right = dest.left + wx;
			    dest.bottom = dest.top + wy;
				
			    if (wx && wy)
			        gsdll.draw(gsdll.device, hdc, &dest, &source);
			    /* Fill areas around page */
			    /* not sure if display.offset.y is from top or bottom */
			    /* code here assumes top (opposite to OS/2) */
			    if (rect.bottom > bitmap.height + display.offset.y) {	/* bottom centre */
				fillrect.bottom = rect.bottom; 
				fillrect.top = bitmap.height + display.offset.y;
				fillrect.left = dest.left;
				fillrect.right = dest.right;
				FillRect(hdc, &fillrect, hbrush);
			    }
			    if (rect.top < display.offset.y) { /* top centre */
				fillrect.bottom = display.offset.y;
				fillrect.top = rect.top;
				fillrect.left = dest.left;
				fillrect.right = dest.right;
				FillRect(hdc, &fillrect, hbrush);
			    }
			    if (rect.left < display.offset.x) { /* left */
				fillrect.bottom = rect.bottom;
				fillrect.top = rect.top;
				fillrect.left = rect.left;
				fillrect.right = display.offset.x;
				FillRect(hdc, &fillrect, hbrush);
			    }
			    if (rect.right > bitmap.width + display.offset.x) { /* right */
				fillrect.bottom = rect.bottom;
				fillrect.top = rect.top;
				fillrect.left = bitmap.width + display.offset.x;
				fillrect.right = rect.right;
				FillRect(hdc, &fillrect, hbrush);
			    }
			}
			else {
			    FillRect(hdc, &rect, hbrush);
			}
			/* draw bounding box */
			if (gsdll.device && (doc != (PSDOC *)NULL) && option.show_bbox) {
			    float x, y;
			    HPEN hpen, hpen_old;
			    /* map bounding box to device coordinates */
			    x = doc->boundingbox[LLX];
			    y = doc->boundingbox[LLY];
			    map_pt_to_pixel(&x, &y);
			    rect.left   = (int)x;
			    rect.bottom = (int)y;
			    x = doc->boundingbox[URX];
			    y = doc->boundingbox[URY];
			    map_pt_to_pixel(&x, &y);
			    rect.right  = (int)x;
			    rect.top    = (int)y;

			    hpen = CreatePen(PS_DOT, 1, RGB(0,0,0));
			    hpen_old = SelectPen(hdc, hpen);
			    SelectPen(hdc, hpen);
			    SetROP2(hdc, R2_XORPEN);
			    MoveTo(hdc, rect.left, rect.bottom);
			    LineTo(hdc, rect.right, rect.bottom);
			    LineTo(hdc, rect.right, rect.top);
			    LineTo(hdc, rect.left, rect.top);
			    LineTo(hdc, rect.left, rect.bottom);
			    SetROP2(hdc, R2_COPYPEN);
			    SelectPen(hdc, hpen_old);
			    DeletePen(hpen);
			}
			/* highlight found search word */
			if (gsdll.device && display.show_find) {
			    float x, y;
			    /* map bounding box to device coordinates */
			    x = psfile.text_bbox.llx;
			    y = psfile.text_bbox.lly;
			    map_pt_to_pixel(&x, &y);
			    rect.left   = (int)x;
			    rect.bottom = (int)y;
			    x = psfile.text_bbox.urx;
			    y = psfile.text_bbox.ury;
			    map_pt_to_pixel(&x, &y);
			    rect.right  = (int)x;
			    rect.top    = (int)y;
			    if (rect.top > rect.bottom) {
				int temp = rect.top;
				rect.top = rect.bottom;
				rect.bottom = temp;
			    }

			    /* invert text */
			    InvertRect(hdc, &rect);

			}

			/* highlight marked words */
			highlight_words(hdc, text_mark_first, text_mark_last);

			highlight_links(hdc);

			EndPaint(hwnd, &ps);
			release_mutex();
			if (hwnd == hwnd_fullscreen)
			    SetCursor(NULL);
			if (gsdll.lock_device && gsdll.device)
			    gsdll.lock_device(gsdll.device, 0);
			return 0;
			}
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#define WHEEL_DELTA 120
#endif

/* parent overlapped window */
LRESULT CALLBACK _export
WndImgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
RECT rect;

    if (message == help_message) {
	WinHelp(hwndimg,szHelpName,HELP_KEY,(DWORD)szHelpTopic);
	return 0;
    } else
    switch(message) {
#ifdef __WIN32__
	case WM_MOUSEWHEEL:
	    /* If Wheel Mice become available with a step size < WHEEL_DELTA,
	     * this code will need to be rewritten */
	    if (LOWORD(wParam) == MK_SHIFT) {
		/* change page */
		int numpages = 0;
		int skip = HIWORD(wParam);
		if (skip > 0x7fff)
		   skip = skip - 0x10000L;
		skip = -skip / WHEEL_DELTA;
		request_mutex();
		if (psfile.doc != (PSDOC *)NULL)
		    numpages = psfile.doc->numpages;
		release_mutex();
	    
		if ( (skip < 0)  && 
		     (psfile.doc != (PSDOC *)NULL) && 
		     (psfile.pagenum != 1)) {
			PostMessage(hwnd ,WM_VSCROLL, SB_BOTTOM,0L);
			gs_page_skip(skip);
		}
		else if ( (skip > 0) && 
		    ((psfile.doc == (PSDOC *)NULL)
		    || (psfile.pagenum < numpages)) ) {
			PostMessage(hwnd ,WM_VSCROLL, SB_TOP,0L);
			if (psfile.doc == (PSDOC *)NULL)
			    skip = 1;
			gs_page_skip(skip);
		}
	    }
	    else if (LOWORD(wParam) ==  MK_CONTROL) {
		float scale = 1.0;
		int scroll_increment;
		scroll_increment = HIWORD(wParam);
		if (scroll_increment > 0x7fff)
		    scroll_increment -= 0x10000L;
		scroll_increment = -scroll_increment;
		if (scroll_increment > 0) {
		    while (scroll_increment > 0) {
			scale *= 1.20;
			scroll_increment -= WHEEL_DELTA;
		    }
		    gs_magnify(scale);
		}
		else if (scroll_increment < 0) {
		    while (scroll_increment < 0) {
			scale /= 1.20;
			scroll_increment += WHEEL_DELTA;
		    }
		    gs_magnify(scale);
		}
	    }
	    else if (LOWORD(wParam) == MK_LBUTTON) {
		/* horizontal scroll */
		int scroll_increment;
		GetClientRect(hwnd, &rect);
		scroll_increment = HIWORD(wParam);
		if (scroll_increment > 0x7fff)
		    scroll_increment -= 0x10000L;
		scroll_increment = -scroll_increment 
		    * ((rect.right-rect.left)/16)
		    / WHEEL_DELTA;
		PostMessage(hwnd_image, WM_HSCROLL, 
		    MAKELONG(SB_FIND, scroll_increment), 0);
	    }
	    else {
		/* vertical scroll */
		int scroll_increment;
		GetClientRect(hwnd, &rect);
		scroll_increment = HIWORD(wParam);
		if (scroll_increment > 0x7fff)
		    scroll_increment -= 0x10000L;
		scroll_increment = -scroll_increment
		    * ((rect.bottom-rect.top)/16) 
		    / WHEEL_DELTA;
		PostMessage(hwnd_image, WM_VSCROLL, 
		    MAKELONG(SB_FIND, scroll_increment), 0);
	    }
	    return 0;
#endif
	case WM_GSV16SPL:
	    hwndspl = (HWND)lParam;	   /* gsv16spl.c window handle */
	    return 0;
	case WM_GSDEVICE:
	    /* hide window if closed */
	    if (!gsdll.device) {
		ShowWindow(hwndimgchild, SW_HIDE);
		if (fullscreen)
		    DestroyWindow(hwnd_fullscreen);
	    }
	    bitmap.changed = TRUE;
	    return 0;
	case WM_GSSYNC:
	    if ( fullscreen &&
		(!IsWindowVisible(hwnd_fullscreen) || bitmap.changed) &&
		(bitmap.width > 1) && (bitmap.height > 1)) {
		ShowWindow(hwndimgchild, SW_HIDE);
		ShowWindow(hwnd_fullscreen, SW_SHOWNA);
		update_scroll_bars();
		bitmap.changed = FALSE;
	    }
	    else if ( (!IsWindowVisible(hwndimgchild) || bitmap.changed) &&
		(bitmap.width > 1) && (bitmap.height > 1)) {
		ShowWindow(hwndimgchild, SW_SHOWNA);
		update_scroll_bars();
		bitmap.changed = FALSE;
	    }
	    if ( !IsIconic(hwndimg) && !fullscreen) {  /* redraw child window */
		if (gsdll.device) {
		    /* don't erase background - the bitmap will cover it anyway */
		    InvalidateRect(hwnd_image, (LPRECT)NULL, FALSE);
		    UpdateWindow(hwnd_image);
		}
	    }
	    return 0;
	case WM_GSPAGE:
	    ignore_sync = FALSE;
	    if ( fullscreen &&
		(!IsWindowVisible(hwnd_fullscreen) || bitmap.changed) &&
		(bitmap.width > 1) && (bitmap.height > 1)) {
		ShowWindow(hwndimgchild, SW_HIDE);
		ShowWindow(hwnd_fullscreen, SW_SHOWNA);
		update_scroll_bars();
		bitmap.changed = FALSE;
	    }
	    else if ( (!IsWindowVisible(hwndimgchild) || bitmap.changed) &&
		(bitmap.width > 1) && (bitmap.height > 1)) {
		ShowWindow(hwndimgchild, SW_SHOWNA);
		update_scroll_bars();
		bitmap.changed = FALSE;
	    }
	    /* showpage has just been called */
	    play_sound(SOUND_PAGE);
	    if (IsIconic(hwndimg))    /* useless as an Icon so fix it */
		ShowWindow(hwndimg, SW_SHOWNORMAL);
	    if ( !IsIconic(hwndimg) ) {  /* redraw child window */
		if (gsdll.device) {
		    if (display.show_find) {
			scroll_to_find();
		    }
		    /* don't erase background - the bitmap will cover it anyway */
		    InvalidateRect(hwnd_image, (LPRECT)NULL, FALSE);
		    UpdateWindow(hwnd_image);
		}
	    }
	    info_link();
	    return 0;
	case WM_GSMESSBOX:
	    /* delayed message box, usually from other thread */
	    {char buf[MAXSTR];
	    load_string((int)wParam, buf, sizeof(buf));
	    message_box(buf, (int)lParam);
	    }
	    return 0;
	case WM_GSSHOWMESS:
	    /* delayed show message, usually from other thread */
	    gs_showmess();
	    return 0;
	case WM_GSREDISPLAY:
	    { PSFILE *tpsfile;
	      if (pending.psfile)
		tpsfile = pending.psfile;	/* new file, old file deleted */
	      else
		tpsfile = gsview_openfile(psfile.name);
	      if (tpsfile) {
		tpsfile->pagenum = psfile.pagenum;
		request_mutex();
		pending.psfile = tpsfile;
		pending.now = TRUE;
		release_mutex();
	      }
	    }
	    return 0;
	case WM_GSTITLE:
	    /* update title */
	    request_mutex();
	    if (psfile.name[0] != '\0') {
		char buf[256];
		GetFileTitle(psfile.name, buf, (WORD)(sizeof(buf)));
		sprintf(buf+strlen(buf), " - %s", szAppName);
		SetWindowText(hwnd, buf);
	    }
	    else
		SetWindowText(hwnd, szAppName);
	    release_mutex();
	    return 0;
	case WM_GSPERCENT:
	    percent_pending = FALSE;
	    InvalidateRect(hwndimg, (LPRECT)&info_rect, FALSE);
	    UpdateWindow(hwndimg);
	    return 0;
	case WM_GSTEXTINDEX:
	    make_text_index();
	    text_marking = FALSE;
	    text_mark_first = text_mark_last = -1;
	    return 0;
	case WM_GSWAIT:
	    info_wait(wParam);
	    return 0;
	case WM_ACTIVATE:
	    {
#ifdef WIN32
	     HWND hwnd_activate = (HWND)lParam;
#else
	     HWND hwnd_activate = (HWND)LOWORD(lParam);
#endif
	    /* We seem to be activated whenever fActive is non-zero
	     * and hwnd_activate == NULL
	     */
	    if ( (LOWORD(wParam) != WA_INACTIVE) &&
		 ( (hwnd_activate == (HWND)NULL) || 
		   (hwnd_activate == hwndimg) ||
		   (hwnd_activate == hwndimgchild) ||
		   (hwnd_activate == hwnd_image)
		 )
               )
	        reload_if_changed();
	    }
	    break;
	case WM_CREATE:
	    hwndimg = hwnd;
	    gsview_create();
	    /* Enable Drag Drop */
	    DragAcceptFiles(hwnd, TRUE);
	    break;
	case WM_CLOSE:
	    quitnow = TRUE;		 	/* exit from nested message loops */
	    pending.unload = TRUE;
	    pending.abort = TRUE;
#ifdef __WIN32__
	    if (multithread)
		SetEvent(display.event);	/* unblock display thread */
#endif
	    if (gsdll.state != UNLOADED)
		return 0;			/* don't close yet */
	    PostQuitMessage(0);
	    break;
	case WM_DESTROY:
	    /* tell GS DLL to unload */
	    quitnow = TRUE;
	    pending.unload = TRUE;

	    /* disable Drag Drop */
	    DragAcceptFiles(hwnd, FALSE);
	    PostQuitMessage(0);			/* redundant */
	    break;
/*
	case WM_ENDSESSION:
	    if (wParam)
		gsview_close();
	    return 0;
*/
	case WM_DROPFILES:
	    {
		LPSTR szFile;
		HGLOBAL hglobal;
		int i, cFiles, length;
		HDROP hdrop = (HDROP)wParam;
#ifdef __WIN32__
		cFiles = DragQueryFile(hdrop, 0xffffffff, (LPSTR)NULL, 0);
#else
		cFiles = DragQueryFile(hdrop, 0xffff, (LPSTR)NULL, 0);
#endif
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
		BOOL idle;
		BOOL addeps;
	        idle = (gsdll.state != BUSY);
		enable_menu_item(IDM_EDITMENU, IDM_COPYCLIP, gsdll.device!=NULL);
		if (OpenClipboard(hwnd)) {
		    enable_menu_item(IDM_EDITMENU, IDM_PASTETO, IsClipboardFormatAvailable(CF_DIB));
		    enable_menu_item(IDM_EDITMENU, IDM_CONVERT, 
			IsClipboardFormatAvailable(CF_DIB) || 
			IsClipboardFormatAvailable(CF_BITMAP));
		    CloseClipboard();
		}

		/* IDM_ADDEPSMENU */
		addeps =  (psfile.doc != (PSDOC *)NULL) && psfile.doc->epsf && idle;
		if (addeps) {
		    EnableMenuItem(hmenuedit, 5, MF_BYPOSITION | MF_ENABLED);
		}
		else {
		    EnableMenuItem(hmenuedit, 5, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		}
		/* Extract EPS sub menu */
		if ( (psfile.preview == IDS_EPST) || (psfile.preview == IDS_EPSW) && idle)
		    EnableMenuItem(hmenuedit, 6, MF_BYPOSITION | MF_ENABLED);
		else
		    EnableMenuItem(hmenuedit, 6, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		enable_menu_item(IDM_ADDEPSMENU, IDM_MAKEEPSU, addeps);
		addeps =  addeps && gsdll.device;
		enable_menu_item(IDM_ADDEPSMENU, IDM_MAKEEPSI, addeps);
		enable_menu_item(IDM_ADDEPSMENU, IDM_MAKEEPST4, addeps);
		enable_menu_item(IDM_ADDEPSMENU, IDM_MAKEEPST6U, addeps);
		enable_menu_item(IDM_ADDEPSMENU, IDM_MAKEEPST6P, addeps);

		enable_menu_item(IDM_EDITMENU, IDM_PSTOEDIT, idle);
		enable_menu_item(IDM_EDITMENU, IDM_TEXTEXTRACT, idle);
		enable_menu_item(IDM_EDITMENU, IDM_TEXTFIND, idle);
		enable_menu_item(IDM_EDITMENU, IDM_TEXTFINDNEXT, idle);


		/* Recent files */
		{   char buf[MAXSTR];
		    int i;
		    HMENU hmenufile = GetSubMenu(hmenu,0);
		    RemoveMenu(hmenufile, IDM_LASTFILE1, MF_BYCOMMAND);
		    RemoveMenu(hmenufile, IDM_LASTFILE2, MF_BYCOMMAND);
		    RemoveMenu(hmenufile, IDM_LASTFILE3, MF_BYCOMMAND);
		    RemoveMenu(hmenufile, IDM_LASTFILE4, MF_BYCOMMAND);
		    for (i=last_files_count; i>0; i--) {
		        sprintf(buf, "&%d %s", i, last_files[i-1]);
			if (strlen(buf)>36) {
			    int j;
			    for (j=strlen(buf); j>0; j--)
				if ((buf[j] == '/') || (buf[j] == '\\'))
				    break;
			    if (strlen(buf) - j > 28)
				memmove(buf+3, buf+strlen(buf)+1-30, 30);
			    else {
				buf[5] = buf[6] = buf[7] = '.';
				memmove(buf+8, buf+j, strlen(buf)+1-j);
			    }
			}
			InsertMenu(hmenufile, 13, MF_BYPOSITION | MF_STRING, IDM_LASTFILE1+i-1, buf);
		    }
		}
		return 0;
	    }
	    break;
	case WM_COMMAND:
	    request_mutex();
	    if (LOWORD(wParam) == IDM_DROP) {
		LPSTR szFile;
		char cmd[MAXSTR];
		szFile = GlobalLock((HGLOBAL)lParam);
		if (szFile && (lstrlen(szFile) < sizeof(cmd)))
		    lstrcpy(cmd, szFile);
		else
		    cmd[0] = '\0';
		GlobalUnlock((HGLOBAL)lParam);
		GlobalFree((HGLOBAL)lParam);
		if ((cmd[0] == '-') || (cmd[0] == '/')) {
		    switch (toupper(cmd[1])) {
			case 'P':
			  gsview_selectfile(cmd+2);
			  if (!dfreopen())
			      break;
			  if (psfile.name[0] != '\0') {
			      option.print_to_file = FALSE;
			      gsview_print();
			  }
			  dfclose();
			  break;
			case 'F':
			  gsview_selectfile(cmd+2);
			  if (!dfreopen())
			      break;
			  if (psfile.name[0] != '\0') {
			      option.print_to_file = TRUE;
			      gsview_print();
			  }
			  dfclose();
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
			release_mutex();
			return 0;	/* obtaining Bounding Box so ignore commands */
		    }
#ifdef NOTUSED
		    if (gsdll.state == BUSY) {
			/* With DLL, more than the following will be safe */
			switch(LOWORD(wParam)) {
			    case IDM_INFO:
			    case IDM_CLOSE_DONE:
			    case IDM_SAVEDIR:
			    case IDM_SETTINGS:
			    case IDM_SAVESETTINGS:
			    case IDM_SOUNDS:
			    case IDM_HELPCONTENT:
			    case IDM_HELPSEARCH:
			    case IDM_ABOUT:
			    case IDM_GSMESS:
			    case IDM_EXIT:
				/* these are safe to use when busy */
				break;
			    default:
				play_sound(SOUND_ERROR);
				release_mutex();
				return 0;	/* Command not permitted now */
			}
		    }
#endif
		    gsview_command(LOWORD(wParam));
		}
	    }
	    release_mutex();
	    return 0;
	case WM_KEYDOWN:
	case WM_KEYUP:
	    end_button_help();
	    /* pass on key presses so that child window scroll bars work */
	    SendMessage(hwnd_image, message, wParam, lParam);
	    return 0;
	case WM_SIZE:
	    /* make child window fill client area */
	    if ((wParam != SIZE_MINIMIZED) && hwndimgchild !=(HWND)NULL)
		SetWindowPos(hwndimgchild, (HWND)NULL, 
		    img_offset.x, img_offset.y,
		LOWORD(lParam)-img_offset.x, HIWORD(lParam)-img_offset.y, 
		SWP_NOZORDER | SWP_NOACTIVATE);
	    /* save window size for INIFILE */
	    if (wParam == SIZE_RESTORED) {
		    GetWindowRect(hwnd,&rect);
		    option.img_size.x = rect.right-rect.left;
		    option.img_size.y = rect.bottom-rect.top;
	    }
	    if (IsWindowVisible(hwnd))
		option.img_max = (wParam == SIZE_MAXIMIZED);
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
	    if (szWait[0] != '\0') {
		if (in_child_client_area()) {
		    SetCursor(hcWait);
		    return TRUE;
		}
	    }
	    /* track cursor and display coordinates if in child window */
	    if (gsdll.device) {
		float x, y;
		if (in_child_client_area() || prev_in_child) {
		    /* update coordinate info */
		    HFONT old_hfont;
		    HDC hdc = GetDC(hwnd);
		    if (info_font)
			old_hfont = SelectObject(hdc, info_font);
		    cursorpos_paint(hdc);
		    if (info_font)
			SelectObject(hdc, old_hfont);
		    ReleaseDC(hwnd, hdc);
		}
		prev_in_child = in_child_client_area();
		if (get_cursorpos(&x, &y)) {
		    PDFLINK link;
		    info_link();
		    if (is_link(x, y, &link)) {
			SetCursor(hcHand);
		        return TRUE;
		    }
		}
	    }
	    break;
	case WM_PARENTNOTIFY:
/*
	    if (wParam == WM_LBUTTONDOWN) {
		if (hDlgModeless && in_child_client_area())
		    SendMessage(hDlgModeless, WM_COMMAND, BB_CLICK, lParam);
		return 0;
	    }
*/
	    if (wParam == WM_RBUTTONDOWN) {
		float x, y;
		RECT rect;
		int zwidth, zheight;
		if (hDlgModeless) {
		    play_sound(SOUND_BUSY);
		    break;
		}
		request_mutex();
		GetWindowRect(hwnd_image,&rect);
		if (get_cursorpos(&x, &y)) {
		    int scrollx, scrolly;
		    zoom = !zoom;
		    display.zoom_xoffset = x;
		    display.zoom_yoffset = y;
		    if (option.quick_open) {
			scrollx = bitmap.scrollx;
			scrolly = bitmap.scrolly;
		    }
		    else
			scrollx = scrolly = 0;
		    if (rect.right - rect.left > bitmap.width)
		        zwidth = bitmap.width;
		    else
		        zwidth = rect.right - rect.left;
		    if (rect.bottom - rect.top > bitmap.height)
		        zheight = bitmap.height;
		    else
		        zheight = rect.bottom - rect.top;
		    x = (scrollx + zwidth/2)*72.0/option.xdpi;
		    y = ((bitmap.height-1) - (scrolly + zheight/2))*72.0/option.ydpi;
		    transform_point(&x, &y);
		    x *= option.xdpi/72.0;
		    y *= option.ydpi/72.0;
		    display.zoom_xoffset -= (int)(x*72.0/option.zoom_xdpi);
		    display.zoom_yoffset -= (int)(y*72.0/option.zoom_ydpi);
		}
		else {
		    zoom = FALSE;
		}
		release_mutex();
		PostMessage(hwndimg, WM_COMMAND, IDM_ZOOM, (LPARAM)0);
	    }
	    break;
	case WM_PAINT:
	    {
	    HDC hdc;
	    PAINTSTRUCT ps;
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
		hpen_highlight = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
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
    GetClientRect(hwnd_image, &rect);
    ScreenToClient(hwnd_image, &pt);
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


/* map from a coordinate in points, to a coordinate in pixels */
/* This is the opposite of the transform part of get_cursorpos */
/* Used when showing bbox */
void
map_pt_to_pixel(float *x, float *y)
{
    if (zoom) {
	/* WARNING - this doesn't cope with EPS Clip */
	*x = (*x - display.zoom_xoffset) * option.zoom_xdpi / 72.0;
	*y = (*y - display.zoom_yoffset) * option.zoom_ydpi / 72.0;
	*x = (*x * 72.0 / option.xdpi);
	*y = (*y * 72.0 / option.ydpi);
	itransform_point(x, y);
	*x = (*x * option.xdpi / 72.0) - bitmap.scrollx + display.offset.x;
	*y = -(*y * option.ydpi / 72.0) + (bitmap.height-1 - bitmap.scrolly) 
		+ display.offset.y;
    }
    else {
	*x = *x - (display.epsf_clipped ? psfile.doc->boundingbox[LLX] : 0);
	*y = *y - (display.epsf_clipped ? psfile.doc->boundingbox[LLY] : 0);
	itransform_point(x, y);
	*x = *x * option.xdpi/72.0
	      - bitmap.scrollx + display.offset.x;
	*y = -(*y * option.ydpi/72.0)
	      + (bitmap.height-1 - bitmap.scrolly) + display.offset.y;
    }
}

BOOL
get_cursorpos(float *x, float *y)
{
RECT rect;
POINT pt;
    if (gsdll.device) {
	GetClientRect(hwnd_image, &rect);
	GetCursorPos(&pt);
	ScreenToClient(hwnd_image, &pt);
	if (PtInRect(&rect, pt)) {
	    *x = bitmap.scrollx+pt.x - display.offset.x;
	    *y = bitmap.height-1 - (bitmap.scrolly+pt.y) + display.offset.y;
	    transform_cursorpos(x, y);
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
char fmt[32];
int digits = option.unitfine ? 2 : 0;
    request_mutex();
    SetBkMode(hdc, TRANSPARENT);
    FillRect(hdc, &info_coord, GetStockObject(LTGRAY_BRUSH));
    /* show coordinate */
    if (get_cursorpos(&x, &y)) {
	switch(option.unit) {
	   case IDM_UNITPT:   
	      sprintf(fmt, "%%.%df, %%.%dfpt", digits, digits);
	      sprintf(buf, fmt, x, y);
	      break;
	   case IDM_UNITMM:   
	      sprintf(fmt, "%%.%df, %%.%dfmm", digits, digits);
	      sprintf(buf, fmt, x/72*25.4, y/72*25.4);
	      break;
	   case IDM_UNITINCH:   
	      digits += 1;
	      sprintf(fmt, "%%.%df, %%.%dfin", digits, digits);
	      sprintf(buf, fmt, x/72, y/72);
	      break;
	}
	SetTextAlign(hdc, TA_RIGHT);
	TextOut(hdc, info_coord.right-1, info_coord.top, buf, strlen(buf));
	measure_paint(x, y);
    }
    release_mutex();
}

/* paint brief info area */
void
info_paint(HWND hwnd, HDC hdc)
{
RECT rect;
int i;
char buf[MAXSTR];
char fmt[MAXSTR];
HFONT old_hfont;
PSDOC *doc;
    request_mutex();
    doc = psfile.doc;
    SetBkMode(hdc, TRANSPARENT);
    if (info_font)
	old_hfont = SelectObject(hdc, info_font);
    if (info_rect.bottom) {
	GetClientRect(hwnd, &rect);
	rect.top = 0;
	rect.left = info_rect.left;
	rect.bottom = info_rect.bottom;
	FillRect(hdc, &rect, GetStockObject(LTGRAY_BRUSH));
	SelectPen(hdc, GetStockObject(BLACK_PEN));
	MoveTo(hdc, rect.left, rect.bottom);
	LineTo(hdc, rect.right, rect.bottom);
	if (is_win4) {
	    SelectPen(hdc, GetStockObject(WHITE_PEN));
	    MoveTo(hdc, rect.left, rect.top+1);
	    LineTo(hdc, rect.right, rect.top+1);
	    SelectPen(hdc, GetStockObject(BLACK_PEN));
	    MoveTo(hdc, rect.left, rect.top);
	    LineTo(hdc, rect.right, rect.top);
	}
    }
    /* write file information */
    if (psfile.name[0] != '\0') {
	i = load_string(IDS_FILE, buf, sizeof(buf));
	GetFileTitle(psfile.name, buf+i, (WORD)(sizeof(buf)-i));
	if (strlen(buf) > 36) {
	    memmove(buf+3, buf+strlen(buf)+1-32, 32);
	    buf[0] = buf[1] = buf[2] = '.';
	}
	TextOut(hdc, info_file.x, info_file.y, buf, strlen(buf));
	if (szWait[0] != '\0') {
	    sprintf(buf, szWait, percent_done);
	    TextOut(hdc, info_page.x, info_page.y, buf, strlen(buf));
	}
	else {
	  if (doc!=(PSDOC *)NULL) {
	    int n = map_page(psfile.pagenum - 1);
	    load_string(IDS_PAGEINFO, fmt, sizeof(fmt));
	    if (on_link) {
		load_string(IDS_LINKPAGE, fmt, sizeof(fmt));
		sprintf(buf, fmt, on_link_page);
	    }
	    else {
		if (doc->pages)
		    sprintf(buf, fmt, doc->pages[n].label ? doc->pages[n].label : " ",psfile.pagenum,  doc->numpages);
		else
		    sprintf(buf, fmt, " " ,psfile.pagenum,  doc->numpages);
	    }
	    if (zoom)
		load_string(IDS_ZOOMED, buf+strlen(buf), sizeof(buf)-strlen(buf));
	    TextOut(hdc, info_page.x, info_page.y, buf, strlen(buf));
	  }
	  else {
	    if (gsdll.state == IDLE)
		load_string(IDS_NOMORE, buf, sizeof(buf));
	    else {
		 load_string(IDS_PAGE, buf, sizeof(buf));
		sprintf(buf+i, "%d", psfile.pagenum);
	    }
	    TextOut(hdc, info_page.x, info_page.y, buf, strlen(buf));
	  }
	  /* show coordinate */
	  cursorpos_paint(hdc);
	}
    }
    else {
	load_string(IDS_NOFILE, buf, sizeof(buf));
	TextOut(hdc, info_file.x, info_file.y, buf, strlen(buf));
	if (szWait[0] != '\0') {
	    sprintf(buf, szWait, percent_done);
	    TextOut(hdc, info_page.x, info_page.y, buf, strlen(buf));
	}
    }
    if (info_font)
	SelectObject(hdc, old_hfont);
    release_mutex();
}


HWND hbutton_info;

void
end_button_help(void)
{
    if (hbutton_info) {
	ReleaseCapture();
	hbutton_info = (HWND)NULL;
	SetFocus(hwndimg);
	InvalidateRect(hwndimg, &info_rect, FALSE);
	UpdateWindow(hwndimg);
    }
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
	    end_button_help();
	    GetWindowRect(hwnd, &rect);
	    GetCursorPos(&pt);
	    SetFocus(GetParent(hwnd));
	    if (PtInRect(&rect, pt))
		SendMessage(GetParent(hwnd), WM_COMMAND, GetWindowID(hwnd), 0L);
	    }
	    break;
	case WM_MOUSEMOVE:
	    {
		POINT pt;
	        HDC hdc; 
	        HFONT old_hfont;
		RECT rect;
		HWND hwnd_cursor;
		char buf[MAXSTR];
		if (GetActiveWindow() != hwndimg)
		    break;  /* ignore if GSview not the top window */
		GetCursorPos(&pt);
		hwnd_cursor = WindowFromPoint(pt);
		if (hwnd != hwnd_cursor) {
		    end_button_help();
		}
		else if (hbutton_info != hwnd) {
		    hbutton_info = hwnd;
		    SetCapture(hwnd);
		    hdc = GetDC(hwndimg);
		    load_string(GetWindowID(hwnd), buf, sizeof(buf));
		    SetBkMode(hdc, TRANSPARENT);
		    if (info_rect.bottom) {
			GetClientRect(hwnd, &rect);
			rect.top = 2;
			rect.left = info_rect.left;
			rect.bottom = info_rect.bottom-1;
			rect.right = info_rect.right;
			FillRect(hdc, &rect, GetStockObject(LTGRAY_BRUSH));
		    }
		    if (info_font)
			old_hfont = SelectObject(hdc, info_font);
		    TextOut(hdc, info_file.x, info_file.y, buf, strlen(buf));
		    if (info_font)
			SelectObject(hdc, old_hfont);
		    ReleaseDC(hwndimg, hdc);
		}
	    }
	    break;
    }
    return CallWindowProc(lpfnButtonWndProc, hwnd, message, wParam, lParam);
}

void
update_scroll_bars(void)
{
    /* Cause update of scroll bars etc. */
    RECT rect;
    GetClientRect(hwnd_image, &rect);
    SendMessage(hwnd_image, WM_SIZE, SIZE_RESTORED, MAKELONG(rect.right-rect.left, rect.bottom-rect.top));
}

/* Thread which loads Ghostscript DLL for display */
#ifdef __BORLANDC__
#pragma argsused
#endif
void 
gs_thread(void *arg)
{
    while (!quitnow) {
	if (!pending.now)
	    wait_event();
	if (!quitnow)
	    gs_process();
    }

    /* signal that we have finished */
    display.tid = 0;
    post_img_message(WM_QUIT, 0);   /* shut down application */
}


/* Return TRUE if is OK to exit */
BOOL
query_close(void)
{
    /* tell GS DLL to unload */
    quitnow = TRUE;
    pending.unload = TRUE;
#ifdef __WIN32__
    if (multithread)
        SetEvent(display.event);	/* unblock display thread */
#endif
    return TRUE;
}


/* remove temporary files etc. */
void
gsview_close()
{
    psfile_free(&psfile);
    if (option.settings)
	write_profile(); 
    else
	write_profile_last_files();	/* always save MRU files */
    SetCursor(GetClassCursor((HWND)NULL));
    if (info_font)
	DeleteObject(info_font);
    if (hcCrossHair)
	DestroyCursor(hcCrossHair);
    if (hcHand)
	DestroyCursor(hcHand);
#ifdef __WIN32__
    if (multithread) {
	CloseHandle(display.event);
	CloseHandle(hmutex_ps);
	DeleteCriticalSection(&crit_sec);
    }
#endif
    unload_zlib();
    return;
}


void
copy_clipboard()
{
HGLOBAL hglobal;
HPALETTE hpalette;
    if ( text_index && (text_mark_first != -1) && (text_mark_last != -1)) {
	/* copy text, not image */
	int first, last, line;
	int length;
	int i;
	LPSTR data;
	first = text_mark_first;
	last = text_mark_last;
	if (first > last) {
	    first = text_mark_last;
	    last = text_mark_first;
	}
	line = text_index[first].line;
	length = 1;
	for (i=first; i<=last; i++) {
	    if (text_index[i].line != line) {
	        line = text_index[i].line;
		length += 2;
	    }
	    length += strlen( text_words + text_index[i].word ) + 1;
	}
	if ((hglobal = GlobalAlloc(GHND | GMEM_SHARE, length)) == (HGLOBAL)NULL) {
	    message_box("out of memory", 0);
	    return;
	}
	if ((data = GlobalLock(hglobal)) == (LPSTR)NULL) {
	    message_box("out of memory", 0);
	    return;
	}
	line = text_index[first].line;
	for (i=first; i<=last; i++) {
	    if (text_index[i].line != line) {
	        line = text_index[i].line;
		lstrcpy(data, "\r\n");
		data += lstrlen(data);
	    }
	    lstrcpy(data, text_words + text_index[i].word);
	    lstrcat(data, " ");
	    data += lstrlen(data);
	}
	GlobalUnlock(hglobal);
	OpenClipboard(hwndimg);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hglobal);
	CloseClipboard();
	return;
    }

    if (gsdll.device) {
	hglobal = (*gsdll.copy_dib)(gsdll.device);
	hpalette = (*gsdll.copy_palette)(gsdll.device);
	if (hglobal == (HGLOBAL)NULL) {
	    MessageBox(hwndimg, "Not enough memory to Copy to Clipboard", 
		szAppName, MB_OK | MB_ICONEXCLAMATION);
	    return;
	}
	OpenClipboard(hwndimg);
	EmptyClipboard();
	SetClipboardData(CF_DIB, hglobal);
	if (hpalette)
	    SetClipboardData(CF_PALETTE, hpalette);
	CloseClipboard();
    }
}


#ifdef __BORLANDC__
#pragma argsused
#endif
/* enable or disable a menu item */
void
enable_menu_item(int menuid, int itemid, BOOL enabled)
{
	if (enabled)
	    EnableMenuItem(hmenu, itemid, MF_ENABLED);
	else
	    EnableMenuItem(hmenu, itemid, MF_DISABLED | MF_GRAYED);
}

/* if found word is not visible, scroll window to make it visible */
void
scroll_to_find(void)
{
    RECT rect, rect_client;
    float x, y;

    request_mutex();
    SendMessage(hwnd_image, WM_SETREDRAW, FALSE, 0);
    /* first translate found box to window coordinates */
    x = psfile.text_bbox.llx;
    y = psfile.text_bbox.lly;
    map_pt_to_pixel(&x, &y);
    rect.left   = (int)x;
    rect.bottom = (int)y;
    x = psfile.text_bbox.urx;
    y = psfile.text_bbox.ury;
    map_pt_to_pixel(&x, &y);
    rect.right  = (int)x;
    rect.top    = (int)y;

    GetClientRect(hwnd_image, &rect_client);

    /* scroll to bring the middle left to the centre of the window */
    if ((rect.left < rect_client.left) || (rect.right > rect_client.right))
#ifdef __WIN32__
	PostMessage(hwnd_image, WM_HSCROLL, MAKELONG(SB_FIND, rect.left - ((rect_client.right-rect_client.left)/2)), 0);
#else
	PostMessage(hwnd_image, WM_HSCROLL, SB_FIND, MAKELONG(rect.left - ((rect_client.right-rect_client.left)/2), 0));
#endif

    if ((rect.top < rect_client.top) || (rect.bottom > rect_client.bottom))
#ifdef __WIN32__
	PostMessage(hwnd_image, WM_VSCROLL, MAKELONG(SB_FIND, (rect.bottom+rect.top - rect_client.bottom-rect_client.top)/2), 0);
#else
	PostMessage(hwnd_image, WM_VSCROLL, SB_FIND, MAKELONG((rect.bottom+rect.top - rect_client.bottom-rect_client.top)/2, 0));
#endif
    SendMessage(hwnd_image, WM_SETREDRAW, TRUE, 0);
    release_mutex();
}


/* highlight words from first to last inclusive */
/* first may be > last */
/* word = -1 means nothing to mark */
void
highlight_words(HDC hdc, int first, int last)
{
    RECT rect;
    float x, y;
    TEXTINDEX *text;
    int i;
    if ((first == -1) || (last == -1))
	return;


    if ((first > text_index_count) || (last > text_index_count)) {
	gs_addmess("\nhighlight_words called with invalid arguments\n");
	return;
    }
    if (first > last) {
        int temp = first;
	first = last;
	last = temp;
    }

    for (i = first; i<=last; i++) {
	text = &text_index[i];
	/* highlight found word */
	/* map bounding box to device coordinates */
	x = text->bbox.llx;
	y = text->bbox.lly;
	map_pt_to_pixel(&x, &y);
	rect.left   = (int)x;
	rect.bottom = (int)y;
	x = text->bbox.urx;
	y = text->bbox.ury;
	map_pt_to_pixel(&x, &y);
	rect.right  = (int)x;
	rect.top    = (int)y;
	if (rect.top > rect.bottom) {
	    int temp = rect.top;
	    rect.top = rect.bottom;
	    rect.bottom = temp;
	}
	if (rect.left > rect.right) {
	    int temp = rect.right;
	    rect.right = rect.left;
	    rect.left = temp;
	}

	/* invert text */
	InvertRect(hdc, &rect);
    }
}


void
highlight_links(HDC hdc)
{
PDFLINK link;
int i = 0;
float x, y;
RECT rect;
LOGBRUSH lb;
HBRUSH hbrush, hbrush_old;
HPEN hpen, hpen_old;
int w2;
    
    while ( pdf_get_link(i, &link) ) {
	i++;
	if (link.border_width) {
	    /* map bounding box to device coordinates */
	    x = link.bbox.llx;
	    y = link.bbox.lly;
	    map_pt_to_pixel(&x, &y);
	    rect.left   = (int)x;
	    rect.bottom = (int)y;
	    x = link.bbox.urx;
	    y = link.bbox.ury;
	    map_pt_to_pixel(&x, &y);
	    rect.right  = (int)x;
	    rect.top    = (int)y;
	    if (rect.top > rect.bottom) {
		int temp = rect.top;
		rect.top = rect.bottom;
		rect.bottom = temp;
	    }
	    if (rect.left > rect.right) {
		int temp = rect.right;
		rect.right = rect.left;
		rect.left = temp;
	    }
	    /* draw border */
	    SetROP2(hdc, R2_COPYPEN);
	    if (link.colour_valid) {
	        hpen = CreatePen(PS_SOLID, (int)(link.border_width+0.5), 
		    RGB((int)(link.colour_red*255 +0.5),
		        (int)(link.colour_green*255 +0.5),
			(int)(link.colour_blue*255 +0.5)));
	    }
	    else {
	        hpen = CreatePen(PS_SOLID, (int)(link.border_width+0.5), RGB(0,255,255));
	        SetROP2(hdc, R2_XORPEN);
	    }
	    hpen_old = SelectPen(hdc, hpen);
	    SelectPen(hdc, hpen);
	    lb.lbStyle = BS_NULL;	/* hollow = transparent */
	    lb.lbColor = 0;		/* ignored */
	    lb.lbHatch = 0;		/* ignored */
	    hbrush = CreateBrushIndirect(&lb);
	    hbrush_old = SelectBrush(hdc, hbrush);
	    w2 = (int)((link.border_width+0.5)/2);
	    RoundRect(hdc, rect.left-w2, rect.top-w2, 
		    rect.right+w2, rect.bottom+w2, 
		    2 * ((int)(link.border_xr+0.5)),
		    2 * ((int)(link.border_yr+0.5)));
	    SelectBrush(hdc, hbrush_old);
	    DeleteBrush(hbrush);
	    SelectPen(hdc, hpen_old);
	    DeletePen(hpen);
	}
    }
}


void
info_link(void)
{
float x, y;
PDFLINK link;
HFONT old_hfont;
HDC hdc;
    if (get_cursorpos(&x, &y)) {
	if (is_link(x, y, &link)) {
	    on_link = TRUE;
	    on_link_page = link.page;
	    hdc = GetDC(hwndimg);
	    if (info_font)
		old_hfont = SelectObject(hdc, info_font);
	    info_paint(hwndimg, hdc);
	    if (info_font)
		SelectObject(hdc, old_hfont);
	    ReleaseDC(hwndimg, hdc);
	}
	else if (on_link)
	{
	    on_link = FALSE;
	    hdc = GetDC(hwndimg);
	    if (info_font)
		old_hfont = SelectObject(hdc, info_font);
	    info_paint(hwndimg, hdc);
	    if (info_font)
		SelectObject(hdc, old_hfont);
	    ReleaseDC(hwndimg, hdc);
	}
    }
}

void
gsview_fullscreen_end(void)
{
    if (fullscreen) {
	gs_addmess("Full Screen ending\r\n");
        DestroyWindow(hwnd_fullscreen);
    }
}

void
gsview_fullscreen(void)
{
WNDCLASS wndclass;
int width, height;
char class_name[MAXSTR];
static BOOL class_registered;

	if (!gsdll.device)
	    return;
        if (!IsWindow(hwnd_fullscreen)) {
	    strcpy(class_name, szAppName);
	    strcat(class_name, "_fullscreen");
	    if (!class_registered) {
		/* register the window class */
		wndclass.style = 0;
		wndclass.lpfnWndProc = WndImgChildProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = phInstance;
		wndclass.hIcon = NULL;
		wndclass.hCursor = NULL;
		hCursorArrow = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
		wndclass.hbrBackground =  GetStockObject(WHITE_BRUSH);
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = class_name;
		RegisterClass(&wndclass);
		class_registered = TRUE;
	    }

	    width = GetSystemMetrics(SM_CXSCREEN);
	    height = GetSystemMetrics(SM_CYSCREEN);

	    fullscreen = TRUE;
	    hwnd_fullscreen = CreateWindow(class_name, (LPSTR)szAppName,
		      WS_POPUP,
		      0, 0, 
		      width, height,
		      NULL /* parent = desktop */, 
		      NULL, phInstance, (void FAR *)NULL);

	    if (hwnd_fullscreen && IsWindow(hwnd_fullscreen)) {
		hwnd_image = hwnd_fullscreen;
		ShowWindow(hwnd_fullscreen, SW_SHOWNORMAL);
		gs_addmess("Full Screen started\r\n");
	    }
	    else {
		gs_addmess("Full Screen failed\r\n");
		fullscreen = FALSE;
	    }
        }


	return;
}


/* Set the current resolution to fill the window.
 * If neither width nor height match, fit whole page
 * into window.  If either width or height match
 * the window size, fit the height or width respectively.
 */
void 
gsview_fitwin(void) 
{
RECT rect;
int width, height;
float dpi, xdpi, ydpi, xdpi2, ydpi2;
	if (psfile.ispdf) {
	    if (option.epsf_clip) {
		width = psfile.doc->boundingbox[URX] 
			- psfile.doc->boundingbox[LLX];
		height = psfile.doc->boundingbox[URY] 
			- psfile.doc->boundingbox[LLY];
	    }
	    else {
		width = psfile.doc->default_page_boundingbox[URX] 
			- psfile.doc->default_page_boundingbox[LLX];
		height = psfile.doc->default_page_boundingbox[URY] 
			- psfile.doc->default_page_boundingbox[LLY];
	    }
	}
	else {
	    width = get_paper_width();
	    height = get_paper_height();
	}

	if (display.orientation & 1) {
	    /* page is rotated 90 degrees */
	    int temp = width;
	    width = height;
	    height = temp;
	}


	if (fullscreen)
	    GetClientRect(hwnd_image, &rect);
	else {
	    /* get size including scroll bars area */
	    GetClientRect(hwndimg, &rect);
	    rect.left += img_offset.x;
	    rect.top += img_offset.y;
	}
	xdpi = (rect.right - rect.left) * 72.0 / width;
	ydpi = (rect.bottom - rect.top) * 72.0 / height;
	if (fullscreen) {
	    xdpi2 = xdpi;
	    ydpi2 = ydpi;
	}
	else {
	    /* These are the resolutions allowing for a scroll bar */
	    xdpi2 = (rect.right - rect.left - GetSystemMetrics(SM_CXVSCROLL)) 
		* 72.0 / width;
	    ydpi2 = (rect.bottom - rect.top - GetSystemMetrics(SM_CYHSCROLL)) 
		* 72.0 / height;
	}

	if (display.orientation & 1) {
	    /* page is rotated 90 degrees */
	    float ftemp;
	    ftemp = xdpi;
	    xdpi = ydpi;
	    ydpi = ftemp;
	    ftemp = xdpi2;
	    xdpi2 = ydpi2;
	    ydpi2 = ftemp;
	}

	if ( ((xdpi + 0.5) > option.xdpi) && (xdpi - 0.5) < option.xdpi) {
	    /* Width matches. Set size based on height. */
	    if (fullscreen || (ydpi <= xdpi))
	        dpi = ydpi;
	    else
	        dpi = ydpi2;
	}
	else if ( ((ydpi + 0.5) > option.ydpi) && (ydpi - 0.5) < option.ydpi) {
	    /* Height matches. Set size based on width. */
	    if (fullscreen || (xdpi <= ydpi))
	        dpi = xdpi;
	    else
	        dpi = xdpi2;
	}
	else  {
	    /* Neither width nor height match.  Fit the whole page. */
	    if (xdpi > ydpi)
		    dpi = ydpi;
	    else
		    dpi = xdpi;
	}
#ifdef DEBUG
	{
	char buf[MAXSTR];
	sprintf(buf, "\nrect=%d %d %d %d\n", 
	rect.left, rect.top, rect.right, rect.bottom);
	gs_addmess(buf);
	sprintf(buf, "size=%d %d\n", width, height);
	gs_addmess(buf);
	sprintf(buf, "old dpi=%f %f\n", option.xdpi, option.ydpi);
	gs_addmess(buf);
	sprintf(buf, "dpi=%f %f %f %f\n", xdpi, ydpi, xdpi2, ydpi2);
	gs_addmess(buf);
	sprintf(buf, "final dpi=%f\n", dpi);
	gs_addmess(buf);
	}
#endif
	option.xdpi = option.ydpi = dpi;
	gs_resize();
}
