/* Copyright (C) 1993-1998, Ghostgum Software Pty Ltd.  All rights reserved.;
  
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

/* winsetup.c */
/* MS-Windows installation program for GSview and Ghostscript */

#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _MSC_VER
#include <direct.h>
#else
#include <dir.h>
#endif
#include <ctype.h>
#include <io.h>

#define MAXSTR 256

#include "gvcver.h"
#include "gvcbeta.h"
#include "gvcrc.h"
#include "setup.h"
#include "gvclang.h"
#include "setupc.h"

void install_init(void);
int gsview_progman(char *groupname, char *gsviewpath, int gsver,
	char *gspath, char *gsargs);

BOOL CALLBACK _export ModelessDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK _export MainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

/* variables that affect where we install things */
char groupname[MAXSTR];
char groupfile[MAXSTR];

HINSTANCE phInstance;
HINSTANCE hlanguage;
#ifdef __WIN32__
#ifdef DECALPHA
char szUnzipDll[]="wizunzda.dll";
#else
char szUnzipDll[]="wizunz32.dll";
#endif
char szIniName[]="gsview32.ini";
#else
char szUnzipDll[]="wizunz16.dll";
char szIniName[]="gsview16.ini";
#endif
int is_win32s;
int is_win4;

#ifdef __WIN32__
/* early versions of Win32s don't support lstrcpyn */
#undef lstrcpyn
#define lstrcpyn(d,s,n) strncpy(d,s,n)
#endif

int
dialog(int resource, DLGPROC dlgproc) 
{
int flag;
#ifndef __WIN32__
DLGPROC lpProcDlg;
#endif
#ifdef __WIN32__
    flag = DialogBoxParam(hlanguage, MAKEINTRESOURCE(resource), hMain, dlgproc, (LPARAM)NULL);
#else
    lpProcDlg = (DLGPROC)MakeProcInstance((FARPROC)dlgproc, phInstance);
    flag = DialogBoxParam(hlanguage, MAKEINTRESOURCE(resource), hMain, lpProcDlg, (LPARAM)NULL);
    FreeProcInstance((FARPROC)lpProcDlg);
#endif
    return flag;
}

int message_box(char *str, int icon)
{
    return MessageBox(hMain, str, szAppName, icon);
}

/* change directory and drive */
int
gs_chdir(char *dirname)
{
#ifdef __WIN32__
    return !SetCurrentDirectory(dirname);
#else
    if (isalpha(dirname[0]) && (dirname[1]==':'))
	(void)setdisk(toupper(dirname[0])-'A');
    if (!((strlen(dirname)==2) && isalpha(dirname[0]) && (dirname[1]==':')))
	return chdir(dirname);
    return -1;
#endif
}

int
load_string(int id, char *str, int len)
{
	return LoadString(hlanguage, id, str, len);
}

void
gs_addmess_update(void)
{
  HWND hwndmess = find_page_from_id(IDD_TEXTWIN)->hwnd;

  if (IsWindow(hwndmess)) {
    HWND hwndtext = GetDlgItem(hwndmess, TEXTWIN_MLE);
    DWORD linecount;
    SendMessage(hwndtext, WM_SETREDRAW, FALSE, 0);
    SetDlgItemText(hwndmess, TEXTWIN_MLE, twbuf);
#ifdef __WIN32__
    /* EM_SETSEL, followed by EM_SCROLLCARET doesn't work */
    linecount = SendDlgItemMessage(hwndmess, TEXTWIN_MLE, EM_GETLINECOUNT, (WPARAM)0, (LPARAM)0);
    SendDlgItemMessage(hwndmess, TEXTWIN_MLE, EM_LINESCROLL, (WPARAM)0, (LPARAM)linecount-14);
#else
    linecount = SendDlgItemMessage(hwndmess, TEXTWIN_MLE, EM_GETLINECOUNT, (WPARAM)0, (LPARAM)0);
    SendDlgItemMessage(hwndmess, TEXTWIN_MLE, EM_LINESCROLL, (WPARAM)0, MAKELPARAM(linecount-14, 0));
#endif
    SendMessage(hwndtext, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(hwndtext, (LPRECT)NULL, TRUE);
    UpdateWindow(hwndtext);
  }
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
	hwnd_current = page->hwnd;
	if (IsWindowEnabled(GetDlgItem(page->hwnd, IDNEXT)))
	    hbutton = GetDlgItem(page->hwnd, IDNEXT);
	else
	    hbutton = GetDlgItem(page->hwnd, IDCANCEL);
	SetFocus(hbutton);
	SendMessage(hbutton, BM_SETSTYLE, (WPARAM)BS_DEFPUSHBUTTON, TRUE);
	return;
    }
}

