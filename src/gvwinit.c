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

/* gvwinit.c */
/* Initialisation routings for Windows GSview */
#include "gvwin.h"

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
#ifdef __WIN32__
	/* skip over EXE name */
	while ( *lpszCmdLine && (*lpszCmdLine!=' ')) 
		lpszCmdLine++;
	while ( *lpszCmdLine && (*lpszCmdLine==' ')) 
		lpszCmdLine++;
#endif
	if (lstrlen(lpszCmdLine) != 0) {
	    /* open file specified on command line */
	    HGLOBAL hglobal;
	    LPSTR szFile;
	    hglobal = GlobalAlloc(GHND | GMEM_SHARE, lstrlen(lpszCmdLine)+1);
	    if (hglobal) {
	        szFile = GlobalLock(hglobal);
		lstrcpy(szFile, lpszCmdLine);
	        GlobalUnlock(hglobal);
		PostMessage(hwnd, WM_COMMAND, IDM_DROP, (LPARAM)hglobal);
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
char workdir[MAXSTR];
char filedir[MAXSTR];
int length = 64;

	while (length && !SetMessageQueue(length))
	    length--;	/* reduce size and try again */
	if (length == 0)
	    exit(0);	/* panic */
	
	if ((LOBYTE(version)<<8) + HIBYTE(version) >= 0x30a)
	    is_win31 = TRUE;

	/* get path to EXE */
	GetModuleFileName(phInstance, szExePath, sizeof(szExePath));
	if ((p = strrchr(szExePath,'\\')) != (char *)NULL)
	    p++;
	else
	    p = szExePath;
	*p = '\0';

	/* get path to INI file */
	szIniFile[0] = '\0';
	/* strcpy(szIniFile, szExePath); */
	strcat(szIniFile, INIFILE);

	/* get path to help file */
	strcpy(szHelpName, szExePath);
	p = szHelpName + strlen(szHelpName);
	LoadString(phInstance, IDS_HELPFILE, p, sizeof(szHelpName) - (p-szHelpName));

	/* help message for GetOpenFileName Dialog Box */
	help_message = RegisterWindowMessage(HELPMSGSTRING);
	LoadString(phInstance, IDS_TOPICROOT, szHelpTopic, sizeof(szHelpTopic));

        load_string(IDS_WAIT, szWait, sizeof(szWait));	/* generic wait message */
	
	/* register the window class */
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndImgProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = sizeof(LONG);
	wndclass.hInstance = phInstance;
	wndclass.hIcon = LoadIcon(phInstance,MAKEINTRESOURCE(ID_GSVIEW));
	wndclass.hCursor = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
	wndclass.hbrBackground =  GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szClassName;
	RegisterClass(&wndclass);

	strcpy(option.gscommand, szExePath);
	strcat(option.gscommand, DEFAULT_GSCOMMAND);
	strcat(option.gscommand, " -I");
	strcat(option.gscommand, szExePath);
	strcpy(option.gscommand+strlen(option.gscommand)-1, ";");
	strcat(option.gscommand, szExePath);
	strcat(option.gscommand, "fonts;");
	strcat(option.gscommand, "c:\\psfonts");
	option.img_origin.x = CW_USEDEFAULT;
	option.img_origin.y = CW_USEDEFAULT;
	option.img_size.x = CW_USEDEFAULT;
	option.img_size.y = CW_USEDEFAULT;
	option.unit = IDM_UNITPT;
	option.quick = TRUE;
	option.settings = TRUE;
	option.button_show = TRUE;
	option.fit_page = TRUE;
	option.safer = TRUE;
	option.media = IDM_LETTER;
	strcpy(option.medianame, "letter");
	option.user_width = 610;
	option.user_height = 792;
	option.epsf_clip = FALSE;
	option.epsf_warn = FALSE;
	option.ignore_dsc = FALSE;
	option.redisplay = TRUE;
	option.orientation = IDM_PORTRAIT;
	option.swap_landscape = FALSE;
	option.xdpi = DEFAULT_RESOLUTION;
	option.ydpi = DEFAULT_RESOLUTION;
	option.save_dir = TRUE;
	/* defaults if entry not in gsview.ini */
	hmenu = LoadMenu(phInstance, "gsview_menu");
	haccel = LoadAccelerators(phInstance, "gsview_accel");
	getcwd(workdir, sizeof(workdir));
	/* read entries from gsview.ini */
	read_profile();

	hwndimg = CreateWindow(szClassName, (LPSTR)szAppName,
		  WS_OVERLAPPEDWINDOW,
		  option.img_origin.x, option.img_origin.y, 
		  option.img_size.x, option.img_size.y, 
		  NULL, NULL, phInstance, (void FAR *)NULL);

	/* load DLL for sounds */
	if (is_win31) {
	    /* MMSYSTEM.DLL requires Windows 3.1, so to allow gsview to run
	       under Windows 3.0 we can't use the import library */
	    hlib_mmsystem = LoadLibrary("MMSYSTEM.DLL");
#ifdef __WIN32__
	    if (hlib_mmsystem != NULL) {
#else
	    if (hlib_mmsystem >= HINSTANCE_ERROR) {
#endif
		lpfnSndPlaySound = (FPSPS)GetProcAddress(hlib_mmsystem, "sndPlaySound");
	    }
	    else {
		gserror(IDS_SOUNDNOMM, NULL, MB_ICONEXCLAMATION, -1);
		hlib_mmsystem = (HINSTANCE)NULL;
	    }
	}

#ifdef __WIN32__
	/* skip over EXE name */
	while ( *lpszCmdLine && (*lpszCmdLine!=' ')) 
		lpszCmdLine++;
	while ( *lpszCmdLine && (*lpszCmdLine==' ')) 
		lpszCmdLine++;
#endif

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
	    HGLOBAL hglobal;
	    LPSTR szFile;
	    hglobal = GlobalAlloc(GHND | GMEM_SHARE, lstrlen(lpszCmdLine)+1);
	    if (hglobal) {
	        szFile = GlobalLock(hglobal);
		lstrcpy(szFile, lpszCmdLine);
	        GlobalUnlock(hglobal);
		PostMessage(hwndimg, WM_COMMAND, IDM_DROP, (LPARAM)hglobal);
	    }
	    /* ignore last saved directory */
	    /* use directory of file if given, or work directory */
	    if ((lpszCmdLine[0] == '/') || (lpszCmdLine[0] == '-')) {
		lpszCmdLine += 2;
		while (*lpszCmdLine && (*lpszCmdLine == ' '))
		    lpszCmdLine++;
	    }
	    lstrcpy(filedir, lpszCmdLine);
	    if ( (p = strrchr(filedir, '\\')) == (char *)NULL ) {
	        if ( (p = strrchr(filedir, ':')) == (char *)NULL )
		    strcpy(filedir, workdir);  /* no path so use work directory */
		else
		    *(++p) = '\0';
	    }
	    else
		*(++p) = '\0';
	    if (!((strlen(filedir)==2) && isalpha(filedir[0]) && (filedir[1]==':')))
	        chdir(filedir);
	    if (isalpha(filedir[0]) && (filedir[1]==':'))
		(void) setdisk(toupper(filedir[0])-'A');
	}
	play_sound(SOUND_START);
        if (changed_version) {
	    message_box("The installed version of GSview has changed.  \
Please read the Installation help and then correctly set\r\
Options | Ghostscript Command", 0);
	    load_string(IDS_TOPICINSTALL, szHelpTopic, sizeof(szHelpTopic));
	    get_help();
        }
}

/* create gsview window menu bar and buttons */
void
gsview_create()
{
int i;
char cReplace;
WNDCLASS wndclass;
HGLOBAL hglobal;
short FAR *pButtonID;
TEXTMETRIC tm;
HDC hdc;
HWND hbutton;
WNDPROC	lpfnMenuButtonProc;
POINT char_size;		/* size of default text characters */
POINT button_size, button_shift;
char thismedia[20];

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
	display.planes = GetDeviceCaps(hdc, PLANES);
	display.bitcount = GetDeviceCaps(hdc, BITSPIXEL);
	
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
	if (!option.button_show)
	    button_rect.right = 0;
	img_offset.x = button_rect.right + (option.button_show ? 1 : 0);
	img_offset.y = info_rect.bottom + 1;
	info_file.x = info_rect.left + 2;
	info_file.y = 0;
	info_coord.left = info_rect.left + 20 * char_size.x;
	info_coord.right = info_rect.left + 34 * char_size.x;
	info_coord.top = 0;
	info_coord.bottom = char_size.y;
	info_page.x = info_rect.left + 36 * char_size.x + 2;
	info_page.y = 0;

	/* check menu items */
	for (i=IDM_LETTER; i<IDM_USERSIZE; i++) {
	    get_menu_string(IDM_MEDIAMENU, i, thismedia, sizeof(thismedia));
	    if (!stricmp(thismedia, option.medianame)) {
		break;
	    }
	}
	option.media = i;
	strncpy(option.medianame,thismedia,sizeof(option.medianame));
	CheckMenuItem(hmenu, option.unit, MF_BYCOMMAND | MF_CHECKED);
	CheckMenuItem(hmenu, option.media, MF_BYCOMMAND | MF_CHECKED);
	CheckMenuItem(hmenu, option.orientation, MF_BYCOMMAND | MF_CHECKED);
	CheckMenuItem(hmenu, gsview_depth_to_menu(option.depth), MF_BYCOMMAND | MF_CHECKED);
	if (option.epsf_clip)
		CheckMenuItem(hmenu, IDM_EPSFCLIP, MF_BYCOMMAND | MF_CHECKED);
	if (option.epsf_warn)
		CheckMenuItem(hmenu, IDM_EPSFWARN, MF_BYCOMMAND | MF_CHECKED);
	if (option.ignore_dsc)
		CheckMenuItem(hmenu, IDM_IGNOREDSC, MF_BYCOMMAND | MF_CHECKED);
	if (option.swap_landscape)
		CheckMenuItem(hmenu, IDM_SWAPLANDSCAPE, MF_BYCOMMAND | MF_CHECKED);
	if (option.save_dir) 
		CheckMenuItem(hmenu, IDM_SAVEDIR, MF_BYCOMMAND | MF_CHECKED);
	if (option.button_show) 
		CheckMenuItem(hmenu, IDM_BUTTONSHOW, MF_BYCOMMAND | MF_CHECKED);
	if (option.fit_page) 
		CheckMenuItem(hmenu, IDM_FITPAGE, MF_BYCOMMAND | MF_CHECKED);
	if (option.quick) 
		CheckMenuItem(hmenu, IDM_QUICK, MF_BYCOMMAND | MF_CHECKED);
	if (option.safer) 
		CheckMenuItem(hmenu, IDM_SAFER, MF_BYCOMMAND | MF_CHECKED);
	if (option.redisplay) 
		CheckMenuItem(hmenu, IDM_AUTOREDISPLAY, MF_BYCOMMAND | MF_CHECKED);
	if (option.settings)
		CheckMenuItem(hmenu, IDM_SAVESETTINGS, MF_BYCOMMAND | MF_CHECKED);

	hcWait = LoadCursor((HINSTANCE)NULL, IDC_WAIT);

	/* add buttons */
	lpfnMenuButtonProc = (WNDPROC)MakeProcInstance((FARPROC)MenuButtonProc, phInstance);
	GetClassInfo((HINSTANCE)NULL, "button", &wndclass);	/* get default button class info */
	lpfnButtonWndProc = wndclass.lpfnWndProc;
	
	hglobal = LoadResource(phInstance, FindResource(phInstance, MAKEINTRESOURCE(IDR_BUTTON), RT_RCDATA));
	if ( (pButtonID = (short FAR *)LockResource(hglobal)) == (short FAR *)NULL)
		return;
	
	for (i=0; pButtonID[i]; i++) {
	    hbutton = CreateWindow("button", NULL,
			WS_CHILD | (option.button_show ? WS_VISIBLE : 0) | BS_OWNERDRAW,
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
	button_rect.right = option.button_show ? real_button_width : 0;
	img_offset.x = button_rect.right + (option.button_show ? 1 : 0);
	if (!option.button_show) {
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
	if (option.button_show) {
	    InvalidateRect(hwndimg, &rect, FALSE);
	    UpdateWindow(hwndimg);
	    while (bp) {
	        ShowWindow(bp->hbutton, SW_SHOWNA);
	        bp = bp->next;
	    }
	}
}

