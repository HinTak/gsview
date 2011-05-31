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

/* gsgrab.c */
/* Main window routine for GSgrab */

/* Add a line "GSGRAB="  to the [Ports] section of win.ini */
/* Print using a PostScript driver connected to GSGRAB */
/* This program loops, waiting until it can get exclusive access */
/* to c:\windows\gsgrab */
/* When this occurs, it runs Ghostscript, waits until Ghostscript */
/* has exited then deletes c:\windows\gsgrab */


/* add WM_TIMER */
/* make shift sensing global to EditProc and ButtonProc */
/* make main window a simple status display, with button */
/*  to bring up dialog box to set other options */
/* find location of window directory */
/* start minimized */

#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "gsgrab.h"

HWND hwndgrab;
HICON hicongrab;
HINSTANCE phInstance;
POINT char_size;
HFILE hf;

char szAppName[] = "GSgrab";
char szHelpName[] = "gsgrab.hlp";
char szCommand[256];
char szGrabFile[128];
char szStatus[128];

char szGhostscript[256];
char szPrinter[64];
char szResolution[64];
char szPort[64];
char szInterval[20];

char szIniName[]="gsgrab.ini";
char szOptionSection[]="Options";
char szDevSection[]="Devices";
char szDefCommand[]="c:\\gs\\gswin -Ic:\\gs;c:\\gs\\fonts;c:\\psfonts";
char szUnknown[]="<Unknown>";

BOOL printing;
int interval;
BOOL bTimerSet;			/* true if TIMER running */
#define ID_MYTIMER 1

HWND hbutton[NBUTTON];
void set_focus(HWND hwnd, WORD id);
void change_focus(HWND hwnd, WORD key, BOOL shift);
LRESULT CALLBACK _export ButtonProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
WNDPROC	lpfnButtonWndProc;