#ifdef __BORLANDC__
#pragma argsused
#endif
/* exit from program */
int
done(HWND hwnd)
{
    PostQuitMessage(0);
    return 0;
}

/* commence installation */
int
finish(HWND hwnd)
{
WIZPAGE *page;
    page = find_page_from_id(IDD_GSVER);
    install_gsview = (BOOL)SendDlgItemMessage(page->hwnd, PARTIAL_GSVIEW, BM_GETCHECK, 
		    (WPARAM)0, (LPARAM)0);
    install_gs = (BOOL)SendDlgItemMessage(page->hwnd, PARTIAL_GS, BM_GETCHECK, 
		    (WPARAM)0, (LPARAM)0);
    /* assume gsver is already correct */

    page = find_page_from_id(IDD_CONFIG);
    install_autoexec = (BOOL)SendDlgItemMessage(page->hwnd, IDM_AUTOEXECBAT, BM_GETCHECK, 
		    (WPARAM)0, (LPARAM)0);
    install_autoexecbak = (BOOL)SendDlgItemMessage(page->hwnd, IDM_AUTOEXECBAK, BM_GETCHECK, 
		    (WPARAM)0, (LPARAM)0);
    /* assume destdir is already correct */

    page = find_page_from_id(IDD_FINISH);
    install_group = (BOOL)SendDlgItemMessage(page->hwnd, IDM_PROGMAN1, BM_GETCHECK, 
		    (WPARAM)0, (LPARAM)0);
    GetDlgItemText(page->hwnd, IDM_PROGMAN2, groupname, sizeof(groupname));

    PostMessage(hwnd, WM_COMMAND, (WPARAM)IDFINISH, (LPARAM)0);
    return 0;
}

int
update_config(void)
{
FILE *infile, *outfile;
char inname[MAXSTR], outname[MAXSTR];
char buf[MAXSTR];
char line[1024];
char tempname[MAXSTR];
int i;
    
    strcpy(inname, bootdrive);
    strcat(inname, "\\autoexec.bat");

    strcpy(tempname, bootdrive);
    strcat(tempname, "\\GSXXXXXX");
    if (mktemp(tempname) == (char *)NULL) {
	load_string(IDS_CANTCREATETEMPFILE, error_message, sizeof(error_message));
	return 1;
    }

    if ( (infile = fopen(inname, "r")) == (FILE *)NULL) {
	load_string(IDS_CANTOPENREAD, buf, sizeof(buf));
	sprintf(error_message, buf, inname);
	return 1;
    }
    if ( (outfile = fopen(tempname, "w")) == (FILE *)NULL)  {
	load_string(IDS_CANTOPENWRITE, buf, sizeof(buf));
	sprintf(error_message, buf, tempname);
	return 1;
    }
    while (fgets(line, sizeof(line), infile)) {
	if (strnicmp(line, "SET TEMP=", 9)==0) {
	    /* it has been added recently */
	    fclose(outfile);
	    fclose(infile);
	    unlink(tempname);
	    return 0;
	}
	fputs(line, outfile);
    }
    sprintf(line, "SET TEMP=%s\\\n", bootdrive);
    fputs(line, outfile);

    fclose(outfile);
    fclose(infile);

    if (install_autoexecbak) {
	for (i=0; i<=999; i++) {
	    sprintf(outname, "%s\\autoexec.%03d", bootdrive, i);
	    if ( (infile = fopen(outname, "r")) != (FILE *)NULL)
		fclose(infile);
	    else
		break;   /* found a suitable name */
	}
        if (rename(inname, outname)) {
	    load_string(IDS_ERRORRENAME, buf, sizeof(buf));
	    sprintf(error_message, buf, inname, outname);
	    return 1;
	}
	strcpy(autoexec_bak, outname);
    }
    else
	unlink(inname);

    if (rename(tempname, inname)) {
	load_string(IDS_ERRORRENAME, buf, sizeof(buf));
	sprintf(error_message, buf, tempname, inname);
	return 1;
    }

    return 0;
}

