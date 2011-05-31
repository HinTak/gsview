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

BOOL parse_args(LPSTR str);

/* Don't start another instance - use previous instance */
void
gsview_init0(LPSTR lpszCmdLine)
{
	HWND hwnd = FindWindow(szClassName, szAppName);
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
    strcat(langdll, "gsvw32");
#else
    strcat(langdll, "gsvw16");
#endif
    switch (language) {
	case IDM_LANGDE:
	    strcat(langdll, "de");
	    break;
	default:
	    strcat(langdll, "en");
    }
    strcat(langdll, ".dll");
    hInstance = LoadLibrary(langdll);
    if (hInstance >= (HINSTANCE)HINSTANCE_ERROR) {
	if (hlanguage)
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

#pragma argsused
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
		gsview_language(language);
	}
    }
}

/* main initialisation */
void
gsview_init1(LPSTR lpszCmdLine)
{
WNDCLASS wndclass;
DWORD version = GetVersion();
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
	if (isalpha(filedir[0]) && (filedir[1]==':'))
	    (void) setdisk(toupper(filedir[0])-'A');
	if (!((strlen(filedir)==2) && isalpha(filedir[0]) && (filedir[1]==':')))
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
char gs_dir[MAXSTR];
char buf[MAXSTR];
char *p;

    /* assume that GS is in the adjacent directory */
    strcpy(gs_dir, szExePath);
    p = strrchr(gs_dir, '\\');	/* remove trailing \ */
    if (p)
	*p = '\0';
    p = strrchr(gs_dir, '\\');	/* remove trailing gsview */
    if (p)
	*(++p) = '\0';
    strcat(gs_dir, GS_BASEDIR);

    rc = RegCreateKey(HKEY_CLASSES_ROOT, keyname, &hkey);
    if (rc != ERROR_SUCCESS)
	return rc;

    rc = RegSetValue(hkey, NULL, REG_SZ, description, strlen(description));

    if (rc == ERROR_SUCCESS)
	rc = RegCreateKey(hkey, "shell\\open", &hsubkey);
    sprintf(buf, "%s%s %%1", szExePath, GSVIEW_EXENAME);
    if (rc == ERROR_SUCCESS)
	rc = RegSetValue(hsubkey, "command", REG_SZ, buf, strlen(buf));
    RegCloseKey(hsubkey);

    if (rc == ERROR_SUCCESS)
	rc = RegCreateKey(hkey, "shell\\print", &hsubkey);
    sprintf(buf, "%s%s /p %%1", szExePath, GSVIEW_EXENAME);
    if (rc == ERROR_SUCCESS)
	rc = RegSetValue(hsubkey, "command", REG_SZ, buf, strlen(buf));
    RegCloseKey(hsubkey);

    if (is_win4) {
	/* icon offset 3 is ID_GSVIEW_DOC */
	sprintf(buf, "%s%s,3", szExePath, GSVIEW_EXENAME);
	if (rc == ERROR_SUCCESS)
	    rc = RegSetValue(hkey, "DefaultIcon", REG_SZ, buf, strlen(buf));
    }

    RegCloseKey(hkey);

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

    if (!ps && !pdf)
	return 0;

    if (ps) {
        if (rc == ERROR_SUCCESS)
	    rc = RegSetValue(HKEY_CLASSES_ROOT, psext, REG_SZ, pskey, strlen(pskey));
	if (rc == ERROR_SUCCESS)
	    rc = RegSetValue(HKEY_CLASSES_ROOT, epsext, REG_SZ, pskey, strlen(pskey));
#ifdef __WIN32__
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
	if (rc == ERROR_SUCCESS)
	    rc = RegSetValue(HKEY_CLASSES_ROOT, pdfext, REG_SZ, pdfkey, strlen(pdfkey));
#ifdef __WIN32__
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


#pragma argsused	/* ignore warning for next function */
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
gsview_create_objects(void)
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
BOOL register_ps;
BOOL register_pdf;
char buf[MAXSTR];
#ifdef __WIN32__
#define GSVIEW_NAME "GSview"
#else
#define GSVIEW_NAME "GSview 16"
#endif

    load_string(IDS_ASSOCPS, buf, sizeof(buf)-1);
    register_ps = (message_box(buf, MB_YESNO) == IDYES);
    load_string(IDS_ASSOCPDF, buf, sizeof(buf)-1);
    register_pdf = (message_box(buf, MB_YESNO) == IDYES);
    if (update_registry(register_ps, register_pdf))
	gserror(IDS_REGERROR, NULL, 0, SOUND_ERROR);

    load_string(IDS_CREATEGROUP, buf, sizeof(buf)-1);
    if (message_box(buf, MB_YESNO) != IDYES)
	return 0;

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

    sprintf(setup, "[CreateGroup(\042GS Tools\042,gstools.grp)][ShowGroup(\042GS Tools\042,1)]");
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

