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

/* gvwinit.c */
/* Initialisation routines for Windows GSview */
#include "gvwin.h"

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
HGLOBAL hglobal_command_line;
BOOL use_existing = FALSE;	/* /E command line option */
BOOL exit_existing = FALSE;	/* /X command line option */
BOOL dde_exit = FALSE;		/* exit after sending DDE command */

BOOL parse_args(LPSTR str);

/* convert gs version integer to string */
/* buf must be 6 chars or longer */
void gsver_string(int ver, char *buf)
{
    /* make sure length including null never exceeds 6 */
    if (ver >= 9999)
	strcpy(buf, "0.0");
    sprintf(buf, "%d.%02d", ver / 100, ver % 100);
}

/* convert gs version string to integer */
int gsver_int(char *buf)
{
    int ver;
    if (strlen(buf) == 4)
	ver = (buf[0]-'0')*100 + (buf[2]-'0')*10 + (buf[3]-'0');
    else if (strlen(buf) == 3)
	ver = (buf[0]-'0')*100 + (buf[2]-'0')*10;
    else
	ver = GS_REVISION;
    return ver;
}

void
drop_filename(HWND hwnd, char *str)
{
    /* Send the message to the main window */
    if (lstrlen(str) != 0) {
	/* open file specified on command line */
	HGLOBAL hglobal;
	LPSTR szFile;
	hglobal = GlobalAlloc(GHND | GMEM_SHARE, lstrlen(str)+1);
	if (hglobal) {
	    szFile = (LPSTR)GlobalLock(hglobal);
	    lstrcpy(szFile, str);
	    GlobalUnlock(hglobal);
	    PostMessage(hwnd, WM_COMMAND, IDM_DROP, (LPARAM)hglobal);
	}
    }
}

/* Don't start another instance - use previous instance */
/* This is never used in Win95/NT */
void
gsview_init0(LPSTR lpszCmdLine)
{
	HWND hwnd = FindWindow(szClassName, NULL);
	BringWindowToTop(hwnd);
#ifdef UNUSED
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
#endif
	drop_filename(hwnd, lpszCmdLine);
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
	case IDM_LANGES:
	    strcat(langdll, "es");
	    break;
	case IDM_LANGFR:
	    strcat(langdll, "fr");
	    break;
	case IDM_LANGIT:
	    strcat(langdll, "it");
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
	if (strcmp(GSVIEW_DOT_VERSION, langdll) != 0)
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

    nHelpTopic = IDS_TOPICROOT;
    init_check_menu();
    InvalidateRect(hwndimg, (LPRECT)NULL, FALSE);
    if (hwnd_measure)
	PostMessage(hwnd_measure, WM_COMMAND, IDCANCEL, 0L);
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
                case IDM_LANGES:
                case IDM_LANGFR:
                case IDM_LANGIT:
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
      || ((option.language == IDM_LANGES) && strnicmp(winlang, "ES", 2))
      || ((option.language == IDM_LANGFR) && strnicmp(winlang, "FR", 2))
      || ((option.language == IDM_LANGIT) && stricmp(winlang, "ITA"))
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
	    case IDM_LANGES:
	    case IDM_LANGFR:
	    case IDM_LANGIT:
		gsview_language(language);
	}
    }
}

void
post_command_line(void)
{
    if (hglobal_command_line)
	PostMessage(hwndimg, WM_COMMAND, IDM_DROP, 
		(LPARAM)hglobal_command_line);
}

