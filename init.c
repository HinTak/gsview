/*
 * init.c   -- Initialisation functions for GSVIEW.EXE,
 *             a graphical interface for MS-Windows Ghostscript
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
#define NeedFunctionPrototypes 1
#include "ps.h"
#include "gsview.h"

/* Open/Save File Dialog Box */
OPENFILENAME ofn;
char szOFilename[MAXSTR];	/* filename for OFN */
char szOFilter[256];		/* filter for OFN */
/* buttons */
WNDPROC lpfnButtonWndProc;	/* default button WndProc */
struct buttonlist {
   HWND hbutton;
   struct buttonlist *next;
};
struct buttonlist *buttonhead, *buttontail;
int real_button_width;

/* Don't start another instance - use previous instance */
void
gsview_init0(LPSTR lpszCmdLine)
{
	HWND hwnd = FindWindow(szClassName, szAppName);
	BringWindowToTop(hwnd);
	if (lstrlen(lpszCmdLine) != 0) {
	    /* open file specified on command line */
	    LPSTR szFile = GlobalAllocPtr(GHND | GMEM_SHARE, lstrlen(lpszCmdLine)+1);
	    if (szFile) {
		lstrcpy(szFile, lpszCmdLine);
		PostMessage(hwnd, WM_COMMAND, IDM_DROP, (LPARAM)szFile);
	    }
	}
}

/* main initialisation */
void
gsview_init1(LPSTR lpszCmdLine)
{
WNDCLASS wndclass;
WORD version = LOWORD(GetVersion());
char *p;

	if ((LOBYTE(version)<<8) + HIBYTE(version) >= 0x30a)
	    is_win31 = TRUE;

	/* get path to help file */
	GetModuleFileName(phInstance, szHelpName, sizeof(szHelpName));
	if ((p = strrchr(szHelpName,'\\')) != (char *)NULL)
	    p++;
	else
	    p = szHelpName;
	LoadString(phInstance, IDS_HELPFILE, p, sizeof(szHelpName) - (p-szHelpName));

	/* help message for GetOpenFileName Dialog Box */
	help_message = RegisterWindowMessage(HELPMSGSTRING);
	LoadString(phInstance, IDS_TOPICROOT, szHelpTopic, sizeof(szHelpTopic));
	
	/* register the window class */
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndImgProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = sizeof(LONG);
	wndclass.hInstance = phInstance;
	wndclass.hIcon = LoadIcon(phInstance,"gsview");
	wndclass.hCursor = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
	wndclass.hbrBackground =  GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szClassName;
	RegisterClass(&wndclass);

	/* defaults if entry not in gsview.ini */
	strcpy(szGSwin, DEFAULT_GSCOMMAND);
	img_origin.x = img_origin.y = CW_USEDEFAULT;
	img_size.x = img_size.y = CW_USEDEFAULT;
	xdpi = ydpi = DEFAULT_RESOLUTION;
	settings = TRUE;
	button_show = TRUE;
	quick = TRUE;
	timeout = DEFAULT_TIMEOUT;
	save_dir = TRUE;
	media = IDM_LETTER;
	epsf_clip = FALSE;
	orientation = IDM_PORTRAIT;
	swap_landscape = FALSE;
	hmenu = LoadMenu(phInstance, "gsview_menu");
	haccel = LoadAccelerators(phInstance, "gsview_accel");
	/* read entries from gsview.ini */
	read_profile();

	hwndimg = CreateWindow(szClassName, (LPSTR)szAppName,
		  WS_OVERLAPPEDWINDOW,
		  img_origin.x, img_origin.y, 
		  img_size.x, img_size.y, 
		  NULL, NULL, phInstance, (void FAR *)NULL);

	/* load DLL for sounds */
	if (is_win31) {
	    /* MMSYSTEM.DLL requires Windows 3.1, so to allow gsview to run
	       under Windows 3.0 we can't use the import library */
	    hlib_mmsystem = LoadLibrary("MMSYSTEM.DLL");
	    if (hlib_mmsystem >= HINSTANCE_ERROR) {
		lpfnSndPlaySound = (FPSPS)GetProcAddress(hlib_mmsystem, "sndPlaySound");
	    }
	    else {
		gserror(IDS_SOUNDNOMM, NULL, MB_ICONEXCLAMATION, -1);
		hlib_mmsystem = (HINSTANCE)NULL;
	    }
	}

	if (lstrlen(lpszCmdLine) >= 2) {
	    if ( ((lpszCmdLine[0] == '/') || (lpszCmdLine[0] == '-'))
	     &&  ((lpszCmdLine[1] == 'D') || (lpszCmdLine[1] == 'd')) ) {
		debug = TRUE;
		lpszCmdLine += 2;
		while (*lpszCmdLine && (*lpszCmdLine == ' '))
		    lpszCmdLine++;
	    }
 	}
	if (lstrlen(lpszCmdLine) != 0) {
	    /* open file specified on command line */
	    LPSTR szFile = GlobalAllocPtr(GHND, lstrlen(lpszCmdLine)+1);
	    if (szFile) {
		lstrcpy(szFile, lpszCmdLine);
		PostMessage(hwndimg, WM_COMMAND, IDM_DROP, (LPARAM)szFile);
	    }
	}

	play_sound(SOUND_START);
}