int
update_ini(char *ininame)
{
char buf[16];
    sprintf(buf, "%3d", gsver);
    WritePrivateProfileString("Options", "Configured", "0", ininame);
    WritePrivateProfileString("Options", "GSversion", buf, ininame);
    return 0;
}

int
create_object(void)
{
int rc;
char gspath[MAXSTR], gsargs[MAXSTR];
char gsviewpath[MAXSTR];
    sprintf(gspath, "%s\\%s\\", destdir, gs_basedir);
    sprintf(gsargs, "%s\\%s;%s\\%s\\fonts", destdir, gs_basedir, 
	destdir, gs_basedir);
    sprintf(gsviewpath, "%s\\%s\\", destdir, gsviewbase);
    rc = gsview_progman(groupname, gsviewpath, gsver, gspath, gsargs);

#ifdef __WIN32__
    /* Create default registry entries for Ghostscript */
    if (!is_win32s) {
	HKEY hkey;
	LONG lrc;
	char buf[MAXSTR];
	sprintf(buf, "SOFTWARE\\Aladdin Ghostscript\\%d.%02d", 
	    gsver / 100, gsver % 100);
	lrc = RegCreateKey(HKEY_LOCAL_MACHINE, buf, &hkey);
	if (lrc == ERROR_SUCCESS) {
	    lrc = RegSetValueEx(hkey, "GS_LIB", 0, REG_SZ, 
		(CONST BYTE *)gsargs, strlen(gsargs)+1);
	    sprintf(buf, "%s%s", gspath, GS_DLLNAME);
	    if (lrc == ERROR_SUCCESS)
		lrc = RegSetValueEx(hkey, "GS_DLL", 0, REG_SZ, 
		    (CONST BYTE *)buf, strlen(buf)+1);
	    RegCloseKey(hkey);
	}
	if (lrc != ERROR_SUCCESS)
	    rc = 1;
    }
#endif

    if (rc)
	load_string(IDS_NODDEPROGMAN, error_message, sizeof(error_message));

    /* tell user what we have done */
    load_string(IDS_PROGMANGROUP5, gsargs, sizeof(gsargs));
    sprintf(gspath, gsargs, groupname);
    SetDlgItemText(find_page_from_id(IDD_DONE)->hwnd, IDD_DONE_GROUP, gspath);

    return rc;
}


DLGPROC lpMainDlgProc;
DLGPROC lpChildDlgProc;


int
create_dialog(void)
{
WIZPAGE *page;
char buf[MAXSTR];
    /* main dialog box */
#ifdef __WIN32__
    hMain = CreateDialogParam(hlanguage, MAKEINTRESOURCE(IDD_MAIN), (HWND)NULL, MainDlgProc, (LPARAM)NULL);
#else
    lpMainDlgProc = (DLGPROC)MakeProcInstance((FARPROC)MainDlgProc, phInstance);
    lpChildDlgProc = (DLGPROC)MakeProcInstance((FARPROC)ModelessDlgProc, phInstance);
    hMain = CreateDialogParam(hlanguage, MAKEINTRESOURCE(IDD_MAIN), (HWND)NULL, lpMainDlgProc, (LPARAM)NULL);
#endif

    sprintf(buf, "%d.%02d - %d.%02d", 
	GS_REVISION_MIN / 100, GS_REVISION_MIN % 100,
	GS_REVISION_MAX / 100, GS_REVISION_MAX % 100);
    SetDlgItemText(find_page_from_id(IDD_INTRO)->hwnd,
	IDD_INTRO_T3, buf);

    /* initialize GS version */
    page = find_page_from_id(IDD_GSVER);
    if (page) {
	sprintf(buf, "%d.%02d", GS_REVISION / 100, GS_REVISION % 100);
	SetDlgItemText(page->hwnd, IDD_GSVER_TEXT, buf);
	SendDlgItemMessage(page->hwnd, PARTIAL_GSVIEW, BM_SETCHECK, 
		    (WPARAM)1, (LPARAM)0);
	SendDlgItemMessage(page->hwnd, PARTIAL_GS, BM_SETCHECK, 
		    (WPARAM)1, (LPARAM)0);
    }
    /* initialize destination directory */
    strcpy(destdir, bootdrive);
    strcat(destdir, INSTALL_DIR);
    SetDlgItemText(find_page_from_id(IDD_DIR)->hwnd, ID_ANSWER, destdir);
    if (init_temp()) {
        page = find_page_from_id(IDD_CONFIG);
	SendDlgItemMessage(page->hwnd, IDM_AUTOEXECBAT, BM_SETCHECK, 
		    (WPARAM)1, (LPARAM)0);
	SendDlgItemMessage(page->hwnd, IDM_AUTOEXECBAK, BM_SETCHECK, 
		    (WPARAM)1, (LPARAM)0);
	find_page_from_id(IDD_FINISH)->prev = IDD_CONFIG; 
    }

    /* program group */
    load_string(IDS_PROGMANGROUP4, buf, sizeof(buf));
    page = find_page_from_id(IDD_FINISH);
    if (page) {
	SendDlgItemMessage(page->hwnd, IDM_PROGMAN1, BM_SETCHECK, 
		    (WPARAM)1, (LPARAM)0);
        SetDlgItemText(page->hwnd, IDM_PROGMAN2, buf);
    }

    return 0; /* success */
}

