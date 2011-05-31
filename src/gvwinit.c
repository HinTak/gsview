/* Copyright (C) 1993-1997, Russell Lang.  All rights reserved.
  
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
#include <ddeml.h>

/* Open/Save File Dialog Box */
OPENFILENAME ofn;
char szOFilename[MAXSTR];	/* filename for OFN */
/* buttons */
WNDPROC lpfnButtonWndProc;	/* default button WndProc */
struct buttonlist {
   HWND hbutton;
   struct buttonlist *next;
};
struct buttonlist *buttonhead, *buttontail;
int real_button_width;

FILE *logfile;

BOOL parse_args(LPSTR str);

/* Don't start another instance - use previous instance */
void
gsview_init0(LPSTR lpszCmdLine)
{
	HWND hwnd = FindWindow(szClassName, NULL);
	BringWindowToTop(hwnd);
#if __BORLANDC__ == 0x452
	/* avoid bug in BC++ 4.0 */
#ifdef __WIN32__
	/* skip over EXE name */
	while ( *lpszCmdLine && (*lpszCmdLine!=' ')) 
		lpszCmdLine++;
	while ( *lpszCmdLine && (*lpszCmdLine==' ')) 
		lpszCmdLine++;
#endif
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


char workdir[MAXSTR];

/* returns TRUE if language change successful */
BOOL
load_language(int language)
{   /* load language dependent resources */
char langdll[MAXSTR];
HINSTANCE hInstance;
    /* load language dependent resources */
    strcpy(langdll, szExePath);
#ifdef __WIN32__
#ifdef DECALPHA
    strcat(langdll, "gsvwda");
#else
    strcat(langdll, "gsvw32");
#endif
#else
    strcat(langdll, "gsvw16");
#endif
    switch (language) {
	case IDM_LANGDE:
	    strcat(langdll, "de");
	    break;
	case IDM_LANGFR:
	    strcat(langdll, "fr");
	    break;
	case IDM_LANGEN:
	default:
	    if ((hlanguage != (HINSTANCE)NULL) && (hlanguage != phInstance))
		FreeLibrary(hlanguage);
	    /* Don't load a DLL */
	    hlanguage = phInstance;
	    return TRUE;
    }
    strcat(langdll, ".dll");
    hInstance = LoadLibrary(langdll);
    if (hInstance >= (HINSTANCE)HINSTANCE_ERROR) {
	if ((hlanguage != (HINSTANCE)NULL) && (hlanguage != phInstance))
	    FreeLibrary(hlanguage);
	hlanguage = hInstance;

	load_string(IDS_GSVIEWVERSION, langdll, sizeof(langdll));
	if (strcmp(GSVIEW_VERSION, langdll) != 0)
	    message_box("Language resources version doesn't match GSview EXE", 0);

	return TRUE;
    }
    
    return FALSE;
}

void
change_language(void)
{
char *p;
    hmenu = LoadMenu(hlanguage, "gsview_menu");
    SetMenu(hwndimg, hmenu);
    haccel = LoadAccelerators(hlanguage, "gsview_accel");

    WinHelp(hwndimg,szHelpName,HELP_QUIT,(DWORD)NULL);
    /* get path to help file */
    strcpy(szHelpName, szExePath);
    p = szHelpName + strlen(szHelpName);
    load_string(IDS_HELPFILE, p, sizeof(szHelpName) - (int)(p-szHelpName));

    load_string(IDS_TOPICROOT, szHelpTopic, sizeof(szHelpTopic));
    init_check_menu();
    InvalidateRect(hwndimg, (LPRECT)NULL, FALSE);
}

#ifdef __BORLANDC__
#pragma argsused
#endif
/* language dialog box */
BOOL CALLBACK _export
LanguageDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
		case IDOK:
                case IDCANCEL:
                    EndDialog(hDlg, 0);
                    return(TRUE);
                case IDM_LANGEN:
                case IDM_LANGDE:
                case IDM_LANGFR:
                    EndDialog(hDlg, LOWORD(wParam));
                    return(TRUE);
                default:
                    return(FALSE);
            }
    }
    return(FALSE);
}

/* prompt to change language if Windows language doesn't match */
/* GSview language */
void 
check_language(void)
{
char winlang[MAXSTR];
int language;
    GetProfileString("Intl", "sLanguage", "ENG", winlang, sizeof(winlang));
    /* if Window language doesn't match GSview language */
    if ( ((option.language == IDM_LANGEN) && strnicmp(winlang, "EN", 2))
      || ((option.language == IDM_LANGDE) && stricmp(winlang, "DEU"))
      || ((option.language == IDM_LANGFR) && strnicmp(winlang, "FR", 2))
	)
    {
#ifdef __WIN32__
	language = DialogBoxParam(hlanguage, "LanguageDlgBox", hwndimg, LanguageDlgProc, (LPARAM)NULL);
#else
	DLGPROC lpProcLanguage;
	lpProcLanguage = (DLGPROC)MakeProcInstance((FARPROC)LanguageDlgProc, phInstance);
	language = DialogBoxParam(hlanguage, "LanguageDlgBox", hwndimg, lpProcLanguage, (LPARAM)NULL);
	FreeProcInstance((FARPROC)lpProcLanguage);
#endif
	switch (language) {
	    case IDM_LANGEN:
	    case IDM_LANGDE:
	    case IDM_LANGFR:
		gsview_language(language);
	}
    }
}