/* create gsview window menu bar and buttons */
void
gsview_create()
{
int i;
char cReplace;
WNDCLASS wndclass;
HGLOBAL hglobal;
int FAR *pButtonID;
TEXTMETRIC tm;
HDC hdc;
HWND hbutton;
WNDPROC	lpfnMenuButtonProc;
POINT char_size;		/* size of default text characters */
POINT button_size, button_shift;

	/* setup OPENFILENAME struct */
	if (!LoadString(phInstance, IDS_FILTER, szOFilter, sizeof(szOFilter)-1))
		return;
	cReplace = szOFilter[strlen(szOFilter)-1];
	for (i=0; szOFilter[i] != '\0'; i++)
	    if (szOFilter[i] == cReplace)
		szOFilter[i] = '\0';
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwndimg;
	ofn.lpstrFilter = szOFilter;
	ofn.nFilterIndex = FILTER_PS;
	ofn.lpstrFile = szOFilename;
	ofn.nMaxFile = sizeof(szOFilename);
	ofn.lpstrFileTitle = (LPSTR)NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrTitle = (LPSTR)NULL;
	ofn.lpstrInitialDir = (LPSTR)NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_SHOWHELP;
	LoadString(phInstance, IDS_TOPICROOT, szHelpTopic, sizeof(szHelpTopic));

	/* add menu to image window */
	SetMenu(hwndimg, hmenu);

	/* get default text size */
	hdc = GetDC(hwndimg);
	GetTextMetrics(hdc,(LPTEXTMETRIC)&tm);
	ReleaseDC(hwndimg,hdc);
	char_size.x = tm.tmAveCharWidth;
	char_size.y = tm.tmHeight;

	/* set size of info area, buttons and offset to child window */
	info_rect.left = 0;
	info_rect.right = info_rect.left + 60 * char_size.x;
	info_rect.top = 0;
	info_rect.bottom = char_size.y;
	button_size.x = 24;
	button_size.y = 24;
	button_shift.x = 0;
	button_shift.y = button_size.y - 1;
	button_rect.top = info_rect.bottom;
	button_rect.left = -1;
	button_rect.right = button_size.x - 2;
	button_rect.bottom = 0;		/* don't care */
	real_button_width = button_rect.right;
	if (!button_show)
	    button_rect.right = 0;
	img_offset.x = button_rect.right + (button_show ? 1 : 0);
	img_offset.y = info_rect.bottom + 1;
	info_file.x = info_rect.left + 2;
	info_file.y = 0;
	info_page.x = info_rect.left + 34 * char_size.x + 2;
	info_page.y = 0;
	info_coord.left = info_rect.left + 20 * char_size.x;
	info_coord.right = info_rect.left + 32 * char_size.x;
	info_coord.top = 0;
	info_coord.bottom = char_size.y;

	/* check menu items */
	CheckMenuItem(hmenu, media, MF_BYCOMMAND | MF_CHECKED);
	CheckMenuItem(hmenu, orientation, MF_BYCOMMAND | MF_CHECKED);
	if (epsf_clip)
		CheckMenuItem(hmenu, IDM_EPSFCLIP, MF_BYCOMMAND | MF_CHECKED);
	if (epsf_warn)
		CheckMenuItem(hmenu, IDM_EPSFWARN, MF_BYCOMMAND | MF_CHECKED);
	if (swap_landscape)
		CheckMenuItem(hmenu, IDM_SWAPLANDSCAPE, MF_BYCOMMAND | MF_CHECKED);
	if (save_dir) 
		CheckMenuItem(hmenu, IDM_SAVEDIR, MF_BYCOMMAND | MF_CHECKED);
	if (button_show) 
		CheckMenuItem(hmenu, IDM_BUTTONSHOW, MF_BYCOMMAND | MF_CHECKED);
	if (quick) 
		CheckMenuItem(hmenu, IDM_QUICK, MF_BYCOMMAND | MF_CHECKED);
	if (redisplay) 
		CheckMenuItem(hmenu, IDM_AUTOREDISPLAY, MF_BYCOMMAND | MF_CHECKED);
	if (settings)
		CheckMenuItem(hmenu, IDM_SAVESETTINGS, MF_BYCOMMAND | MF_CHECKED);

	hcWait = LoadCursor((HINSTANCE)NULL, IDC_WAIT);

	/* add buttons */
	lpfnMenuButtonProc = (WNDPROC)MakeProcInstance((FARPROC)MenuButtonProc, phInstance);
	GetClassInfo((HINSTANCE)NULL, "button", &wndclass);	/* get default button class info */
	lpfnButtonWndProc = wndclass.lpfnWndProc;
	
	hglobal = LoadResource(phInstance, FindResource(phInstance, "gsview_button", RT_RCDATA));
	if ( (pButtonID = (int FAR *)LockResource(hglobal)) == (int FAR *)NULL)
		return;
	
	for (i=0; pButtonID[i]; i++) {
	    hbutton = CreateWindow("button", NULL,
			WS_CHILD | (button_show ? WS_VISIBLE : 0) | BS_OWNERDRAW,
			button_rect.left + i * button_shift.x,
			button_rect.top  + i * button_shift.y,
			button_size.x, button_size.y,
			hwndimg, (HMENU)pButtonID[i],
			phInstance, NULL);
	    SetWindowLong(hbutton, GWL_WNDPROC, (LONG)lpfnMenuButtonProc);
	    if (hbutton) {
		if (buttonhead == (struct buttonlist *)NULL)
		    buttontail = buttonhead = (struct buttonlist *)malloc(sizeof(struct buttonlist));
		else {
		    buttontail->next = (struct buttonlist *)malloc(sizeof(struct buttonlist)); 
		    buttontail = buttontail->next;
		}
		buttontail->hbutton = hbutton;
		buttontail->next = NULL;
	    }
	}
	FreeResource(hglobal);
}

