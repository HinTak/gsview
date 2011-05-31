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

/* gsginit.c */
/* initialisation functions for GSgrab */

#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include "gsgrab.h"

#define WINDOWWIDTH	(130*char_size.x/4)
#define WINDOWHEIGHT	(72*char_size.y/8)
#define BUTTONWIDTH	(32*char_size.x/4)
#define BUTTONHEIGHT	(14*char_size.y/8)
int ibutton[NBUTTON] = {IDC_SETUP, IDC_CANCEL, IDC_HELP, IDC_ABOUT};
char *tbutton[NBUTTON] = {"Setup", "Cancel", "Help", "About"};
POINT pbutton[NBUTTON] = { {4,8}, {4, 28}, {48, 8}, {48,28} };

void
init_window(void)
{
int i;
WNDCLASS wndclass;
WNDPROC	lpfnButtonProc;
HDC hdc;
TEXTMETRIC tm;
HFONT hfont;

	/* register the window class */
	wndclass.style = 0;
	wndclass.lpfnWndProc = WndGSgrabProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = phInstance;
	wndclass.hIcon = hicongrab = LoadIcon(phInstance, MAKEINTRESOURCE(GRABICON));
	wndclass.hCursor = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
	wndclass.hbrBackground =  GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;
	RegisterClass(&wndclass);

	hdc = GetDC(NULL);  /* desktop */
	hfont = GetStockObject(SYSTEM_FIXED_FONT);
	SelectObject(hdc, hfont);
	GetTextMetrics(hdc, (LPTEXTMETRIC)&tm);
	ReleaseDC(NULL, hdc);
	char_size.x = tm.tmAveCharWidth;
	char_size.y = tm.tmHeight;

	hwndgrab = CreateWindow(szAppName, (LPSTR)szAppName,
		  WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		  CW_USEDEFAULT, CW_USEDEFAULT, 
		  WINDOWWIDTH, WINDOWHEIGHT,
		  NULL, NULL, phInstance, (void FAR *)NULL);

	/* add buttons */
	lpfnButtonProc = (WNDPROC)MakeProcInstance((FARPROC)ButtonProc, phInstance);
	GetClassInfo((HINSTANCE)NULL, "button", &wndclass);	/* get default button class info */
	lpfnButtonWndProc = wndclass.lpfnWndProc;
	
	for (i=0; i<NBUTTON; i++) {
	    hbutton[i] = CreateWindow("button", (LPSTR)tbutton[i], 
		WS_CHILD | WS_VISIBLE, 
		pbutton[i].x*char_size.x/4, pbutton[i].y*char_size.y/8,
		BUTTONWIDTH, BUTTONHEIGHT,
		hwndgrab, (HMENU)ibutton[i], phInstance, NULL);
	    SetWindowLong(hbutton[i], GWL_WNDPROC, (LONG)lpfnButtonProc);
	}
	SetFocus(GetDlgItem(hwndgrab, IDC_SETUP));
	return;
}

void
init_profile(void)
{
char buf[64];
	GetPrivateProfileString(szOptionSection, "Ghostscript", szDefCommand, szGhostscript, sizeof(szGhostscript), szIniName);
	GetPrivateProfileString(szOptionSection, "Printer", "", szPrinter, sizeof(szPrinter), szIniName);
	GetPrivateProfileString(szOptionSection, "Resolution", "", szResolution, sizeof(szResolution), szIniName);
	GetPrivateProfileString(szOptionSection, "Port", szUnknown, szPort, sizeof(szPort), szIniName);
	GetPrivateProfileString(szOptionSection, "Interval", "10", szInterval, sizeof(szInterval), szIniName);
	interval = atoi(szInterval);
	if (interval > 60) {
	    interval = 60;
	    strcpy(szInterval, "60");
	}

	GetWindowsDirectory(szGrabFile, sizeof(szGrabFile));
	strcat(szGrabFile,"\\GSGRAB");
	strcpy(szCommand, szGhostscript);
	strcat(szCommand, " -dNOPAUSE -sDEVICE=");
	strcat(szCommand, szPrinter);
	strcat(szCommand, " -r");
	strcat(szCommand, szResolution);
	if (strcmp(szPort, szUnknown)!=0) {
	    strcat(szCommand, " -sOutputFile=");
	    strcat(szCommand, szPort);
	}
	strcat(szCommand, " ");
	strcat(szCommand, szGrabFile);
	strcat(szCommand, " quit.ps");
/*
	strcat(szCommand, "-c quit");
*/
	if (strlen(szCommand) >= 127)
	    MessageBox(hwndgrab, "Ghostscript command line is too long", szAppName, MB_OK);

	GetPrivateProfileString(szOptionSection, "Version", "", buf, sizeof(buf), szIniName);
	if (strcmp(buf, GSGRAB_VERSION)) {
	    /* changed or new version */
	    PostMessage(hwndgrab, WM_COMMAND, IDC_SETUP, (LPARAM)0);
	    WinHelp(hwndgrab,szHelpName,HELP_KEY,(DWORD)"Installation");
	}
}