int PASCAL 
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int cmdShow)
{
	MSG msg;
	phInstance = hInstance;

	if (hPrevInstance) {
	    hwndgrab = FindWindow(szAppName, szAppName);
	    BringWindowToTop(hwndgrab);
	    return 1;
	}

	init_window();
	strcpy(szStatus, "Idle");
	/* ShowWindow(hwnd, SW_SHOWMINIMIZED); */
	ShowWindow(hwndgrab, cmdShow);
	init_profile();

	if (lpszCmdLine[0] != '\0')
	    MessageBox(hwndgrab, "Arguments Ignored", szAppName, MB_OK);

	while (GetMessage(&msg, (HWND)NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DestroyWindow(hwndgrab);
	return 0;
}

BOOL
start_timer(void)
{
	if (SetTimer(hwndgrab, ID_MYTIMER, interval*1000, NULL) != 0) {
		bTimerSet = TRUE;
		return TRUE;
	}
	bTimerSet = FALSE;
	MessageBox(hwndgrab, "No timers available", szAppName, MB_OK);
	return FALSE;
}

void
stop_timer(void)
{
	if (bTimerSet)
		KillTimer(hwndgrab, ID_MYTIMER);
	bTimerSet = FALSE;
}

BOOL
call_gs(void)
{
    MSG msg;
    HINSTANCE hinst;

    strcpy(szStatus, "Starting Ghostscript");
    InvalidateRect(hwndgrab, NULL, TRUE);
    UpdateWindow(hwndgrab);

    printing = TRUE;
/*
    hinst = (HINSTANCE)WinExec(szCommand, SW_SHOWMINNOACTIVE);
*/
    hinst = (HINSTANCE)WinExec(szCommand, SW_SHOWNORMAL);
#ifdef __WIN32__
    if (hinst == NULL)
#else
    if (hinst < HINSTANCE_ERROR)
#endif
    {
	char buf[256];
	strcpy(szStatus, "Idle");
	InvalidateRect(hwndgrab, NULL, TRUE);
	strcpy(buf, "Can't run ");
	strcat(buf, szCommand);
	MessageBox(hwndgrab, buf, szAppName, MB_OK);
	hinst = (HINSTANCE)NULL;
	printing = FALSE;
	return FALSE;
    }

    strcpy(szStatus, "Printing with Ghostscript");
    InvalidateRect(hwndgrab, NULL, TRUE);
    UpdateWindow(hwndgrab);

    /* wait until Ghostscript finishes */
    while (GetModuleUsage(hinst)) {
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    strcpy(szStatus, "Idle");
    InvalidateRect(hwndgrab, NULL, TRUE);
    UpdateWindow(hwndgrab);

    printing = FALSE;
    return TRUE;
}


/* subclass button WndProc to give focus back to parent window */
LRESULT CALLBACK _export
ButtonProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
static BOOL shift = FALSE;
	switch(message) {
	    case WM_KEYDOWN:
		switch(LOWORD(wParam)) {
		    case VK_TAB:
		    case VK_UP:
		    case VK_DOWN:
		    case VK_LEFT:
		    case VK_RIGHT:
			change_focus(hwnd, LOWORD(wParam), shift);
			return 0;
		    case VK_RETURN:
			wParam = VK_SPACE;
			break;
		    case VK_SHIFT:
			shift = TRUE;
			break;
		}
		break;
	    case WM_KEYUP:
		switch(LOWORD(wParam)) {
		    case VK_SHIFT:
			shift = FALSE;
			break;
		}
		break;
	}
	return CallWindowProc(lpfnButtonWndProc, hwnd, message, wParam, lParam);
}


void
set_focus(HWND hwnd, WORD newid)
{
/* int id = GetWindowWord(hwnd, GWW_ID); */
HWND newhwnd = GetDlgItem(GetParent(hwnd), newid); 
	/* may need to do some fudging if it gives focus to the wrong window */
	SetFocus(newhwnd);
}

#define VK_BACKTAB 11	/* vertical tab code */
void
change_focus(HWND hwnd, WORD key, BOOL shift)
{
int id = GetWindowWord(hwnd, GWW_ID);
    if (shift && (key==VK_TAB))
	key = VK_BACKTAB;
	
    switch(id) {
        case IDC_SETUP:
	    switch(key) {
		case VK_TAB:
		case VK_RIGHT:
		case VK_DOWN:
			set_focus(hwnd, IDC_CANCEL);
			break;
		case VK_BACKTAB:
		case VK_LEFT:
		case VK_UP:
			set_focus(hwnd, IDC_ABOUT);
			break;
	    }
	    break;
        case IDC_CANCEL:
	    switch(key) {
		case VK_TAB:
		case VK_RIGHT:
		case VK_DOWN:
			set_focus(hwnd, IDC_HELP);
			break;
		case VK_BACKTAB:
		case VK_LEFT:
		case VK_UP:
			set_focus(hwnd, IDC_SETUP);
			break;
	    }
	    break;
        case IDC_HELP:
	    switch(key) {
		case VK_TAB:
		case VK_RIGHT:
		case VK_DOWN:
			set_focus(hwnd, IDC_ABOUT);
			break;
		case VK_BACKTAB:
		case VK_LEFT:
		case VK_UP:
			set_focus(hwnd, IDC_CANCEL);
			break;
	    }
	    break;
        case IDC_ABOUT:
	    switch(key) {
		case VK_TAB:
		case VK_RIGHT:
		case VK_DOWN:
			set_focus(hwnd, IDC_SETUP);
			break;
		case VK_BACKTAB:
		case VK_LEFT:
		case VK_UP:
			set_focus(hwnd, IDC_HELP);
			break;
	    }
	    break;
    }
}

/* Window Procedure */
LRESULT CALLBACK _export
WndGSgrabProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
HMENU hmenu;
PAINTSTRUCT ps;
HDC hdc;
    switch(message) {
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		TextOut(hdc, 1*char_size.x, 6*char_size.y, "Status:", 7);
		TextOut(hdc, 8*char_size.x, 6*char_size.y, szStatus, strlen(szStatus));
		DrawIcon(hdc, 22*char_size.x, char_size.y, hicongrab);
		EndPaint(hwnd, &ps);
		return 0;
	case WM_CREATE:
		hmenu = GetSystemMenu(hwnd,FALSE);
		EnableMenuItem(hmenu, SC_SIZE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		EnableMenuItem(hmenu, SC_MAXIMIZE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		AppendMenu(hmenu, MF_SEPARATOR, 0, NULL);
		AppendMenu(hmenu, MF_ENABLED, IDC_ABOUT, "&About");
		break;
/*	case WM_CLOSE:   */
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_TIMER:
	    if ( (hf = _lopen(szGrabFile, READ | OF_SHARE_EXCLUSIVE))
		  != HFILE_ERROR ) {
	        stop_timer();
		_lclose(hf);
	        if (call_gs()) {
		    unlink(szGrabFile);
	            start_timer();
		}
	        else {
		    if (MessageBox(hwndgrab, "Restart Timer?", szAppName, MB_YESNO) == IDYES)
	                start_timer();
	        }
	    }
	    break;
	case WM_COMMAND:
	    switch(LOWORD(wParam)) {
		case IDC_SETUP:
		    stop_timer();
		    if (setup())
			init_profile();
		    start_timer();
		    return 0;
		case IDC_CANCEL:
		    PostQuitMessage(0);
		    return 0;
		case IDC_HELP:
		    WinHelp(hwndgrab,szHelpName,HELP_CONTENTS,(DWORD)NULL);
		    return 0;
		case IDC_ABOUT:
		    show_about();
		    return 0;
	    }
	case WM_SYSCOMMAND:
        	switch(LOWORD(wParam)) {
		    case IDC_ABOUT:
			show_about();
			return 0;
		}
		break;
	case WM_SETFOCUS:
		set_focus(hbutton[0], IDC_SETUP);
		break;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}