void
show_buttons(void)
{
struct buttonlist *bp = buttonhead;
RECT rect;
	button_rect.right = button_show ? real_button_width : 0;
	img_offset.x = button_rect.right + (button_show ? 1 : 0);
	if (!button_show) {
	    while (bp) {
	        ShowWindow(bp->hbutton, SW_HIDE);
	        bp = bp->next;
	    }
	    if (hwndimgchild == (HWND)NULL) {
	        GetClientRect(hwndimg, &rect);
	        rect.right = real_button_width + 1;
	        rect.top = button_rect.top;
	        InvalidateRect(hwndimg, &rect, TRUE);
	        UpdateWindow(hwndimg);
	    }
	}
	GetClientRect(hwndimg, &rect);
	SetWindowPos(hwndimgchild, (HWND)NULL, rect.left+img_offset.x, rect.top+img_offset.y,
		rect.right-img_offset.x, rect.bottom-img_offset.y, 
		SWP_NOZORDER | SWP_NOACTIVATE);
	rect.right = real_button_width + 1;
	rect.top = button_rect.top;
	if (button_show) {
	    InvalidateRect(hwndimg, &rect, FALSE);
	    UpdateWindow(hwndimg);
	    while (bp) {
	        ShowWindow(bp->hbutton, SW_SHOWNA);
	        bp = bp->next;
	    }
	}
}