void
install_init(void)
{
int i;
char *s, *d;
    if (gsver % 100 == 0)
	sprintf(gs_basedir, "gs%d.%01d", gsver / 100, gsver % 100);
    else
	sprintf(gs_basedir, "gs%d.%02d", gsver / 100, gsver % 100);
    sprintf(gs_zipprefix, "gs%3d", gsver);

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
}


int
init_setup(LPSTR lpszCmdLine)
{
char *p;
DWORD version = GetVersion();
LPSTR d, s;
    if (lpszCmdLine[0] != '\0') {
	d = destdir;
	s = lpszCmdLine;
	if (*s == '\042')
	    s++; 		/* don't copy quotes */
	while (*s) {
	    if (*s == '\042')
		s++; 	/* don't copy quotes */
	    else
		*d++ = *s++;
	    if (d - destdir > sizeof(destdir) - 1) {
	        *d = '\0';
		break;
	    }
	}
	batch = TRUE;
    }

    /* find out if we are running under Win32s */
    /* Win32s */
    if ( ((HIWORD(version) & 0x8000)!=0) && ((HIWORD(version) & 0x4000)==0) )
	    is_win32s = TRUE;
    /* Windows 4.0 */
    if (LOBYTE(LOWORD(version)) >= 4)
	is_win4 = TRUE;

    /* get path to EXE */
    GetModuleFileName(phInstance, sourcedir, sizeof(sourcedir));
    gs_addmess("Setup program is ");
    gs_addmess(sourcedir);
    gs_addmess("\n");

    if ((p = strrchr(sourcedir,'\\')) != (char *)NULL)
	p++;
    else
	p = sourcedir;
    *p = '\0';
    gs_addmess("Source directory ");
    gs_addmess(sourcedir);
    gs_addmess("\n");

    /* Inspect system, get boot drive */
    getcwd(workdir, sizeof(workdir));	/* remember the working directory */
    strcpy(bootdrive, "c:");

    gs_addmess("Current directory ");
    gs_addmess(workdir);
    gs_addmess("\n");

    gsver = GS_REVISION;

    return 0;
}

/* returns TRUE if language change successful */
BOOL
load_language(int language)
{   /* load language dependent resources */
char langdll[MAXSTR];
HINSTANCE hInstance;
    /* load language dependent resources */
    strcpy(langdll, sourcedir);
#ifdef __WIN32__
    strcat(langdll, "setp32");
#else
    strcat(langdll, "setp16");
#endif
    switch (language) {
	case IDM_LANGDE:
	    strcat(langdll, "de");
	    break;
	case IDM_LANGFR:
	    strcat(langdll, "fr");
	    break;
	case IDM_LANGIT:
	    strcat(langdll, "it");
	    break;
	case IDM_LANGEN:
	default:
	    hlanguage = phInstance;
	    return TRUE;
    }
    strcat(langdll, ".dll");
    hInstance = LoadLibrary(langdll);
    if (hInstance >= (HINSTANCE)HINSTANCE_ERROR) {
	hlanguage = hInstance;
	return TRUE;
    }
    
    hlanguage = phInstance;
    return FALSE;
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
    /* if Window language isn't English */
    if ( strnicmp(winlang, "EN", 2)
#ifdef BETA
      || TRUE
#endif
	)
    {
#ifdef __WIN32__
	language = DialogBoxParam(hlanguage, MAKEINTRESOURCE(IDD_LANG), HWND_DESKTOP, LanguageDlgProc, (LPARAM)NULL);
#else
	DLGPROC lpProcLanguage;
	lpProcLanguage = (DLGPROC)MakeProcInstance((FARPROC)LanguageDlgProc, phInstance);
	language = DialogBoxParam(hlanguage, MAKEINTRESOURCE(IDD_LANG), HWND_DESKTOP, lpProcLanguage, (LPARAM)NULL);
	FreeProcInstance((FARPROC)lpProcLanguage);
#endif
	switch (language) {
	    case IDM_LANGEN:
	    case IDM_LANGDE:
	    case IDM_LANGFR:
	    case IDM_LANGIT:
		load_language(language);
	}
    }
}