void
system_colours(void)
{
LOGBRUSH lb;
    if (hpen_btnshadow)
	DeleteBrush(hpen_btnshadow);
    hpen_btnshadow = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
    if (hpen_btnhighlight)
	DeleteBrush(hpen_btnhighlight);
    hpen_btnhighlight = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
    lb.lbStyle = BS_SOLID;
    lb.lbHatch = 0;
    lb.lbColor = GetSysColor(COLOR_WINDOW);
    if (lb.lbColor == RGB(255,255,255))	/* don't allow white background */
	lb.lbColor = GetSysColor(COLOR_MENU);
    if (lb.lbColor == RGB(255,255,255))	/* don't allow white background */
	lb.lbColor = GetSysColor(COLOR_APPWORKSPACE);
    if (lb.lbColor == RGB(255,255,255))	/* don't allow white background */
	lb.lbColor = RGB(192,192,192);
    if (hbrush_window)
	DeleteBrush(hbrush_window);
    hbrush_window = CreateBrushIndirect(&lb);
    lb.lbColor = GetSysColor(COLOR_MENU);
    if (hbrush_menu)
	DeleteBrush(hbrush_menu);
    hbrush_menu =  CreateBrushIndirect(&lb);

    /* change class background brush of main and image windows */
    SetClassLong(hwndimg, GCL_HBRBACKGROUND, (LONG)hbrush_window);
    SetClassLong(hwndimgchild, GCL_HBRBACKGROUND, (LONG)hbrush_window);
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
BOOL parse_correct;
	getcwd(workdir, sizeof(workdir));

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
	if (is_win95 && is_win4 && HIBYTE(LOWORD(version)) >= 10)
	    is_win98 = TRUE;
#endif

	multithread = FALSE;
	if (is_win95 || is_winnt)
	    multithread = TRUE;

	// It is essential that arguments are parsed before
	// malloc is called, otherwise both malloc and debug_malloc
	// are called which is fatal.
	parse_correct = parse_args(lpszCmdLine);

	if (exit_existing) {
	    dde_execute("[FileExit()]");
	    dde_exit = TRUE;
	}
	if (use_existing) {
	    LPSTR line = (LPSTR)GlobalLock(hglobal_command_line);
	    if (line && dde_execute_line(line))
		dde_exit = TRUE;
	    dde_execute("[ShowWindow(1)]");
	    use_existing = FALSE;
	}

	if (dde_exit) {
	    GlobalUnlock(hglobal_command_line);
	    GlobalFree(hglobal_command_line);
	    return FALSE;
	}

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
	    if (multithread) {
		display.event = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!display.event) {
		    multithread = FALSE;
		    error_message("Failed to create display.event object");
		}
		if (multithread)
		    InitializeCriticalSection(&crit_sec);
		if (multithread)
		    hmutex_ps = CreateMutex(NULL, FALSE, NULL);
		if (hmutex_ps == NULL) {
		    error_message("Failed to create mutex");
		    multithread = FALSE;
		}
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
	    DWORD fa;
	    /* Find the user profile directory */
	    rc = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\ProfileReconciliation", 0, KEY_READ, &hkey);
	    if (rc == ERROR_SUCCESS) {
		cbData = sizeof(szIniFile)-sizeof(INIFILE);
		keytype =  REG_SZ;
		rc = RegQueryValueEx(hkey, "ProfileDirectory", 0, &keytype, (LPBYTE)szIniFile, &cbData);
		RegCloseKey(hkey);
	    }
	    if (rc == ERROR_SUCCESS) {
		fa = GetFileAttributes(szIniFile);
		if ((fa != 0xffffffff) && (fa & FILE_ATTRIBUTE_DIRECTORY))
		    strcat(szIniFile, "\\");
		else
		    szIniFile[0] = '\0';
	    }
	    else {
		    /* If we didn't succeed, use the Windows directory */
		    szIniFile[0] = '\0';
	    }
	}
	if (szIniFile[0] == '\0') {
	    DWORD fa;
	    /* If we didn't succeed, try %USERPROFILE% */
	    char *p = getenv("USERPROFILE");
	    if (p && *p) {
		strcpy(szIniFile, p);
#ifdef __BORLANDC__
		OemToCharBuff(szIniFile, szIniFile, lstrlen(szIniFile));
#endif
		p = szIniFile + strlen(szIniFile) - 1;
		if ((*p == '\\') || (*p == '/'))
		    *p = '\0';
		/* check if USERPROFILE contains a directory name */
		fa = GetFileAttributes(szIniFile);
		if ((fa != 0xffffffff) && (fa & FILE_ATTRIBUTE_DIRECTORY))
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
#ifdef __BORLANDC__
		OemToAnsiBuff(szIniFile, szIniFile, lstrlen(szIniFile));
#endif
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

	/* defaults if entry not in gsview.ini */
 	init_options();
	/* read entries from gsview.ini */
	read_profile(szIniFile);

	if (debug) {
	    gs_addmess("INI file is \042");
	    gs_addmess(szIniFile);
	    gs_addmess("\042\n");
	}

	if (!load_language(option.language)) {
	    message_box("Couldn't load language specific resources.  Resetting to English.", 0);
	    option.language = IDM_LANGEN;
	    if (!load_language(option.language))
		message_box("Couldn't load English resources.  Please reinstall GSview", 0);
	}

	system_colours();

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
	wndclass.hbrBackground =  hbrush_window;
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
	wndclass.hbrBackground =  hbrush_window;
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


#ifdef UNUSED
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
#endif

#ifdef __WIN32__
	if (is_win32s)
#endif
	    multithread = FALSE;	/* Win32s doesn't support multithreading */

	gsview_initc(lpszCmdLine);

	if (!parse_correct)
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
BOOL error = FALSE;
char filedir[MAXSTR];

    hglobal_command_line = NULL;
    hglobal = GlobalAlloc(GHND | GMEM_SHARE, lstrlen(str)+lstrlen(workdir)+32);
    if (hglobal)
	szFile = (LPSTR)GlobalLock(hglobal);
    else
	return FALSE;

    p = szFile;
    while (!error && *str) {
	if ((*str == '/') || (*str == '-')) {
	    /* a command line switch */
	    if ((str[1] == 'D') || (str[1] == 'd')) {
		str+=2;
		debug = atoi(str);
		while (*str && (*str!=' '))
		    str++;
		if (!debug)
		    debug = 1;
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
	    else if ((str[1] == 'E') || (str[1] == 'e')) {
		/* skip over trailing garbage */
		use_existing = TRUE;
		while (*str && (*str != ' '))
		    str++;
	    }
	    else if ((str[1] == 'G') || (str[1] == 'g')) {
		/* test DDE - /G5 goes to page 5 */
		char buf[MAXSTR];
		sprintf(buf, "[GotoPage(%d)]", atoi(str+2));
		dde_execute(buf);
		while (*str && (*str != ' '))
		    str++;
		dde_exit = TRUE;
	    }
	    else if ((str[1] == 'X') || (str[1] == 'x')) {
		exit_existing = TRUE;
	
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
		print_silent = TRUE;
		print_exit = TRUE;
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
		print_silent = TRUE;
		print_exit = TRUE;
	    }
	    else if ((str[1] == 'S') || (str[1] == 's')) {
		if (found_command) {
		    error = TRUE;
	            gserror(IDS_DUPOPT, str, MB_ICONEXCLAMATION, SOUND_ERROR);
		}
		/* copy to output */
		*p++ = *str++;
		*p++ = *str++;
		if (*str == '\042') {
		    *p++ = *str++;
		    while (*str && (*str != '\042'))
			*p++ = *str++;
		    if (*str)
		        *p++ = *str++;
		}
		else {
		    while (*str && (*str != ' '))
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
	    /* a filename - we only allow one */
	    if (*str == '\042')
		str++; 		/* don't copy quotes */
	    /* we need the filename to include the path */
	    if (str[0] && (str[0]=='\\' || str[0]=='/')) {
		if (str[1] && (str[1]=='\\' ||  str[1]=='/')) {
		    /* UNC name */
		    /* do nothing */
		}
		else {
		    /* referenced from root directory, so add drive */
		    *p++ = workdir[0];
		    *p++ = workdir[1];
		}
	    }
	    else if (*str && ! (isalpha(*str) && str[1]==':')) {
		/* Doesn't include drive code, so add work dir */
		char *t = workdir;
		while (*t)
		   *p++ = *t++;
		t--;
		if (*t != '\\')
		   *p++ = '\\';
	    }
	    else {
		/* contains full path */
		/* make this the work dir */
		char *t;
		lstrcpy(filedir, str);
	        if ( (t = strrchr(filedir, '\\')) != (char *)NULL ) {
		    *(++t) = '\0';
#ifndef _MSC_VER
		    if (isalpha(filedir[0]) && (filedir[1]==':'))
			(void) setdisk(toupper(filedir[0])-'A');
		    if (!((strlen(filedir)==2) && isalpha(filedir[0]) && 
			(filedir[1]==':')))
#endif
			gs_chdir(filedir);
		}
	    }
	    while (*str) {
		if (*str == '\042')
		    str++; 	/* don't copy quotes */
		else
		    *p++ = *str++;
	    }
	}
    }
    *p = '\0';

    getcwd(filedir, sizeof(filedir));
    if (debug) {
	gs_addmess("Working directory: ");
	gs_addmess(filedir);
	gs_addmess("\n");
    }

    /* if a print option or filename was specified, pass it on */
    if (!error && lstrlen(szFile)) {
	if (debug) {
	    gs_addmess("Command line to be posted: ");
	    gs_addmess(szFile);
	    gs_addmess("\n");
	}
	GlobalUnlock(hglobal);
	hglobal_command_line = hglobal;
    }
    else {
	GlobalUnlock(hglobal);
	GlobalFree(hglobal);
    }

    return !error;
}


void
delete_buttons(void)
{
    struct buttonlist *bp = buttonhead;
    while (bp) {
	buttonhead = bp->next;
	free(bp);
	bp = buttonhead;
    }
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
	nHelpTopic = IDS_TOPICROOT;

	/* get default text size */
	hdc = GetDC(hwndimg);
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = 8;  /* 8 pts */
	strcpy(lf.lfFaceName, "Helv");
	info_font = CreateFontIndirect(&lf);
	old_hfont = (HFONT)SelectObject(hdc, info_font);
	GetTextMetrics(hdc,(LPTEXTMETRIC)&tm);
	display.planes = GetDeviceCaps(hdc, PLANES);
	display.bitcount = GetDeviceCaps(hdc, BITSPIXEL);
	SelectObject(hdc, old_hfont);
	ReleaseDC(hwndimg,hdc);
	char_size.x = tm.tmAveCharWidth;
	char_size.y = tm.tmHeight;

	/* set size of info area, buttons and offset to child window */
	info_rect.left = 0;
	info_rect.right = info_rect.left + 86 * char_size.x;
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
	info_coord.left = info_rect.left + 32 * char_size.x;
	info_coord.right = info_rect.left + 52 * char_size.x;
	info_coord.top = 2;
	info_coord.bottom = char_size.y+4;
	info_page.x = info_rect.left + 54 * char_size.x + 2;
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
	hwnd_image = hwndimgchild = 
		  CreateWindow(szImgClassName, (LPSTR)szAppName,
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


char hkey_root[]="HKEY_CLASSES_ROOT";
char reg_win32s_error[]="ERROR: You can't set named values under Win32s\n";

void
reg_quote(char *d, char *s)
{
    while (*s) {
	if (*s == '\\')
	    *d++ = '\\';
	*d++ = *s++;
    }
    *d = *s;
}

/* Open the key. If it doesn't exist, create it */
BOOL
reg_open_key(FILE *newfile, FILE *oldfile, char *name, HKEY *hkey)
{
LONG lrc; 
    lrc = RegOpenKey(HKEY_CLASSES_ROOT, name, hkey);
    if (lrc == ERROR_SUCCESS) {
	if (oldfile) 
	    fprintf(oldfile, "\n[%s\\%s]\n", hkey_root, name);
    }
    else {
	lrc = RegCreateKey(HKEY_CLASSES_ROOT, name, hkey);
    }
    if (newfile)
	fprintf(newfile, "\n[%s\\%s]\n", hkey_root, name);
    if (lrc != ERROR_SUCCESS)
        *hkey = HKEY_CLASSES_ROOT;
    return (lrc == ERROR_SUCCESS);
}


BOOL
reg_set_value(FILE *newfile, FILE *oldfile, HKEY hkey, char *name, char *value)
{
#ifdef __WIN32__
DWORD keytype;
DWORD cbData;
#endif
char buf[MAXSTR];
char qbuf[MAXSTR];
LONG lenbuf;
    if (hkey == HKEY_CLASSES_ROOT)
	return FALSE;
    if (oldfile) {
        lenbuf = sizeof(buf);
	if (name==(char *)NULL) {
	    if (RegQueryValue(hkey, (LPSTR)name, (LPSTR)buf, &lenbuf)
		== ERROR_SUCCESS) {
		if (strlen(buf)) {
		    reg_quote(qbuf, buf);
		    fprintf(oldfile, "@=\042%s\042\n", qbuf);
		}
	    }
	}
#ifdef __WIN32__
	else if (!is_win32s) {
	    cbData = sizeof(buf);
	    keytype =  REG_SZ;
	    if (RegQueryValueEx(hkey, name, 0, &keytype, 
		(LPBYTE)buf, &cbData) == ERROR_SUCCESS) {
	        reg_quote(qbuf, buf);
	        fprintf(oldfile, "\042%s\042=\042%s\042\n", name, qbuf);
	    }
	}
#endif
	else {
	    fprintf(oldfile, reg_win32s_error);
	    return FALSE;
	}
    }
    if (name==(char *)NULL) {
	reg_quote(qbuf, value);
	if (newfile)
	    fprintf(newfile, "@=\042%s\042\n", qbuf);
	if (RegSetValue(hkey, NULL, REG_SZ, 
	    value, strlen(value)) != ERROR_SUCCESS)
	    return FALSE;
    }
#ifdef __WIN32__
    else if (!is_win32s) {
	reg_quote(qbuf, value);
	if (newfile)
	    fprintf(newfile, "\042%s\042=\042%s\042\n", name, qbuf);
	if (RegSetValueEx(hkey, name, 0, REG_SZ, 
	    (CONST BYTE *)value, strlen(value)+1) != ERROR_SUCCESS)
	    return FALSE;
    }
#endif
    else {
	if (newfile)
	    fprintf(newfile, reg_win32s_error);
	return FALSE;
    }
    return TRUE; 
}

void
reg_close_key(HKEY *hkey)
{
    RegCloseKey(*hkey);
    *hkey = HKEY_CLASSES_ROOT;
}

BOOL
create_registry_type(FILE *newfile, FILE *oldfile, 
	char *keyname, char *description)
{
HKEY hkey;
char buf[MAXSTR];
char kbuf[MAXSTR];
const char shellsubkey[]= "\\shell";
const char opensubkey[] = "\\open";
const char printsubkey[] = "\\print";
const char commandsubkey[] = "\\command";
BOOL flag = TRUE;

    if (flag)
	flag = reg_open_key(newfile, oldfile, keyname, &hkey);
    if (flag) {
	flag = reg_set_value(newfile, oldfile, hkey, NULL, description);
	reg_close_key(&hkey);
    }

    strcpy(kbuf, keyname);
    strcat(kbuf, shellsubkey);
    if (flag)
	flag = reg_open_key(newfile, oldfile, kbuf, &hkey);
    if (flag)
	reg_close_key(&hkey);

    strcat(kbuf, opensubkey);
    if (flag)
	flag = reg_open_key(newfile, oldfile, kbuf, &hkey);
    if (flag)
	reg_close_key(&hkey);
    strcat(kbuf, commandsubkey);
    if (flag)
	flag = reg_open_key(newfile, oldfile, kbuf, &hkey);
#ifdef __WIN32__
    if (!is_win32s)
        sprintf(buf, "\042%s%s\042 \042%%1\042", szExePath, GSVIEW_EXENAME);
    else
#endif
        sprintf(buf, "%s%s %%1", szExePath, GSVIEW_EXENAME);
    if (flag) {
	flag = reg_set_value(newfile, oldfile, hkey, NULL, buf);
	reg_close_key(&hkey);
    }

    strcpy(kbuf, keyname);
    strcat(kbuf, shellsubkey);
    strcat(kbuf, printsubkey);
    if (flag)
	flag = reg_open_key(newfile, oldfile, kbuf, &hkey);
    if (flag)
	reg_close_key(&hkey);
    strcat(kbuf, commandsubkey);
    if (flag)
	flag = reg_open_key(newfile, oldfile, kbuf, &hkey);
#ifdef __WIN32__
    if (!is_win32s)
        sprintf(buf, "\042%s%s\042 /p \042%%1\042", szExePath, GSVIEW_EXENAME);
    else
#endif
        sprintf(buf, "%s%s /p %%1", szExePath, GSVIEW_EXENAME);
    if (flag) {
	flag = reg_set_value(newfile, oldfile, hkey, NULL, buf);
	reg_close_key(&hkey);
    }

    if (is_win4) {
	strcpy(kbuf, keyname);
	strcat(kbuf, "\\DefaultIcon");
	if (flag)
	    flag = reg_open_key(newfile, oldfile, kbuf, &hkey);
	sprintf(buf, "%s%s,3", szExePath, GSVIEW_EXENAME);
	if (flag) {
	    flag = reg_set_value(newfile, oldfile, hkey, NULL, buf);
	    reg_close_key(&hkey);
	}
    }

    return flag;
}


int
update_registry(BOOL ps, BOOL pdf)
{
#ifdef __WIN32__
char *psmime="application/postscript";
char *pdfmime="application/pdf";
char *contentname="Content Type";
char *extension="Extension";
#endif
char buf[MAXSTR];
HKEY hkey;
char *pskey="psfile";
char *pdfkey="pdffile";
char *psext=".ps";
char *epsext=".eps";
char *pdfext=".pdf";
char *p;
FILE *oldfile, *newfile;
BOOL flag = TRUE;
const char regheader[]="REGEDIT4\n";

    if (!ps && !pdf)
	return 0;

    strcpy(buf, szExePath);
    strcat(buf, GSVIEW_ZIP);
    p = strrchr(buf, '.');
    /* Write the old registry file, but only if it doesn't exist */
    strcpy(p, "old.reg");
    oldfile = fopen(buf, "r");
    if (oldfile == (FILE *)NULL) {
        oldfile = fopen(buf, "w");
	/* If we failed to open the file, the destination is probably 
	 * read only.  Don't worry, just don't write to the log file.
	 */
    }
    else {
	fclose(oldfile);
	oldfile = (FILE *)NULL;
    }

    /* Write the new registry file */
    strcpy(p, "new.reg");
    newfile = fopen(buf, "w");

    if (oldfile != (FILE *)NULL)
	fprintf(oldfile, regheader);
    if (newfile != (FILE *)NULL)
	fprintf(newfile, regheader);

    if (ps) {
	if (flag)
	    flag = reg_open_key(newfile, oldfile, psext, &hkey);
	if (flag) {
	    flag = reg_set_value(newfile, oldfile, hkey, NULL, pskey);
#ifdef __WIN32__
	    if (flag && !is_win32s)
		reg_set_value(newfile, oldfile, hkey, contentname, psmime);
#endif
	    reg_close_key(&hkey);
	}

	if (flag)
	    flag = reg_open_key(newfile, oldfile, epsext, &hkey);
	if (flag) {
	    flag = reg_set_value(newfile, oldfile, hkey, NULL, pskey);
#ifdef __WIN32__
	    if (flag && !is_win32s)
		flag = reg_set_value(newfile, oldfile, hkey, 
		    contentname, psmime);
#endif
	    reg_close_key(&hkey);
	}


#ifdef __WIN32__
	/* Don't bother with undelete information for these */
	if (!is_win32s) {
	    sprintf(buf, "MIME\\Database\\%s\\%s", contentname, psmime);
	    if (flag)
		flag = reg_open_key(newfile, oldfile, buf, &hkey);
	    if (flag) {
		flag = reg_set_value(newfile, oldfile, hkey, extension, psext);
		reg_close_key(&hkey);
	    }
	}
#endif
	if (flag) 
	  flag = create_registry_type(newfile, oldfile, pskey, "PostScript");
    }

    if (pdf) {
	if (flag)
	    flag = reg_open_key(newfile, oldfile, pdfext, &hkey);
	if (flag) {
	    flag = reg_set_value(newfile, oldfile, hkey, NULL, pdfkey);
#ifdef __WIN32__
	    if (flag && !is_win32s)
		reg_set_value(newfile, oldfile, hkey, contentname, pdfmime);
#endif
	    reg_close_key(&hkey);
	}

#ifdef __WIN32__
	/* Don't bother with undelete information for these */
	if (!is_win32s) {
	    sprintf(buf, "MIME\\Database\\%s\\%s", contentname, pdfmime);
	    if (flag)
		flag = reg_open_key(newfile, oldfile, buf, &hkey);
	    if (flag) {
		flag = reg_set_value(newfile, oldfile, hkey, extension, pdfext);
		reg_close_key(&hkey);
	    }
	}
#endif

	if (flag)
	    flag = create_registry_type(newfile, oldfile, pdfkey, "Portable Document Format");
    }

    if (oldfile)
	fclose(oldfile);

    if (newfile)
	fclose(newfile);

    return !flag;
}


int
gsview_create_objects(char *groupname)
{
char gspath[MAXSTR];
char *p;
int rc;
    strcpy(gspath, option.gsdll);
    if ((p = strrchr(gspath,'\\')) != (char *)NULL)
	p++;
    else
	p = gspath;
    *p = '\0';

    rc = gsview_progman(groupname, szExePath, 
		option.gsversion, gspath, option.gsinclude);
    if (rc)
	gserror(IDS_NOPROGMAN, NULL, 0, SOUND_ERROR);
    return rc;
}

/***************************/

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
#ifdef __WIN32__
#ifdef DECALPHA
    char zlibname[] = "zlibda.dll";
#else
    char zlibname[] = "zlib32.dll";
#endif
#else
    char zlibname[] = "zlib16.dll";
#endif
    if (zlib_hinstance != (HINSTANCE)NULL)
	return TRUE;	/* already loaded */

    /* first look in GSview directory */
    strcpy(buf, szExePath);
    strcat(buf, zlibname);
    gs_addmess("Attempting to load ");
    gs_addmess(buf);
    gs_addmess("\n");
    zlib_hinstance = LoadLibrary(buf);
    if (zlib_hinstance < (HINSTANCE)HINSTANCE_ERROR) {
	/* if that fails, use the system search path */
	strcpy(buf, zlibname);
	gs_addmess("Attempting to load ");
	gs_addmess(buf);
	gs_addmess("\n");
	zlib_hinstance = LoadLibrary(buf);
    }
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
	    nHelpTopic = IDS_TOPICZLIB;
	    get_help();
	}
	return FALSE;
    }
    
    return TRUE;
}

/***************************/

#ifdef __WIN32__
HINSTANCE bzip2_hinstance;
PFN_bzopen bzopen;
PFN_bzread bzread;
PFN_bzclose bzclose;

void
unload_bzip2(void)
{
    if (bzip2_hinstance == (HINSTANCE)NULL)
	return;
    FreeLibrary(bzip2_hinstance);
    bzip2_hinstance = NULL;
    bzopen = NULL;
    bzread = NULL;
    bzclose = NULL;
}

/* load bzip2 DLL for gunzip */
BOOL
load_bzip2(void)
{   
char buf[MAXSTR];
    char bzip2name[] = "libbz2.dll";
    if (bzip2_hinstance != (HINSTANCE)NULL)
	return TRUE;	/* already loaded */

    /* first look in GSview directory */
    strcpy(buf, szExePath);
    strcat(buf, bzip2name);
    gs_addmess("Attempting to load ");
    gs_addmess(buf);
    gs_addmess("\n");
    bzip2_hinstance = LoadLibrary(buf);
    if (bzip2_hinstance < (HINSTANCE)HINSTANCE_ERROR) {
	/* if that fails, use the system search path */
	strcpy(buf, bzip2name);
	bzip2_hinstance = LoadLibrary(buf);
	gs_addmess("Attempting to load ");
	gs_addmess(buf);
	gs_addmess("\n");
    }
    if (bzip2_hinstance >= (HINSTANCE)HINSTANCE_ERROR) {
        bzopen = (PFN_bzopen) GetProcAddress(bzip2_hinstance, "BZ2_bzopen");
	if (bzopen == NULL) {
	    unload_bzip2();
	}
	else {
	    bzread = (PFN_bzread) GetProcAddress(bzip2_hinstance, "BZ2_bzread");
	    if (bzread == NULL) {
		unload_bzip2();
	    }
	    else {
		bzclose = (PFN_bzclose) GetProcAddress(bzip2_hinstance, "BZ2_bzclose");
		if (bzclose == NULL) {
		    unload_bzip2();
		}
	    }
	}
    }
    else
	bzip2_hinstance = NULL;

    if (bzip2_hinstance == NULL) {
	load_string(IDS_BZIP2_FAIL, buf, sizeof(buf));
	if (message_box(buf, MB_OKCANCEL) == IDOK) {
	    nHelpTopic = IDS_TOPICBZIP2;
	    get_help();
	}
	return FALSE;
    }
    
    return TRUE;
}

#endif

/****************************************************/
/* Easy Configure */

#ifdef __BORLANDC__
#pragma argsused
#endif
/* easy configure dialog box */
BOOL CALLBACK _export
EasyConfigureDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    WORD notify_message;
    switch(message) {
	case WM_INITDIALOG:
	    {
		int *gsver = (int *)lParam;
		int i;
		char buf[16];
	        for (i=1; i<=gsver[0]; i++) {
		    gsver_string(gsver[i], buf);
		    /* put string in list box */
		    SendDlgItemMessage(hDlg, IDC_GSVER, LB_ADDSTRING, 
			0, (LPARAM)((LPSTR)buf));
		}
		SendDlgItemMessage(hDlg, IDC_GSVER, LB_SETCURSEL, 
		    gsver[0]-1, 0L);
	    }
	    return TRUE;
        case WM_COMMAND:
	    notify_message = GetNotification(wParam,lParam);
            switch(LOWORD(wParam)) {
		case IDOK:
		    {
		    char buf[16];
		    int i = (int)SendDlgItemMessage(hDlg, IDC_GSVER, 
			LB_GETCURSEL, 0, 0L);
		    SendDlgItemMessage(hDlg, IDC_GSVER, LB_GETTEXT, 
			i, (LPARAM)(LPSTR)buf);
                    EndDialog(hDlg, gsver_int(buf));
		    }
                    return(TRUE);
                case IDCANCEL:
                    EndDialog(hDlg, 0);
                    return(TRUE);
		case ID_HELP:
		    get_help();
		    return(FALSE);
		case IDC_GSVER:
		    if (notify_message == LBN_DBLCLK)
			PostMessage(hDlg, WM_COMMAND, IDOK, 0);
		    return(FALSE);
                default:
                    return(FALSE);
            }
    }
    return(FALSE);
}

int
config_easy(void)
{
#ifndef __WIN32__
#error Win16 is no longer supported
#endif
	int result = 0;
	int *gsver;
	int gs_count = 0;
	get_gs_versions(&gs_count);
	if (gs_count == 0)
	   return 1;
	gsver = (int *)malloc(sizeof(int) * (gs_count + 1));
	if (gsver == (int *)NULL)
	    return 1;
	gsver[0] = gs_count+1;
	nHelpTopic = IDS_TOPICEASYCFG;
	if (get_gs_versions(gsver)) {
	    if (gsver[0] == 1) {
		/* Only one copy of Ghostscript installed */
		/* Don't prompt user */
		result = gsver[1];
	    }
	    else {
		/* Multiple copies of Ghostscript installed */
		/* Ask user to choose one */
		result = DialogBoxParam(hlanguage, "EasyConfigureDlgBox", 
			hwndimg, EasyConfigureDlgProc, (LPARAM)gsver);
	    }
	}
	free(gsver);

	if (result == 0)
	    return 1;	/* don't configure */

	option.gsversion= result;
	get_gs_string(option.gsversion, "GS_DLL", option.gsdll, 
	    sizeof(option.gsdll));
	get_gs_string(option.gsversion, "GS_LIB", option.gsinclude, 
	    sizeof(option.gsinclude));
	strcpy(option.gsother, "-dNOPLATFONTS -sFONTPATH=\042c:\\psfonts\042");

	/* copy printer.ini */
	gsview_printer_profiles();

	option.configured = TRUE;

	write_profile();

	return 0; /* success */
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
    if (isdigit((int)(p[0])) && (p[1]=='.') && 
	isdigit((int)(p[2])) && isdigit((int)(p[3]))) {
	strcpy(p, verstr);
        SetDlgItemText(hwnd, IDC_CFG22, buf);
    }
    else {
	p = buf + strlen(buf) - 3;
	if (isdigit((int)(p[0])) && (p[1]=='.') && isdigit((int)(p[2]))) {
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

    /* get info from wizard */
    page = find_page_from_id(IDD_CFG2);
    option.gsversion = add_gsver(page->hwnd, 0);
    GetDlgItemText(page->hwnd, IDC_CFG22, buf, sizeof(buf));
    if (option.gsversion >= 593) {
        sprintf(option.gsdll, "%s\\bin\\%s", buf, GS_DLLNAME);
    }
    else {
        sprintf(option.gsdll, "%s\\%s", buf, GS_DLLNAME);
    }
    default_gsinclude_from_path(option.gsinclude, buf);
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
    strcpy(buf, option.gsinclude);
    p = strchr(buf, ';');	/* remove trailing paths */
    if (p)
	*p = '\0';
    strcat(buf, "\\gs_init.ps");
    if ( (f = fopen(buf, "rb")) == (FILE *)NULL ) {
	load_string(IDS_GSLIBNOTINSTALLED, buf, sizeof(buf));
	SetDlgItemText(find_page_from_id(IDD_CFG7)->hwnd, IDC_CFG71,
	    buf);
	return 1;
    }
    fclose(f);
    /* at this stage we don't look for fonts, but maybe we should */

    assoc_ps = (BOOL)SendDlgItemMessage(find_page_from_id(IDD_CFG4)->hwnd, 
	    IDC_CFG41, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
    assoc_pdf = (BOOL)SendDlgItemMessage(find_page_from_id(IDD_CFG4)->hwnd, 
	    IDC_CFG42, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
    if (update_registry(assoc_ps, assoc_pdf)) {
	return 1;
    }

    GetDlgItemText(find_page_from_id(IDD_CFG5)->hwnd, 
	IDC_CFG52, buf, sizeof(buf));
    if (SendDlgItemMessage(find_page_from_id(IDD_CFG5)->hwnd, 
	    IDC_CFG51, BM_GETCHECK, (WPARAM)0, (LPARAM)0)
	&& gsview_create_objects(buf)) {
	return 1;
    }
    
    if (SendDlgItemMessage(find_page_from_id(IDD_CFG3)->hwnd, 
	    IDC_CFG32, BM_GETCHECK, (WPARAM)0, (LPARAM)0))
	gsview_printer_profiles();

    option.configured = TRUE;

    write_profile();

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

#ifdef __BORLANDC__
#pragma argsused
#endif
/* Download GS dialog box */
BOOL CALLBACK _export
DownloadGSDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
		case IDOK:
                    EndDialog(hDlg, TRUE);
                    return(TRUE);
                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return(TRUE);
		case ID_HELP:
		    get_help();
		    return(FALSE);
                default:
                    return(FALSE);
            }
    }
    return(FALSE);
}


int
config_wizard(void)
{
    /* We don't use a configure wizard anymore - this is done in
     * the setup program.
     * Instead we have several options:
     * 1. GS is installed on hard disk - offer the easy configure
     *    which relies on the setup program having written entries
     *    to the registry.
     * 2. If GS not installed, or 1. fails, look for 
     *      ..\gsN.NN\bin\gsdll32.dll 
     *    If this exists, configure silently since we are either
     *    running from CD-ROM or network drive.
     * 3. Tell user to download GS
     */   
    int gscount;
    char basedir[MAXSTR];
    char gsdir[MAXSTR];
    char gsdll[MAXSTR];
    int gsver;
    char *p;
    FILE *f;

    /* 1. If GS installed, easy configure */
    gscount = 0;
    get_gs_versions(&gscount);
    if (gscount > 0) {
	if (config_easy() == 0)
	    return 0;	/* success */
    }

    /* 2. GS not installed.  Look for GS in adajacent directory */
    strcpy(basedir, szExePath);
    p = strrchr(basedir, '\\');	/* remove trailing backslash */
    if (p)
	*p = '\0';
    p = strrchr(basedir, '\\');	/* remove trailing gsview */
    if (p)
	*(++p) = '\0';
    strcpy(gsdir, basedir);

    p = gsdir + strlen(gsdir);


    gs_addmess("Ghostscript registry entries not present.\n");

    gsver = GS_REVISION;
    while (gsver <= GS_REVISION_MAX) {
	if (gsver % 100 == 0)
	    sprintf(p, "gs%d.%d", gsver / 100, gsver % 100);
	else
	    sprintf(p, "gs%d.%02d", gsver / 100, gsver % 100);

	strcpy(gsdll, gsdir);
	strcat(gsdll, "\\bin\\gsdll32.dll");

	if ( (f = fopen(gsdll, "rb")) != (FILE *)NULL ) {
	    /* GS DLL exists. Configure GSview */
	    fclose(f);
	    gs_addmess("Found ");
	    gs_addmess(gsdll);
	    gs_addmess("\n");
	    option.gsversion = gsver;
	    strcpy(option.gsdll, gsdll);
	    sprintf(option.gsinclude, "%s\\lib;%sfonts", gsdir, basedir);
	    strcpy(option.gsother, "-dNOPLATFONTS -sFONTPATH=\042c:\\psfonts\042");
	    gsview_printer_profiles();
	    option.configured = TRUE;
	    write_profile();
	    return 0;	/* success */
	}
	gsver++;
    }

    
    nHelpTopic = IDS_TOPICDOWNLOAD;
    if (DialogBoxParam(hlanguage, "DownloadGSDlgBox", hwndimg, 
		DownloadGSDlgProc, (LPARAM)0)) {
	/* download now */
	ShellExecute(hwndimg, NULL, "http://www.cs.wisc.edu/~ghost/index.html",
	    NULL, NULL, SW_SHOWNORMAL);
    }
    
    return 1;	/* failed */
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
		    post_command_line();
		    /* should post message to main window to delete thunks */
                    return(TRUE);
                default:
                    return(FALSE);
            }
	case WM_CLOSE:
	    EnableWindow(hwndimg, TRUE);
	    DestroyWindow(hwnd);
	    hDlgModeless = NULL;
	    post_command_line();
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