/* read settings fron INI file */
void
read_profile()
{
int i;
char profile[128];
LPSTR file = INIFILE;
LPSTR section = INISECTION;
	GetPrivateProfileString(section, "Origin", "", profile, sizeof(profile), file);
	if (sscanf(profile,"%d %d", &img_origin.x, &img_origin.y) != 2) {
		img_origin.x = CW_USEDEFAULT;
		img_origin.y = CW_USEDEFAULT;
	}
	GetPrivateProfileString(section, "Size", "", profile, sizeof(profile), file);
	if (sscanf(profile,"%d %d", &img_size.x, &img_size.y) != 2) {
		img_origin.x = CW_USEDEFAULT;
		img_origin.y = CW_USEDEFAULT;
	}
	GetPrivateProfileString(section, "SaveSettings", "", profile, sizeof(profile), file);
	if (sscanf(profile,"%d", &i) == 1)
		settings = i;
	GetPrivateProfileString(section, "ButtonBar", "", profile, sizeof(profile), file);
	if (sscanf(profile,"%d", &i) == 1)
		button_show = i;
	GetPrivateProfileString(section, "Resolution", "", profile, sizeof(profile), file);
	if (sscanf(profile,"%f %f", &xdpi, &ydpi) != 2) {
		xdpi = DEFAULT_RESOLUTION;
		ydpi = DEFAULT_RESOLUTION;
	}
	GetPrivateProfileString(section, "Media", "", profile, sizeof(profile), file);
	if (strlen(profile)!=0) {
		char medianame[20];
		for (i=IDM_LETTER; i<IDM_USERSIZE; i++) {
		    GetMenuString(hmenu, i, medianame, sizeof(medianame), MF_BYCOMMAND);
		    if (!stricmp(medianame, profile)) {
		        break;
		    }
		}
		media = i;
        }
	GetPrivateProfileString(section, "UserSize", "", profile, sizeof(profile), file);
	if (sscanf(profile,"%d %d", &user_width, &user_height) != 2) {
		/* this gives 640x480 pixels at 96dpi */
		user_width = 480;
		user_height = 360;
	}
	GetPrivateProfileString(section, "EpsfClip", "", profile, sizeof(profile), file);
	if (sscanf(profile,"%d", &i) == 1)
		epsf_clip = i;
	GetPrivateProfileString(section, "EpsfWarn", "", profile, sizeof(profile), file);
	if (sscanf(profile,"%d", &i) == 1)
		epsf_warn = i;
	GetPrivateProfileString(section, "Orientation", "", profile, sizeof(profile), file);
	if (sscanf(profile,"%d", &i) == 1)
		orientation = i+IDM_PORTRAIT;
	GetPrivateProfileString(section, "SwapLandscape", "", profile, sizeof(profile), file);
	if (sscanf(profile,"%d", &i) == 1)
		swap_landscape = i;
	GetPrivateProfileString(section, "QuickOpen", "", profile, sizeof(profile), file);
	if (sscanf(profile,"%d", &i) == 1)
		quick = i;
	GetPrivateProfileString(section, "AutoRedisplay", "", profile, sizeof(profile), file);
	if (sscanf(profile,"%d", &i) == 1)
		redisplay = i;
	GetPrivateProfileString(section, "Timeout", "", profile, sizeof(profile), file);
	if (sscanf(profile,"%d", &i) == 1)
		timeout = i;
	if (timeout == 0)
		timeout = 120;
	GetPrivateProfileString(section, "SaveLastDir", "", profile, sizeof(profile), file);
	if (sscanf(profile,"%d", &i) == 1)
		save_dir = i;
	if (save_dir) {
	    GetPrivateProfileString(section, "LastDir", "", profile, sizeof(profile), file);
	    if (!((strlen(profile)==2) && isalpha(profile[0]) && (profile[1]==':')))
	        chdir(profile);
	    if (isalpha(profile[0]) && (profile[1]==':'))
		(void) setdisk(toupper(profile[0])-'A');
	}
	GetPrivateProfileString(section, "Ghostscript", "", profile, sizeof(profile), file);
	if (profile[0] == '\0')
		strcpy(szGSwin, DEFAULT_GSCOMMAND);
	else
		strcpy(szGSwin, profile);
	GetPrivateProfileString(section, "Printer", ",", profile, sizeof(profile), file);
	strcpy(device_name, strtok(profile,","));
	strcpy(device_resolution, strtok(NULL, ","));
	for (i=0; i<NUMSOUND; i++) {
		GetPrivateProfileString(section, sound[i].entry, sound[i].file, profile, sizeof(profile), file);
		strcpy(sound[i].file, profile);
	}
}