#ifdef __BORLANDC__
#pragma argsused	/* ignore warning for next function */
#endif
int PASCAL 
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int cmdShow)
{
int rc;
MSG msg;
    /* copy the hInstance into a variable so it can be used */
    hlanguage = phInstance = hInstance;

    init_setup(lpszCmdLine);

    check_language();

    load_string(IDS_GSVIEWBASE, gsviewbase, sizeof(gsviewbase));
    load_string(IDS_PROGMANGROUP4, groupname, sizeof(groupname));

    if (!beta_warn()) {
	if (batch)
	    rc = do_install();
	else
	    rc = create_dialog();
	
	if (!batch) {
	    while (GetMessage(&msg, (HWND)NULL, 0, 0)) {
		if (!IsDialogMessage(hwnd_current, &msg)
		    && !IsDialogMessage(hMain, &msg) ) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	    }
	    DestroyWindow(hMain);
	}

#ifndef __WIN32__
	if (!batch) {
	    if (lpMainDlgProc)
		FreeProcInstance((FARPROC)lpMainDlgProc);
	    if (lpChildDlgProc)
		FreeProcInstance((FARPROC)lpChildDlgProc);
	}
#endif
    }
    if (hlanguage != phInstance)
	FreeLibrary(hlanguage);

    return rc;
}


#ifdef __BORLANDC__
#pragma argsused	/* ignore warning for next function */
#endif
/* input string dialog box */
BOOL CALLBACK _export
InputDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
        case WM_INITDIALOG:
	    SetDlgItemText(hDlg, ID_ANSWER, get_string_answer);
            return( TRUE);
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
		case IDOK:
		    GetDlgItemText(hDlg, ID_ANSWER, get_string_answer, sizeof(get_string_answer));
                    EndDialog(hDlg, IDOK);
                    return(TRUE);
                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    return(TRUE);
                default:
                    return(FALSE);
            }
    }
    return(FALSE);
}

#ifdef __BORLANDC__
#pragma argsused	/* ignore warning for next function */
#endif
/* Modeless Dialog Box */
BOOL CALLBACK _export
MainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
static BOOL initialised;
    switch(message) {
        case WM_INITDIALOG:
	    /* create child dialog windows */
	    if (!initialised) {
		WIZPAGE *page;
		initialised=TRUE;
		for (page=pages; page->id; page++) {
#ifdef __WIN32__
		    page->hwnd = CreateDialogParam(hlanguage, MAKEINTRESOURCE(page->id), hDlg, ModelessDlgProc, (LPARAM)NULL);
#else
		    page->hwnd = CreateDialogParam(hlanguage, MAKEINTRESOURCE(page->id), hDlg, lpChildDlgProc, (LPARAM)NULL);
#endif
		    ShowWindow(page->hwnd, SW_HIDE);
		}
		ShowWindow(pages[0].hwnd, SW_SHOW);
		SetFocus(GetDlgItem(pages[0].hwnd, IDNEXT));
		SendDlgItemMessage(pages[0].hwnd, IDNEXT, BM_SETSTYLE, 
		    (WPARAM)BS_DEFPUSHBUTTON, TRUE);
		hwnd_current = pages[0].hwnd;
	    }
            return FALSE;	/* we decide the focus */
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case IDCANCEL:
                case IDOK:
		    PostQuitMessage(0);
                    return(TRUE);
                default:
                    return(FALSE);
            }
	case WM_CLOSE:
	    PostQuitMessage(0);
	    return TRUE;
    }
    return FALSE;
}