/* main initialisation */
BOOL
gsview_init1(LPSTR lpszCmdLine)
{
WNDCLASS wndclass;
#ifdef __WIN32__
DWORD version = GetVersion();
#endif
char *p;
int length = 64;

	while (length && !SetMessageQueue(length))
	    length--;	/* reduce size and try again */
	if (length == 0)
	    exit(0);	/* panic */
	
	/* figure out which version of Windows */
#ifdef __WIN32__
	/* Win32s: bit 15 HIWORD is 1 and bit 14 is 0 */
	/* Win95:  bit 15 HIWORD is 1 and bit 14 is 1 */
	/* WinNT:  bit 15 HIWORD is 0 and bit 14 is 0 */
	/* WinNT with Win95 shell recognised by WinNT + LOBYTE(LOWORD) >= 4 */
	/* check if Windows NT */
	if ((HIWORD(version) & 0x8000)==0)
	    is_winnt = TRUE;
	/* check if Windows 95 (Windows 4.0) */
	if ( ((HIWORD(version) & 0x8000)!=0) && ((HIWORD(version) & 0x4000)!=0) )
	    is_win95 = TRUE;
	/* Win32s */
	if ( ((HIWORD(version) & 0x8000)!=0) && ((HIWORD(version) & 0x4000)==0) )
	    is_win32s = TRUE;
	/* Windows 4.0 */
	if (LOBYTE(LOWORD(version)) >= 4)
	    is_win4 = TRUE;
#endif

	multithread = FALSE;
#ifdef __WIN32__
	if (is_win32s) {
	    /* don't allow multiple copies under Win32s */
	    HWND hwnd = FindWindow(szClassName, NULL);
	    if (hwnd != (HWND)NULL) {
		gsview_init0(lpszCmdLine);
		return FALSE;
	    }
	}
	if (is_win95 || is_winnt) {
	    display.event = CreateEvent(NULL, TRUE, FALSE, NULL);
  	    if (display.event)
	        multithread = TRUE;
	    else
	        error_message("Failed to create display.event object");
	    if (multithread)
	        InitializeCriticalSection(&crit_sec);
	    if (multithread)
	        hmutex_ps = CreateMutex(NULL, FALSE, NULL);
	    if (hmutex_ps == NULL) {
	        error_message("Failed to create mutex");
	        multithread = FALSE;
	    }
	}
	else {
#ifndef __WIN32__   /* OLD Win32s method */
	   szSpoolPrefix = "";	/* no spooler in Win32s */
#endif
	}
#endif

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
#ifdef __WIN32__
	/* allow for user profiles */
	if (is_win4) {
	    LONG rc;
	    HKEY hkey;
	    DWORD keytype;
	    DWORD cbData;
	    /* Find the user profile directory */
	    rc = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\ProfileReconciliation", 0, KEY_READ, &hkey);
	    if (rc == ERROR_SUCCESS) {
		cbData = sizeof(szIniFile)-sizeof(INIFILE);
		keytype =  REG_SZ;
		rc = RegQueryValueEx(hkey, "ProfileDirectory", 0, &keytype, (LPBYTE)szIniFile, &cbData);
		RegCloseKey(hkey);
	    }
	    if (rc == ERROR_SUCCESS)
		strcat(szIniFile, "\\");
	    else {
		    /* If we didn't succeed, use the Windows directory */
		    szIniFile[0] = '\0';
	    }
	}
	if (szIniFile[0] == '\0') {
	    /* If we didn't succeed, try %USERPROFILE% */
	    char *p = getenv("USERPROFILE");
	    DWORD fa;
	    if (p && *p) {
		strcpy(szIniFile, p);
		p = szIniFile + strlen(szIniFile) - 1;
		if ((*p == '\\') || (*p == '/'))
		    *p = '\0';
		/* check if USERPROFILE contains a directory name */
		fa = GetFileAttributes(szIniFile);
		if (fa & FILE_ATTRIBUTE_DIRECTORY)
		    strcat(szIniFile, "\\");
		else
		    szIniFile[0] = '\0';
	    }
	}
#else
	{
	    char *p = getenv("USERPROFILE");
	    DIR *d;
	    if (p && *p) {
		strcpy(szIniFile, p); 
		p = szIniFile + strlen(szIniFile) - 1;
		if ((*p == '\\') || (*p == '/'))
		    *p = '\0';
		/* check if USERPROFILE contains a directory name */
		d = opendir(szIniFile);
		if (d) {
		    closedir(d);
		    strcat(szIniFile, "\\");
		}
		else {
		    /* If we didn't succeed, use the Windows directory */
		    szIniFile[0] = '\0';
		}
	    }
	}
#endif
	strcat(szIniFile, INIFILE);

	getcwd(workdir, sizeof(workdir));
	/* defaults if entry not in gsview.ini */
 	init_options();
	/* read entries from gsview.ini */
	read_profile(szIniFile);

	if (!load_language(option.language)) {
	    message_box("Couldn't load language specific resources.  Resetting to English.", 0);
	    option.language = IDM_LANGEN;
	    if (!load_language(option.language))
		message_box("Couldn't load English resources.  Please reinstall GSview", 0);
	}

	/* register the child image window class */
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndImgChildProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = sizeof(LONG);
	wndclass.hInstance = phInstance;
	wndclass.hIcon = LoadIcon(phInstance,MAKEINTRESOURCE(ID_GSVIEW));
/*
	wndclass.hCursor = LoadCursor((HINSTANCE)NULL, IDC_CROSS);
*/
	wndclass.hCursor = hcCrossHair = LoadCursor(phInstance,MAKEINTRESOURCE(IDP_CROSSHAIR)); 
	wndclass.hbrBackground =  GetStockObject(LTGRAY_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szImgClassName;
	RegisterClass(&wndclass);
	
	/* register the parent window class */
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndImgProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = sizeof(LONG);
	wndclass.hInstance = phInstance;
	wndclass.hIcon = LoadIcon(phInstance,MAKEINTRESOURCE(ID_GSVIEW));
	wndclass.hCursor = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
	wndclass.hbrBackground =  GetStockObject(LTGRAY_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szClassName;
	RegisterClass(&wndclass);

	/* create parent window */
	hwndimg = CreateWindow(szClassName, (LPSTR)szAppName,
		  WS_OVERLAPPEDWINDOW,
		  option.img_origin.x, option.img_origin.y, 
		  option.img_size.x, option.img_size.y, 
		  NULL, NULL, phInstance, (void FAR *)NULL);

	/* help message for GetOpenFileName Dialog Box */
	help_message = RegisterWindowMessage(HELPMSGSTRING);

	change_language();
        load_string(IDS_WAIT, szWait, sizeof(szWait));	/* generic wait message */

	/* load DLL for sounds */
	/* MMSYSTEM.DLL requires Windows 3.1, so to allow gsview to run
	   under Windows 3.0 we can't use the import library */
#ifdef __WIN32__
	hlib_mmsystem = LoadLibrary("WINMM.DLL");
	if (hlib_mmsystem != NULL) {
	    lpfnSndPlaySound = (FPSPS)GetProcAddress(hlib_mmsystem, "sndPlaySoundA");
	}
#else
	hlib_mmsystem = LoadLibrary("MMSYSTEM.DLL");
	if (hlib_mmsystem >= HINSTANCE_ERROR) {
	    lpfnSndPlaySound = (FPSPS)GetProcAddress(hlib_mmsystem, "sndPlaySound");
	}
#endif
	else {
	    gserror(IDS_SOUNDNOMM, NULL, MB_ICONEXCLAMATION, -1);
	    hlib_mmsystem = (HINSTANCE)NULL;
	}


#if __BORLANDC__ == 0x452
	/* avoid bug in BC++ 4.0 */
#ifdef __WIN32__
	/* skip over EXE name */
	while ( *lpszCmdLine && (*lpszCmdLine!=' ')) 
		lpszCmdLine++;
	while ( *lpszCmdLine && (*lpszCmdLine==' ')) 
		lpszCmdLine++;
#endif
#endif

#ifdef __WIN32__
	if (is_win32s)
#endif
	    multithread = FALSE;	/* Win32s doesn't support multithreading */

	gsview_initc(lpszCmdLine);

	if (!parse_args(lpszCmdLine))
	    gserror(IDS_PARSEERROR, NULL, 0, SOUND_ERROR);
	return TRUE;
}

BOOL
parse_args(LPSTR str)
{
HGLOBAL hglobal;
LPSTR szFile;
LPSTR p;
BOOL found_command = FALSE;
LPSTR filename = NULL;
BOOL error = FALSE;
char filedir[MAXSTR];

    hglobal = GlobalAlloc(GHND | GMEM_SHARE, lstrlen(str)+1);
    if (hglobal)
	szFile = GlobalLock(hglobal);
    else
	return FALSE;

    p = szFile;
    while (!error && *str) {
	if ((*str == '/') || (*str == '-')) {
	    /* a command line switch */
	    if ((str[1] == 'D') || (str[1] == 'd')) {
	        debug = TRUE;
		str+=2;
	    }
	    else if ((str[1] == 'T') || (str[1] == 't')) {
		/* multithread option */
		if (str[2] == '\0')
		    multithread = !multithread;
		else if (str[2] == ' ')
		    multithread = !multithread;
		else if (str[2] == '0')
		    multithread = FALSE;
		else 
		    multithread = TRUE;
		/* skip over trailing garbage */
		while (*str && (*str != ' '))
		    str++;
	    }
	    else if ((str[1] == 'P') || (str[1] == 'p')) {
		if (found_command) {
		    error = TRUE;
	            gserror(IDS_DUPOPT, str, MB_ICONEXCLAMATION, SOUND_ERROR);
		}
		/* copy to output */
		while (*str && (*str != ' ')) {
		    *p++ = *str++;
		}
		*p++ = ' ';	/* add trailing space */
		found_command = TRUE;
	    }
	    else if ((str[1] == 'F') || (str[1] == 'f')) {
		if (found_command) {
		    error = TRUE;
	            gserror(IDS_DUPOPT, str, MB_ICONEXCLAMATION, SOUND_ERROR);
		}
		/* copy to output */
		while (*str && (*str != ' ')) {
		    *p++ = *str++;
		}
		*p++ = ' ';	/* add trailing space */
		found_command = TRUE;
	    }
	    else if ((str[1] == 'S') || (str[1] == 's')) {
		if (found_command) {
		    error = TRUE;
	            gserror(IDS_DUPOPT, str, MB_ICONEXCLAMATION, SOUND_ERROR);
		}
		/* copy to output */
		while (*str && (*str != ' ')) {
		    *p++ = *str++;
		}
		*p++ = ' ';	/* add trailing space */
		found_command = TRUE;
	    }
	    else {
	        gserror(IDS_BADCLI, str, MB_ICONEXCLAMATION, SOUND_ERROR);
		error = TRUE;
	    }

	    /* skip over trailing spaces */
	    while (*str && (*str == ' '))
		str++;
	}
	else {
	    /* a filename */
	    if (*str == '\042')
		str++; 		/* don't copy quotes */
	    filename = p;	/* save for extracting directory */
	    while (*str) {
		if (*str == '\042')
		    str++; 	/* don't copy quotes */
		else
		    *p++ = *str++;
	    }
	}
    }
    *p = '\0';

    /* use path to filename as current directory if specified */
    if (filename) {
	lstrcpy(filedir, filename);
	if ( (p = strrchr(filedir, '\\')) == (char *)NULL ) {
	    if ( (p = strrchr(filedir, ':')) == (char *)NULL )
		strcpy(filedir, workdir);  /* no path so use work directory */
	    else
		*(++p) = '\0';
	}
	else
	    *(++p) = '\0';
#ifndef _MSC_VER
	if (isalpha(filedir[0]) && (filedir[1]==':'))
	    (void) setdisk(toupper(filedir[0])-'A');
	if (!((strlen(filedir)==2) && isalpha(filedir[0]) && (filedir[1]==':')))
#endif
	    gs_chdir(filedir);
    }

    /* if a print option or filename was specified, pass it on */
    if (!error && lstrlen(szFile)) {
	GlobalUnlock(hglobal);
	PostMessage(hwndimg, WM_COMMAND, IDM_DROP, (LPARAM)hglobal);
    }
    else {
	GlobalUnlock(hglobal);
	GlobalFree(hglobal);
    }

    if (debug) {
	gs_addmess("INI file is \042");
	gs_addmess(szIniFile);
	gs_addmess("\042\n");
    }
    return !error;
}


/* create gsview window menu bar, buttons and child window */
void
gsview_create()
{
int i;
WNDCLASS wndclass;
HGLOBAL hglobal;
short FAR *pButtonID;
TEXTMETRIC tm;
HDC hdc;
HWND hbutton;
WNDPROC	lpfnMenuButtonProc;
POINT char_size;		/* size of default text characters */
POINT button_size, button_shift;
LOGFONT lf;
HFONT old_hfont;
RECT rect;

	/* setup OPENFILENAME struct */
	ofn.lpstrFilter = (LPSTR)NULL;
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwndimg;
	ofn.nFilterIndex = FILTER_PS;
	ofn.lpstrFile = szOFilename;
	ofn.nMaxFile = sizeof(szOFilename);
	ofn.lpstrFileTitle = (LPSTR)NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrTitle = (LPSTR)NULL;
	ofn.lpstrInitialDir = (LPSTR)NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_SHOWHELP;
	load_string(IDS_TOPICROOT, szHelpTopic, sizeof(szHelpTopic));

	/* get default text size */
	hdc = GetDC(hwndimg);
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = 8;  /* 8 pts */
	strcpy(lf.lfFaceName, "Helv");
	info_font = CreateFontIndirect(&lf);
	old_hfont = SelectObject(hdc, info_font);
	GetTextMetrics(hdc,(LPTEXTMETRIC)&tm);
	display.planes = GetDeviceCaps(hdc, PLANES);
	display.bitcount = GetDeviceCaps(hdc, BITSPIXEL);
	SelectObject(hdc, old_hfont);
	ReleaseDC(hwndimg,hdc);
	char_size.x = tm.tmAveCharWidth;
	char_size.y = tm.tmHeight;

	/* set size of info area, buttons and offset to child window */
	info_rect.left = 0;
	info_rect.right = info_rect.left + 64 * char_size.x;
	info_rect.top = 0;
	info_rect.bottom = char_size.y+4;
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
	info_file.y = 2;
	info_coord.left = info_rect.left + 20 * char_size.x;
	info_coord.right = info_rect.left + 34 * char_size.x;
	info_coord.top = 2;
	info_coord.bottom = char_size.y+4;
	info_page.x = info_rect.left + 36 * char_size.x + 2;
	info_page.y = 2;

	hcWait = LoadCursor((HINSTANCE)NULL, IDC_WAIT);
	hcHand = LoadCursor(phInstance,MAKEINTRESOURCE(IDP_HAND)); 

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

	/* create child window */
	GetClientRect(hwndimg, &rect);
	hwndimgchild = CreateWindow(szImgClassName, (LPSTR)szAppName,
		  WS_CHILD /* | WS_VISIBLE */,
		  rect.left, rect.top,
		  rect.right-rect.left, rect.bottom-rect.top,
		  hwndimg, NULL, phInstance, (void FAR *)NULL);

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
/* what is this for????? */
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

LONG
create_registry_type(char *keyname, char *description)
{
HKEY hkey;
HKEY hsubkey;
LONG rc;
char buf[MAXSTR];

LONG lrc;
LONG lold;
BOOL oldkey = FALSE;
char oldvalue[MAXSTR];
char oldopen[MAXSTR];
char oldprint[MAXSTR];
char oldicon[MAXSTR];

const char opensubkey[] = "shell\\open";
const char printsubkey[] = "shell\\print";
const char commandsubkey[] = "command";

    /* Save existing key for uninstall */
    oldvalue[0] = oldopen[0] = oldprint[0] = oldicon[0] = '\0';
    lrc = RegOpenKey(HKEY_CLASSES_ROOT, keyname, &hkey);
/*
    lrc = RegOpenKeyEx(HKEY_CLASSES_ROOT, keyname, 0, KEY_ALL_ACCESS, &hkey);
*/
    if (lrc == ERROR_SUCCESS) {
	oldkey = TRUE;
        lold = sizeof(oldvalue);
	RegQueryValue(HKEY_CLASSES_ROOT, keyname, oldvalue, &lold);

	lrc = RegOpenKey(hkey, opensubkey, &hsubkey);
	if (lrc == ERROR_SUCCESS) {
	    lold = sizeof(oldopen);
	    RegQueryValue(hsubkey, commandsubkey, oldopen, &lold);
	    RegCloseKey(hsubkey);
	}

	lrc = RegOpenKey(hkey, printsubkey, &hsubkey);
	if (lrc == ERROR_SUCCESS) {
	    lold = sizeof(oldprint);
	    RegQueryValue(hsubkey, commandsubkey, oldprint, &lold);
	    RegCloseKey(hsubkey);
	}

	if (is_win4) {
	    lold = sizeof(oldicon);
	    RegQueryValue(hkey, "DefaultIcon", oldicon, &lold);
	}
	RegCloseKey(hkey);
    }

    /* Write new information */
    if (logfile != (FILE *)NULL)
	fprintf(logfile, "OpenKey=%s\n", keyname);
    rc = RegCreateKey(HKEY_CLASSES_ROOT, keyname, &hkey);
    if (rc != ERROR_SUCCESS)
	return rc;

    if (logfile != (FILE *)NULL)
	fprintf(logfile, "DeleteValue=\n");
    rc = RegSetValue(hkey, NULL, REG_SZ, description, strlen(description));

    if (logfile != (FILE *)NULL)
	fprintf(logfile, "OpenSubKey=%s\n", opensubkey);
    if (rc == ERROR_SUCCESS)
	rc = RegCreateKey(hkey, opensubkey, &hsubkey);
    sprintf(buf, "%s%s %%1", szExePath, GSVIEW_EXENAME);
    if (rc == ERROR_SUCCESS)
	rc = RegSetValue(hsubkey, commandsubkey, REG_SZ, buf, strlen(buf));
    if (logfile != (FILE *)NULL)
	fprintf(logfile, "DeleteKey=%s\n", commandsubkey);
    RegCloseKey(hsubkey);
    if (logfile != (FILE *)NULL)
	fprintf(logfile, "CloseSubKey=\n");
    /* fprintf(logfile, "DeleteSubKey=%s\n", opensubkey); */
    

    if (logfile != (FILE *)NULL)
	fprintf(logfile, "OpenSubKey=%s\n", printsubkey);
    if (rc == ERROR_SUCCESS)
	rc = RegCreateKey(hkey, "shell\\print", &hsubkey);
    sprintf(buf, "%s%s /p %%1", szExePath, GSVIEW_EXENAME);
    if (rc == ERROR_SUCCESS)
	rc = RegSetValue(hsubkey, commandsubkey, REG_SZ, buf, strlen(buf));
    if (logfile != (FILE *)NULL)
	fprintf(logfile, "DeleteKey=%s\n", commandsubkey);
    RegCloseKey(hsubkey);
    if (logfile != (FILE *)NULL)
	fprintf(logfile, "CloseSubKey=\n");
    /* fprintf(logfile, "DeleteSubKey=%s\n", printsubkey); */

    if (is_win4) {
	/* icon offset 3 is ID_GSVIEW_DOC */
	sprintf(buf, "%s%s,3", szExePath, GSVIEW_EXENAME);
	if (logfile != (FILE *)NULL)
	    fprintf(logfile, "DeleteKey=%s\n", "DefaultIcon");
	if (rc == ERROR_SUCCESS)
	    rc = RegSetValue(hkey, "DefaultIcon", REG_SZ, buf, strlen(buf));
    }

    RegCloseKey(hkey);
    if (logfile != (FILE *)NULL) {
	fprintf(logfile, "CloseKey=\n");
	if (!oldkey)
	    fprintf(logfile, "DeleteKey=%s\n", keyname);

	/* Restore previous values */
	if (oldkey) {
	    if (logfile != (FILE *)NULL)
		fprintf(logfile, "CreateKey=%s\n", keyname);
	    if (oldvalue[0])
		fprintf(logfile, "SetValue=,%s\n", oldvalue);
	    if (oldopen[0]) {
		fprintf(logfile, "CreateSubKey=%s\\%s\n", opensubkey, commandsubkey);
		fprintf(logfile, "SetValue=,%s\n", oldopen);
		fprintf(logfile, "CloseSubKey=\n");
	    }
	    if (oldprint[0]) {
		fprintf(logfile, "CreateSubKey=%s\\%s\n", printsubkey, commandsubkey);
		fprintf(logfile, "SetValue=,%s\n", oldprint);
		fprintf(logfile, "CloseSubKey=\n");
	    }
	    if (is_win4 && oldicon[0]) {
		fprintf(logfile, "SetValue=DefaultIcon,%s\n", oldicon);
	    }
	    fprintf(logfile, "CloseKey=\n");
	}
    }

    return rc;
}

int
update_registry(BOOL ps, BOOL pdf)
{
#ifdef __WIN32__
char *psmime="application/postscript";
char *pdfmime="application/pdf";
char *contentname="Content Type";
char *extension="Extension";
char buf[MAXSTR];
HKEY hkey;
#endif
char *pskey="psfile";
char *pdfkey="pdffile";
char *psext=".ps";
char *epsext=".eps";
char *pdfext=".pdf";
LONG rc = ERROR_SUCCESS;
char old[MAXSTR];
LONG lold;
LONG lrc = !ERROR_SUCCESS;

    if (!ps && !pdf)
	return 0;

    if (logfile != (FILE *)NULL)
	fprintf(logfile, "\n[Registry]\n");
    if (ps) {

        if (rc == ERROR_SUCCESS) {
	    lold = sizeof(old);
	    lrc = RegQueryValue(HKEY_CLASSES_ROOT, psext, old, &lold);
	}
        if (rc == ERROR_SUCCESS)
	    rc = RegSetValue(HKEY_CLASSES_ROOT, psext, REG_SZ, pskey, strlen(pskey));
	if (logfile != (FILE *)NULL)
	    fprintf(logfile, "OpenKey=%s\nDeleteValue=\nCloseKey=\n", psext);
	if ((lrc == ERROR_SUCCESS) && (old[0] !='\0')) {
	    if (logfile != (FILE *)NULL)
	        fprintf(logfile, "SetValue=%s,%s\n", psext, old);
	}

        if (rc == ERROR_SUCCESS) {
	    lold = sizeof(old);
	    lrc = RegQueryValue(HKEY_CLASSES_ROOT, epsext, old, &lold);
	}
	if (rc == ERROR_SUCCESS)
	    rc = RegSetValue(HKEY_CLASSES_ROOT, epsext, REG_SZ, pskey, strlen(pskey));
	if (logfile != (FILE *)NULL)
	    fprintf(logfile, "OpenKey=%s\nDeleteValue=\nCloseKey=\n", epsext);
	if ((lrc == ERROR_SUCCESS) && (old[0] !='\0')) {
	    if (logfile != (FILE *)NULL)
		fprintf(logfile, "SetValue=%s,%s\n", epsext, old);
	}

#ifdef __WIN32__
	/* Don't bother with undelete information for these */
	if (!is_win32s) {
	    sprintf(buf, "MIME\\Database\\%s\\%s", contentname, psmime);
	    if (rc == ERROR_SUCCESS) {
		rc = RegCreateKey(HKEY_CLASSES_ROOT, buf, &hkey);
		if (rc == ERROR_SUCCESS) {
		    rc = RegSetValueEx(hkey, extension, 0, REG_SZ, (CONST BYTE *)psext, strlen(psext)+1);
		    RegCloseKey(hkey);
		}
	    }

	    if (rc == ERROR_SUCCESS) {
		rc = RegOpenKeyEx(HKEY_CLASSES_ROOT, psext, 0, KEY_SET_VALUE, &hkey);
		if (rc == ERROR_SUCCESS) {
		    rc = RegSetValueEx(hkey, contentname, 0, REG_SZ, (CONST BYTE *)psmime, strlen(psmime)+1);
		    RegCloseKey(hkey);
		}
	    }
	    if (rc == ERROR_SUCCESS) {
		rc = RegOpenKeyEx(HKEY_CLASSES_ROOT, epsext, 0, KEY_SET_VALUE, &hkey);
		if (rc == ERROR_SUCCESS) {
		    rc = RegSetValueEx(hkey, contentname, 0, REG_SZ, (CONST BYTE *)psmime, strlen(psmime)+1);
		    RegCloseKey(hkey);
		}
	    }
	}
#endif
	if (rc == ERROR_SUCCESS)
	  rc = create_registry_type(pskey, "PostScript");
    }

    if (pdf) {

        if (rc == ERROR_SUCCESS) {
	    lold = sizeof(old);
	    lrc = RegQueryValue(HKEY_CLASSES_ROOT, pdfext, old, &lold);
	}
	if (rc == ERROR_SUCCESS)
	    rc = RegSetValue(HKEY_CLASSES_ROOT, pdfext, REG_SZ, pdfkey, strlen(pdfkey));
	if (logfile != (FILE *)NULL)
	    fprintf(logfile, "OpenKey=%s\nDeleteValue=\nCloseKey=\n", pdfext);
	if ((lrc == ERROR_SUCCESS) && (old[0] !='\0')) {
	    if (logfile != (FILE *)NULL)
		fprintf(logfile, "SetValue=%s,%s\n", pdfext, old);
	}

#ifdef __WIN32__
	/* Don't bother with undelete information for these */
	if (!is_win32s) {
	    sprintf(buf, "MIME\\Database\\%s\\%s", contentname, pdfmime);
	    if (rc == ERROR_SUCCESS) {
		rc = RegCreateKey(HKEY_CLASSES_ROOT, buf, &hkey);
		if (rc == ERROR_SUCCESS) {
		    rc = RegSetValueEx(hkey, extension, 0, REG_SZ, (CONST BYTE *)pdfext, strlen(pdfext)+1);
		    RegCloseKey(hkey);
		}
	    }
	    if (rc == ERROR_SUCCESS) {
		rc = RegOpenKeyEx(HKEY_CLASSES_ROOT, ".pdf", 0, KEY_SET_VALUE, &hkey);
		if (rc == ERROR_SUCCESS) {
		    rc = RegSetValueEx(hkey, contentname, 0, REG_SZ, (CONST BYTE *)pdfmime, strlen(pdfmime)+1);
		    RegCloseKey(hkey);
		}
	    }
	}
#endif
	if (rc == ERROR_SUCCESS)
	  rc = create_registry_type(pdfkey, "Portable Document Format");
    }

    if (rc != ERROR_SUCCESS)
	return 1;

    return 0;
}


#ifdef __BORLANDC__
#pragma argsused	/* ignore warning for next function */
#endif
HDDEDATA CALLBACK 
DdeCallback(UINT type, UINT fmt, HCONV hconv,
    HSZ hsz1, HSZ hsz2, HDDEDATA hData, DWORD dwData1, DWORD dwData2)
{
  switch (type) {
    default:
	return (HDDEDATA)NULL;
  }
}

int
gsview_create_objects(char *groupname)
{
DWORD idInst = 0L;
FARPROC lpDdeProc;
HSZ hszServName;
HSZ hszSysTopic;
HCONV hConv;
char setup[MAXSTR+MAXSTR];
DWORD dwResult;
char gspath[MAXSTR];
char *p;
#ifdef __WIN32__
#define GSVIEW_NAME "GSview"
#else
#define GSVIEW_NAME "GSview 16"
#endif
char groupfile[MAXSTR];
int i;
char *s, *d;


    /* derive group filename from group name */
    for (i=0, s=groupname, d=groupfile; i<8 && *s; s++) {
	if (isalpha(*s) || isdigit(*s)) {
	    *d++ = *s;
	    i++;
	} 
    }
    *d = '\0';
    if (strlen(groupfile)==0)
	strcpy(groupfile, "gstools");

    lpDdeProc = MakeProcInstance((FARPROC)DdeCallback, phInstance);
    if (DdeInitialize(&idInst, (PFNCALLBACK)lpDdeProc, CBF_FAIL_POKES, 0L)) {
#ifndef __WIN32__
	FreeProcInstance(lpDdeProc);
#endif
	return 1;
    }
    hszServName = DdeCreateStringHandle(idInst, "PROGMAN", CP_WINANSI);
    hszSysTopic = DdeCreateStringHandle(idInst, "PROGMAN", CP_WINANSI);
    hConv = DdeConnect(idInst, hszServName, hszSysTopic, (PCONVCONTEXT)NULL);
    if (hConv == NULL) {
	gserror(IDS_NOPROGMAN, NULL, 0, SOUND_ERROR);
	return 1;
    }

#define DDEEXECUTE(str)\
    DdeClientTransaction((LPBYTE)str, strlen(str)+1, hConv,\
	NULL, CF_TEXT, XTYP_EXECUTE, 2000, &dwResult)

    sprintf(setup, "[CreateGroup(\042%s\042,%s.grp)][ShowGroup(\042%s\042,1)]",
	groupname, groupfile, groupname);
    DDEEXECUTE(setup);
    sprintf(setup, "[ReplaceItem(\042%s\042)]", GSVIEW_NAME);
    DDEEXECUTE(setup);
    if (!is_win4)
       sprintf(setup, "[AddItem(\042%s%s\042,\042%s\042, \042%sgsview32.ico\042)]", 
	  szExePath, GSVIEW_EXENAME, GSVIEW_NAME, szExePath);
    else
       sprintf(setup, "[AddItem(\042%s%s\042,\042%s\042)]", 
	  szExePath, GSVIEW_EXENAME, GSVIEW_NAME);
    DDEEXECUTE(setup);

/* Win3.1 documentation says you must put quotes around names */
/* with embedded spaces. */
/* In Win95, it appears you must put quotes around the EXE name */
/* and options separately */

    sprintf(setup, "[ReplaceItem(\042GSview README\042)]");
    DDEEXECUTE(setup);
    if (!is_win4)
	sprintf(setup, "[AddItem(\042notepad.exe %sREADME.TXT\042,\042GSview README\042)]", 
	    szExePath);
    else
	sprintf(setup, "[AddItem(\042notepad.exe\042 \042%sREADME.TXT\042,\042GSview README\042,\042notepad.exe\042,1)]", 
	    szExePath);
    DDEEXECUTE(setup);

    strcpy(gspath, option.gsdll);
    if ((p = strrchr(gspath,'\\')) != (char *)NULL)
	p++;
    else
	p = gspath;
    *p = '\0';

    sprintf(setup, "[ReplaceItem(\042Ghostscript\042)]");
    DDEEXECUTE(setup);
    if (!is_win4)
        sprintf(setup, "[AddItem(\042%s%s -I%s\042,\042Ghostscript\042, \042%sgstext.ico\042)]", 
	    gspath, GS_EXENAME, option.gsinclude, gspath);
    else
        sprintf(setup, "[AddItem(\042%s%s\042 \042-I%s\042,\042Ghostscript\042)]", 
	    gspath, GS_EXENAME, option.gsinclude);
    DDEEXECUTE(setup);

    sprintf(setup, "[ReplaceItem(\042Ghostscript README\042)]");
    DDEEXECUTE(setup);
    if (!is_win4)
        sprintf(setup, "[AddItem(\042notepad.exe %sREADME.\042,\042Ghostscript README\042)]", 
	     gspath);
    else
        sprintf(setup, "[AddItem(\042notepad.exe\042 \042%sREADME.\042,\042Ghostscript README\042, \042notepad.exe\042,1)]", 
	     gspath);
    DDEEXECUTE(setup);
#undef DDEXECUTE

    DdeDisconnect(hConv);
    DdeUninitialize(idInst);

    return 0;
}

HINSTANCE zlib_hinstance;
PFN_gzopen gzopen;
PFN_gzread gzread;
PFN_gzclose gzclose;

void
unload_zlib(void)
{
    if (zlib_hinstance == (HINSTANCE)NULL)
	return;
    FreeLibrary(zlib_hinstance);
    zlib_hinstance = NULL;
    gzopen = NULL;
    gzread = NULL;
    gzclose = NULL;
}

/* load zlib DLL for gunzip */
BOOL
load_zlib(void)
{   
char buf[MAXSTR];
    if (zlib_hinstance != (HINSTANCE)NULL)
	return TRUE;	/* already loaded */

    strcpy(buf, szExePath);
#ifdef __WIN32__
    strcat(buf, "zlib32.dll");
#else
    strcat(buf, "zlib16.dll");
#endif
    zlib_hinstance = LoadLibrary(buf);
    if (zlib_hinstance >= (HINSTANCE)HINSTANCE_ERROR) {
        gzopen = (PFN_gzopen) GetProcAddress(zlib_hinstance, "gzopen");
	if (gzopen == NULL) {
	    unload_zlib();
	}
	else {
	    gzread = (PFN_gzread) GetProcAddress(zlib_hinstance, "gzread");
	    if (gzread == NULL) {
		unload_zlib();
	    }
	    else {
		gzclose = (PFN_gzclose) GetProcAddress(zlib_hinstance, "gzclose");
		if (gzclose == NULL) {
		    unload_zlib();
		}
	    }
	}
    }
    else
	zlib_hinstance = NULL;

    if (zlib_hinstance == NULL) {
	load_string(IDS_ZLIB_FAIL, buf, sizeof(buf));
	if (message_box(buf, MB_OKCANCEL) == IDOK) {
	    load_string(IDS_TOPICZLIB, szHelpTopic, sizeof(szHelpTopic));
	    get_help();
	}
	return FALSE;
    }
    
    return TRUE;
}

/***************************/
/* configure dialog wizard */

HWND hWiz = HWND_DESKTOP;
int gsver = GS_REVISION;

int wiz_exit(HWND hwnd);
int check_gsver(HWND hwnd);
int config_finish(HWND hwnd);
BOOL CALLBACK _export CfgChildDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK _export CfgMainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

/* hDlgModeless */
typedef struct tagWIZPAGE {
   int id;		/* resource ID */
   int prev;		/* resource id of previous page */
   int next;		/* resource id of next page */
   int (*func)(HWND);	/* function to run on exit from page */
   HWND hwnd;		/* window handle of dialog */
} WIZPAGE;

WIZPAGE pages[]={
	{IDD_CFG1, IDD_CFG1, IDD_CFG2, NULL, 0},
	{IDD_CFG2, IDD_CFG1, IDD_CFG3, check_gsver, 0},
	{IDD_CFG3, IDD_CFG2, IDD_CFG4, NULL, 0},
	{IDD_CFG4, IDD_CFG3, IDD_CFG5, NULL, 0},
	{IDD_CFG5, IDD_CFG4, IDD_CFG6, NULL, 0},
	{IDD_CFG6, IDD_CFG5, IDD_CFG7, config_finish, 0},
	{IDD_CFG7, IDD_CFG7, IDD_CFG7, wiz_exit, 0},
	{0, 0, 0, NULL, 0}
};

#ifdef __BORLANDC__
#pragma argsused
#endif
int wiz_exit(HWND hwnd)
{
    PostMessage(hWiz, WM_COMMAND, (WPARAM)IDOK, (LPARAM)0);
    return 0;
}

WIZPAGE *
find_page_from_id(int id)
{
WIZPAGE *page;
    for (page=pages; page->id; page++) {
	if (page->id == id)
	    return page;
    }
    return NULL;
}

void
goto_page(HWND hwnd, int id)
{
WIZPAGE *page;
HWND hbutton;
    page = find_page_from_id(id);
    if (page) {
	ShowWindow(hwnd, SW_HIDE);
	ShowWindow(page->hwnd, SW_SHOW);
	hDlgModeless = page->hwnd;
	if (IsWindowEnabled(GetDlgItem(page->hwnd, IDNEXT)) )
	    hbutton = GetDlgItem(page->hwnd, IDNEXT);
	else
	    hbutton = GetDlgItem(page->hwnd, IDCANCEL);
	SetFocus(hbutton);
	SendMessage(hbutton, BM_SETSTYLE, (WPARAM)BS_DEFPUSHBUTTON, TRUE);
	return;
    }
}

void
next_page(HWND hwnd)
{
WIZPAGE *page;
int id;
    for (page=pages; page->id; page++) {
	if (page->hwnd == hwnd) {
	    if (page->func) {
		/* need to execute a function before continuing */
		if ( (id = page->func(hwnd)) != 0) {
		    goto_page(hwnd, id);
		    return;
		}
	    }
	    goto_page(hwnd, page->next);
	    return;
	}
    }
}

void
prev_page(HWND hwnd)
{
WIZPAGE *page;
    for (page=pages; page->id; page++) {
	if (page->hwnd == hwnd) {
	    goto_page(hwnd, page->prev);
	}
    }
}

int
add_gsver(HWND hwnd, int offset)
{
char buf[MAXSTR];
int ver;
    GetDlgItemText(hwnd, IDC_CFG20, buf, sizeof(buf));
    if (strlen(buf) == 4)
	ver = (buf[0]-'0')*100 + (buf[2]-'0')*10 + (buf[3]-'0');
    else if (strlen(buf) == 3)
	ver = (buf[0]-'0')*100 + (buf[2]-'0')*10;
    else
	return GS_REVISION;
    ver += offset;
    if (ver > GS_REVISION_MAX)
       ver = GS_REVISION_MAX;
    if (ver < GS_REVISION_MIN)
       ver = GS_REVISION_MIN;
    return ver;
}

int
check_gsver(HWND hwnd)
{
char buf[MAXSTR];
int ver = GS_REVISION;
BOOL fixit = FALSE;
    /* should allow edit field to be changed  */
    /* then make sure it is within range */
    GetDlgItemText(hwnd, IDC_CFG20, buf, sizeof(buf));
    if (strlen(buf) == 4) {
	ver = (buf[0]-'0')*100 + (buf[2]-'0')*10 + (buf[3]-'0');
	if ( (ver > GS_REVISION_MAX) || (ver < GS_REVISION_MIN) )
	    fixit = TRUE;
    }
    else
	fixit = TRUE;
    if (fixit) {
	ver = GS_REVISION;
	sprintf(buf, "%d.%02d", ver / 100, ver % 100);
	SetDlgItemText(hwnd, IDC_CFG20, buf);
	/* don't move until it is valid */
	return IDD_CFG2;
    }
    
    gsver = ver;
    return 0;
}

/* update GS directory edit field when version number changes */
void
gsdir_fix(HWND hwnd, char *verstr)
{
char buf[MAXSTR];
char *p;
    GetDlgItemText(hwnd, IDC_CFG22, buf, sizeof(buf));
    if (strlen(buf) < 6)
	return;
    p = buf + strlen(buf) - 4;
    if (isdigit(p[0]) && (p[1]=='.') && isdigit(p[2]) && isdigit(p[3])) {
	strcpy(p, verstr);
        SetDlgItemText(hwnd, IDC_CFG22, buf);
    }
    else {
	p = buf + strlen(buf) - 3;
	if (isdigit(p[0]) && (p[1]=='.') && isdigit(p[2])) {
	    strcpy(p, verstr);
	    SetDlgItemText(hwnd, IDC_CFG22, buf);
	}
    }
}

int
config_now(void)
{
BOOL assoc_ps;
BOOL assoc_pdf;
char buf[MAXSTR];
WIZPAGE *page;
FILE *f;
char *p;
char logname[MAXSTR];

    /* get info from wizard */
    page = find_page_from_id(IDD_CFG2);
    option.gsversion = add_gsver(page->hwnd, 0);
    GetDlgItemText(page->hwnd, IDC_CFG22, buf, sizeof(buf));
    sprintf(option.gsdll, "%s\\%s", buf, GS_DLLNAME);
    sprintf(option.gsinclude, "%s;%s\\fonts", buf, buf);
    strcpy(option.gsother, "-dNOPLATFONTS ");
    GetDlgItemText(page->hwnd, IDC_CFG23, buf, sizeof(buf));
    if (strlen(buf)) {
	strcat(option.gsother, "-sFONTPATH=\042");
	strcat(option.gsother, buf);
	strcat(option.gsother, "\042");
    }

    /* check if Ghostscript really has been installed */
    /* first look for the DLL */
    if ( (f = fopen(option.gsdll, "rb")) == (FILE *)NULL ) {
	load_string(IDS_GSNOTINSTALLED, buf, sizeof(buf));
	SetDlgItemText(find_page_from_id(IDD_CFG7)->hwnd, IDC_CFG71,
	    buf);
	return 1;
    }
    fclose(f);

    /* next look for gs_init.ps */
    strcpy(buf, option.gsdll);
    p = strrchr(buf, '\\');	/* remove trailing DLLNAME */
    if (p)
	*(++p) = '\0';
    strcat(buf, "gs_init.ps");
    if ( (f = fopen(buf, "rb")) == (FILE *)NULL ) {
	load_string(IDS_GSLIBNOTINSTALLED, buf, sizeof(buf));
	SetDlgItemText(find_page_from_id(IDD_CFG7)->hwnd, IDC_CFG71,
	    buf);
	return 1;
    }
    fclose(f);
    /* at this stage we don't look for fonts, but maybe we should */
    
    strcpy(logname, szExePath);
    strcat(logname, GSVIEW_ZIP);
    p = strrchr(logname, '.');
    strcpy(p, ".log");
    logfile = fopen(logname, "a");	/* append */
    if (logfile == (FILE *)NULL) {
	logfile = fopen(logname, "w");	/* don't append */
        /* if (logfile == (FILE *)NULL) { */
	    /* We can't write the logfile, probably because the destination */
	    /* is read only.  Don't worry about this, just remember not */
	    /* to write to logfile! */
	/* } */
    }

    assoc_ps = (BOOL)SendDlgItemMessage(find_page_from_id(IDD_CFG4)->hwnd, 
	    IDC_CFG41, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
    assoc_pdf = (BOOL)SendDlgItemMessage(find_page_from_id(IDD_CFG4)->hwnd, 
	    IDC_CFG42, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
    if (update_registry(assoc_ps, assoc_pdf)) {
	if (logfile != (FILE *)NULL)
	    fclose(logfile);
	return 1;
    }

    GetDlgItemText(find_page_from_id(IDD_CFG5)->hwnd, 
	IDC_CFG52, buf, sizeof(buf));
    if (SendDlgItemMessage(find_page_from_id(IDD_CFG5)->hwnd, 
	    IDC_CFG51, BM_GETCHECK, (WPARAM)0, (LPARAM)0)
	&& gsview_create_objects(buf)) {
	if (logfile != (FILE *)NULL)
	    fclose(logfile);
	return 1;
    }
    
    if (SendDlgItemMessage(find_page_from_id(IDD_CFG3)->hwnd, 
	    IDC_CFG32, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
	gsview_printer_profiles();


    option.configured = TRUE;

    write_profile();

    if (logfile != (FILE *)NULL)
	fclose(logfile);

    return 0;
}


int
config_finish(HWND hwnd)
{
    EnableWindow(GetDlgItem(hwnd, IDNEXT), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDPREV), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDCANCEL), FALSE);
    if (config_now())
    {	char buf[MAXSTR];
	load_string(IDS_CFG73, buf, sizeof(buf));
	SetDlgItemText(find_page_from_id(IDD_CFG7)->hwnd, IDC_CFG70, buf);
    }
    return 0;
}


#ifndef __WIN32__
DLGPROC lpProcCfgMain;
DLGPROC lpProcCfgChild;
#endif

int
config_wizard(void)
{
    /* main dialog box */
    EnableWindow(hwndimg, FALSE);
    /* we must use modeless dialog box to get the correct dialog control */
    /* handling in the child windows */
#ifdef __WIN32__
    hWiz = CreateDialogParam(hlanguage, MAKEINTRESOURCE(IDD_CFG0), hwndimg, CfgMainDlgProc, (LPARAM)NULL);
#else
    if (!lpProcCfgMain)
        lpProcCfgMain = (DLGPROC)MakeProcInstance((FARPROC)CfgMainDlgProc, phInstance);
    if (!lpProcCfgChild)
        lpProcCfgChild = (DLGPROC)MakeProcInstance((FARPROC)CfgChildDlgProc, phInstance);
    hWiz = CreateDialogParam(hlanguage, MAKEINTRESOURCE(IDD_CFG0), hwndimg, lpProcCfgMain, (LPARAM)NULL);
    
/* We can't free these thunks until the dialog box has returned */
/* This happens elsewhere which makes it hard to delete them */
/* For the present, leave them lying around because we won't be */
/* using the config wizard often */
/* 
    FreeProcInstance((FARPROC)lpProcCfgMain);
    lpProcCfgMain = (DLGPROC)NULL;
    FreeProcInstance((FARPROC)lpProcCfgChild);
    lpProcCfgChild = (DLGPROC)NULL;
*/
#endif

    return 0; /* success */
}



#ifdef __BORLANDC__
#pragma argsused	/* ignore warning for next function */
#endif
/* Modeless Dialog Box */
BOOL CALLBACK _export
CfgMainDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
        case WM_INITDIALOG:
	    /* create child dialog windows */
	    {
		WIZPAGE *page;
		char buf[MAXSTR];
		char gsdir[MAXSTR];
		char *p;
		for (page=pages; page->id; page++) {
#ifdef __WIN32__
		    page->hwnd = CreateDialogParam(hlanguage, MAKEINTRESOURCE(page->id), hwnd, CfgChildDlgProc, (LPARAM)NULL);
#else
		    page->hwnd = CreateDialogParam(hlanguage, MAKEINTRESOURCE(page->id), hwnd, lpProcCfgChild, (LPARAM)NULL);
#endif
		    ShowWindow(page->hwnd, SW_HIDE);
		}
		ShowWindow(pages[0].hwnd, SW_SHOW);
		SetFocus(GetDlgItem(pages[0].hwnd, IDNEXT));
		SendDlgItemMessage(pages[0].hwnd, IDNEXT, BM_SETSTYLE, 
		    (WPARAM)BS_DEFPUSHBUTTON, TRUE);
		hDlgModeless = pages[0].hwnd;

		/* initialize GS version */
		page = find_page_from_id(IDD_CFG2);
		if (page) {
		    sprintf(buf, "%d.%02d", option.gsversion / 100, 
			option.gsversion % 100);
		    SetDlgItemText(page->hwnd, IDC_CFG20, buf);
		    SetDlgItemText(page->hwnd, IDC_CFG22, szExePath);
		    SetDlgItemText(page->hwnd, IDC_CFG23, "c:\\psfonts");
		}

		/* assume that GS is in the adjacent directory */
		if (option.gsversion % 100 == 0)
		    sprintf(buf, "%d.%01d", option.gsversion / 100, 
			option.gsversion % 100);
		else
		    sprintf(buf, "%d.%02d", option.gsversion / 100, 
			option.gsversion % 100);
		strcpy(gsdir, szExePath);
		p = strrchr(gsdir, '\\');	/* remove trailing \ */
		if (p)
		    *p = '\0';
		p = strrchr(gsdir, '\\');	/* remove trailing gsview */
		if (p)
		    *(++p) = '\0';
		strcat(gsdir, "gs");
		strcat(gsdir, buf);
		SetDlgItemText(page->hwnd, IDC_CFG22, gsdir);
		SetDlgItemText(page->hwnd, IDC_CFG23, "c:\\psfonts");

		SendDlgItemMessage(find_page_from_id(IDD_CFG3)->hwnd, IDC_CFG32, BM_SETCHECK, 
			    (WPARAM)1, (LPARAM)0);
		SendDlgItemMessage(find_page_from_id(IDD_CFG4)->hwnd, IDC_CFG41, BM_SETCHECK, 
			    (WPARAM)1, (LPARAM)0);
		SendDlgItemMessage(find_page_from_id(IDD_CFG4)->hwnd, IDC_CFG42, BM_SETCHECK, 
			    (WPARAM)0, (LPARAM)0);   /* PDF is NOT the default */

		/* program group */
		load_string(IDS_PROGMANGROUP4, buf, sizeof(buf));
		page = find_page_from_id(IDD_CFG5);
		if (page) {
		    SendDlgItemMessage(page->hwnd, IDC_CFG51, BM_SETCHECK, 
				(WPARAM)1, (LPARAM)0);
		    SetDlgItemText(page->hwnd, IDC_CFG52, buf);
		}

	    }
            return FALSE;	/* we decide the focus */
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case IDCANCEL:
                case IDOK:
		    EnableWindow(hwndimg, TRUE);
		    DestroyWindow(hwnd);
		    hDlgModeless = NULL;
		    /* should post message to main window to delete thunks */
                    return(TRUE);
                default:
                    return(FALSE);
            }
	case WM_CLOSE:
	    EnableWindow(hwndimg, TRUE);
	    DestroyWindow(hwnd);
	    hDlgModeless = NULL;
	    return TRUE;
    }
    return FALSE;
}


#ifdef __BORLANDC__
#pragma argsused	/* ignore warning for next function */
#endif
/* Modeless Dialog Box */
BOOL CALLBACK _export
CfgChildDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
        case WM_INITDIALOG:
            return( TRUE);
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
		case IDOK:
                case IDNEXT:
		    next_page(hwnd);
                    return(TRUE);
                case IDPREV:
		    SendDlgItemMessage(hwnd, IDPREV, BM_SETSTYLE, 
		        (WPARAM)0, TRUE);	/* remove default style */
		    prev_page(hwnd);
                    return(TRUE);
                case IDCANCEL:
		    {	char buf[MAXSTR];
			load_string(IDS_CFG74, buf, sizeof(buf));
			SetDlgItemText(find_page_from_id(IDD_CFG7)->hwnd, IDC_CFG70, buf);
			goto_page(hwnd, IDD_CFG7);
		    }
                    return(TRUE);
		case IDC_CFG20:
		    if (GetNotification(wParam,lParam) == EN_CHANGE)
		    { int ver;
		      char buf[16];
		      ver = add_gsver(hwnd, 0);
		      if (ver % 100 == 0)
			  sprintf(buf, "%d.%01d", ver / 100, ver % 100);
		      else
			  sprintf(buf, "%d.%02d", ver / 100, ver % 100);
		      /* don't use touch IDC_CFG20 - this would be recursive */
		      gsdir_fix(hwnd, buf);
		    }
                    return(TRUE);
                default:
                    return(FALSE);
            }
	case WM_VSCROLL:
	    { int ver;
	      char buf[16];
		switch(LOWORD(wParam)) {
		    case SB_LINEUP:
			ver = add_gsver(hwnd, 1);
			break;
		    case SB_LINEDOWN:
			ver = add_gsver(hwnd, -1);
			break;
		    default:
			ver = add_gsver(hwnd, 0);
			break;
		}
		sprintf(buf, "%d.%02d", ver / 100, ver % 100);
		SetDlgItemText(hwnd, IDC_CFG20, buf);
	    }
	    return TRUE;
	case WM_CLOSE:
	    PostMessage(GetParent(hwnd), WM_COMMAND, (WPARAM)IDCANCEL, (LPARAM)0);
	    return TRUE;
    }
    return FALSE;
}