/* write settings to INI file */
void
write_profile()
{
char profile[MAXSTR];
LPSTR file = INIFILE;
LPSTR section = INISECTION;
int i;
	sprintf(profile, "%d %d", img_origin.x, img_origin.y);
	WritePrivateProfileString(section, "Origin", profile, file);
	sprintf(profile, "%d %d", img_size.x, img_size.y);
	WritePrivateProfileString(section, "Size", profile, file);
	sprintf(profile, "%d", settings);
	WritePrivateProfileString(section, "SaveSettings", profile, file);
	sprintf(profile, "%d", button_show);
	WritePrivateProfileString(section, "ButtonBar", profile, file);
	sprintf(profile, "%g %g", xdpi, ydpi);
	WritePrivateProfileString(section, "Resolution", profile, file);
	if (media == IDM_USERSIZE)
	    strcpy(profile, "User Defined");
	else
	    GetMenuString(hmenu, media, profile, sizeof(profile), MF_BYCOMMAND);
	WritePrivateProfileString(section, "Media", profile, file);
	sprintf(profile, "%u %u", user_width, user_height);
	WritePrivateProfileString(section, "UserSize", profile, file);
	sprintf(profile, "%d", epsf_clip);
	WritePrivateProfileString(section, "EpsfClip", profile, file);
	sprintf(profile, "%d", epsf_warn);
	WritePrivateProfileString(section, "EpsfWarn", profile, file);
	sprintf(profile, "%d", orientation - IDM_PORTRAIT);
	WritePrivateProfileString(section, "Orientation", profile, file);
	sprintf(profile, "%d", swap_landscape);
	WritePrivateProfileString(section, "SwapLandscape", profile, file);
	sprintf(profile, "%d", quick);
	WritePrivateProfileString(section, "QuickOpen", profile, file);
	sprintf(profile, "%d", redisplay);
	WritePrivateProfileString(section, "AutoRedisplay", profile, file);
	sprintf(profile, "%d", timeout);
	WritePrivateProfileString(section, "Timeout", profile, file);
	if (save_dir) {
	    getcwd(profile, sizeof(profile));
	    WritePrivateProfileString(section, "LastDir", profile, file);
	}
	WritePrivateProfileString(section, "Ghostscript", szGSwin, file);
	if (device_name[0] != '\0') {
	    sprintf(profile,"%s,%s",device_name,device_resolution);
	    WritePrivateProfileString(section, "Printer", profile, file);
	}
	for (i=0; i<NUMSOUND; i++)
	    WritePrivateProfileString(section, sound[i].entry, sound[i].file, file);
}