#ifdef __BORLANDC__
#pragma argsused	/* ignore warning for next function */
#endif
/* Modeless Dialog Box */
BOOL CALLBACK _export
ModelessDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
        case WM_INITDIALOG:
            return( TRUE);
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
		case IDOK:
                case IDNEXT:
		    next_page(hDlg);
                    return(TRUE);
                case IDPREV:
		    SendDlgItemMessage(hDlg, IDPREV, BM_SETSTYLE, 
		        (WPARAM)0, TRUE);	/* remove default style */
		    prev_page(hDlg);
                    return(TRUE);
		case IDFINISH:
		    installing = TRUE;
		    if (do_install() || cancelling) {
			WIZPAGE *page;
			/* FAILED */
			failed = TRUE;
		        page = find_page_from_id(IDD_TEXTWIN);
			page->next = IDD_FAILED;	/* KLUDGE */
			EnableWindow(GetDlgItem(page->hwnd, IDNEXT), TRUE);
			SetDlgItemText(find_page_from_id(IDD_FAILED)->hwnd,
			   IDD_FAILED_REASON, error_message); 
			gs_addmess(error_message);
			gs_addmess_update();
		    }
		    else {
			WIZPAGE *page;
		        page = find_page_from_id(IDD_TEXTWIN);
			page->next = IDD_DONE;	/* KLUDGE */
			EnableWindow(GetDlgItem(page->hwnd, IDNEXT), TRUE);
		    }
		    installing = FALSE;
		    if (autoexec_bak[0] != '\0') {
			char buf1[MAXSTR], buf2[MAXSTR];
			load_string(IDS_DONEBAK, buf1, sizeof(buf2));
			sprintf(buf2, buf1, autoexec_bak);
			SetDlgItemText(find_page_from_id(IDD_DONE)->hwnd,
			   IDD_DONE_BAK, buf2); 
		    }
		    PostMessage(find_page_from_id(IDD_TEXTWIN)->hwnd,
			WM_COMMAND, (WPARAM)IDNEXT, (LPARAM)0);
                    return(TRUE);
                case IDCANCEL:
		    if (installing) {
			gs_addmess("\ncancelling\n");
			cancelling = 1;
		    }
		    else
		        PostMessage(GetParent(hDlg), WM_COMMAND, (WPARAM)IDCANCEL, (LPARAM)0);
                    return(TRUE);
		case TEXTWIN_COPY:
		    {HGLOBAL hglobal;
		    LPSTR p;
		    DWORD result;
		    int start, end;
		    result = SendDlgItemMessage(hDlg, TEXTWIN_MLE, EM_GETSEL, (WPARAM)0, (LPARAM)0);
		    start = LOWORD(result);
		    end   = HIWORD(result);
		    if (start == end) {
			start = 0;
			end = twend;
		    }
		    hglobal = GlobalAlloc(GHND | GMEM_SHARE, end-start+1);
		    if (hglobal == (HGLOBAL)NULL) {
			MessageBeep(-1);
			return(FALSE);
		    }
		    p = GlobalLock(hglobal);
		    if (p == (LPSTR)NULL) {
			MessageBeep(-1);
			return(FALSE);
		    }
		    lstrcpyn(p, twbuf+start, end-start);
		    GlobalUnlock(hglobal);
		    OpenClipboard(hDlg);
		    EmptyClipboard();
		    SetClipboardData(CF_TEXT, hglobal);
		    CloseClipboard();
		    }
                default:
                    return(FALSE);
            }
	case WM_VSCROLL:
	    { int ver;
	      char buf[16];
		switch(LOWORD(wParam)) {
		    case SB_LINEUP:
			ver = add_gsver(hDlg, 1);
			break;
		    case SB_LINEDOWN:
			ver = add_gsver(hDlg, -1);
			break;
		    default:
			ver = add_gsver(hDlg, 0);
			break;
		}
		sprintf(buf, "%d.%02d", ver / 100, ver % 100);
		SetDlgItemText(hDlg, IDD_GSVER_TEXT, buf);
	    }
	    return TRUE;
	case WM_CLOSE:
	    PostMessage(GetParent(hDlg), WM_COMMAND, (WPARAM)IDCANCEL, (LPARAM)0);
	    return TRUE;
    }
    return FALSE;
}


