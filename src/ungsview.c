/* Copyright (C) 1997-1998, Russell Lang.  All rights reserved.
  
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

/* ungsview.c */

#define STRICT
#include <windows.h>
#include <shellapi.h>
#include <ddeml.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "ungsview.h"
#include "gvcver.h"

#define MAXSTR 256
#ifdef __WIN32__
char prefix[] = "\\win32";
#else
char prefix[] = "\\win16";
#endif
#define UNINSTALLKEY TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall")

HWND hDlgModeless;
char path[MAXSTR];
int language = 0;
BOOL is_win32s = FALSE;
BOOL is_win4 = FALSE;
HINSTANCE phInstance;
char szIniFile[MAXSTR];
char title[MAXSTR];
char szGSversion[16];
BOOL removed_gs = FALSE;

#define COMPLETED 2
#define NOTDONE 1
#define FAIL 0

/* linked list for deleting registry entries in reverse order */
typedef struct tagKEY {
    long index;
    struct tagKEY *previous;
} KEY;
KEY *last_key = NULL;


/* first version of uninstall program is command line only */

int registry_delete(char *filename);
int registry_import(char *filename);
int doprogman(char *filename);
int dofiles(char *filename);

#ifndef __WIN32__
#define Sleep(x)
#endif

int
remove_progman(void)
{
int flag;
char fname[MAXSTR];
    Sleep(500);
    strcpy(fname, path);
    strcat(fname, prefix);
    strcat(fname, "dde.log");
    flag = doprogman(fname);
    if (flag == COMPLETED)
	unlink(fname);
   return flag;
}

int
remove_registry(void)
{
int flag;
char fname[MAXSTR];
    Sleep(500);
    strcpy(fname, path);
    strcat(fname, prefix);
    strcat(fname, "new.reg");
    flag = registry_delete(fname);
    if (flag == COMPLETED)
	unlink(fname);
   return flag;
}

int
restore_registry(void)
{
int flag;
char fname[MAXSTR];
    Sleep(500);
    strcpy(fname, path);
    strcat(fname, prefix);
    strcat(fname, "old.reg");
    flag = registry_import(fname);
    if (flag == COMPLETED)
	unlink(fname);
   return flag;
}

int
remove_files(void)
{
int flag;
char fname[MAXSTR];
    Sleep(500);
    strcpy(fname, path);
    strcat(fname, prefix);
    strcat(fname, ".log");
    flag = dofiles(fname);
    if (flag == COMPLETED)
	unlink(fname);
#ifdef __WIN32__
    /* Delete Ghostscript registry version key and names */
    if (!is_win32s && removed_gs) {
	HKEY hkey;
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Aladdin Ghostscript", 
	    0, 0, &hkey) == ERROR_SUCCESS) {
	    RegDeleteKey(hkey, szGSversion);
	    RegCloseKey(hkey);
	}
    }
#endif
   return flag;
}

int
dofiles(char *filename)
{
FILE *f;
char line[MAXSTR];
char gstag[16];
char ungsview_path[MAXSTR];
int ungsview_len;
char *p;
    GetModuleFileName(phInstance, ungsview_path, sizeof(ungsview_path));
    ungsview_len = strlen(ungsview_path);
    sprintf(gstag, "gs%s", szGSversion);
    if ((f = fopen(filename, "r")) == (FILE *)NULL)
	return NOTDONE;	/* this isn't an error */

    while (fgets(line, sizeof(line), f) != (char *)NULL) {
	if ( (p = strchr(line, '\n')) != (char *)NULL )
	    *p = '\0';
	if (strlen(line) == 0)
	    break;
	if (strnicmp(ungsview_path, line, ungsview_len) != 0)	
	    unlink(line); 		/* don't delete ourselves */
	if (strstr(line, gstag))
	    removed_gs = TRUE;	/* found gsN.NN */
    }
    fclose(f);
    return COMPLETED;
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
doprogman(char *logname)
{
DWORD idInst = 0L;
FARPROC lpDdeProc;
HSZ hszServName;
HSZ hszSysTopic;
HCONV hConv;
DWORD dwResult;
char *p;
FILE *logfile;
char line[MAXSTR];
    if ((logfile = fopen(logname, "r")) == (FILE *)NULL)
	return NOTDONE;	/* this isn't an error */

    lpDdeProc = MakeProcInstance((FARPROC)DdeCallback, phInstance);
    if (DdeInitialize(&idInst, (PFNCALLBACK)lpDdeProc, CBF_FAIL_POKES, 0L)) {
#ifndef __WIN32__
	FreeProcInstance(lpDdeProc);
#endif
	fclose(logfile);
	return FAIL;
    }
    hszServName = DdeCreateStringHandle(idInst, "PROGMAN", CP_WINANSI);
    hszSysTopic = DdeCreateStringHandle(idInst, "PROGMAN", CP_WINANSI);
    hConv = DdeConnect(idInst, hszServName, hszSysTopic, (PCONVCONTEXT)NULL);
    if (hConv == NULL) {
	DdeFreeStringHandle(idInst, hszServName);
	DdeFreeStringHandle(idInst, hszSysTopic);
	fclose(logfile);
	//fprintf(stderr, "No program manager!\n");
	return FAIL;
    }

#define DDEEXECUTE(str)\
    DdeClientTransaction((LPBYTE)str, strlen(str)+1, hConv,\
	NULL, CF_TEXT, XTYP_EXECUTE, 2000, &dwResult)

    while (fgets(line, sizeof(line), logfile) != (char *)NULL) {
	if ( (p = strchr(line, '\n')) != (char *)NULL )
	    *p = '\0';
	if (strlen(line) == 0)
	    break;
        DDEEXECUTE(line);
    }

#undef DDEXECUTE

    DdeDisconnect(hConv);
    DdeFreeStringHandle(idInst, hszServName);
    DdeFreeStringHandle(idInst, hszSysTopic);
    DdeUninitialize(idInst);
    fclose(logfile);
    return COMPLETED;
}



void
registry_unquote(char *line)
{
char *s, *d;
int value;
    s = d = line;
    while (*s) {
	if (*s != '\\') {
	    *d++ = *s;
	}
	else {
	    s++;
	    if (*s == '\\')
		*d++ = *s;
	    else {
		value = 0;
		if (*s) {
		    value = *s++ - '0';
		}
		if (*s) {
		    value <<= 3;
		    value += *s++ - '0';
		}
		if (*s) {
		    value <<= 3;
		    value += *s - '0';
		}
		*d++ = (char)value;
	    }
	}
	s++;
    }
    *d = '\0';
}

BOOL
registry_import2(FILE *f)
{
char line[MAXSTR];
HKEY hkey = HKEY_CLASSES_ROOT;
HKEY hrkey;
char *rkey, *skey;
char *value;
#ifdef __WIN32__
char *name;
DWORD dwResult;
#endif
    if (fgets(line, sizeof(line), f) == (char *)NULL)
	return FALSE;
    if (strncmp(line, "REGEDIT4", 8) != 0)
	return FALSE;

    while (fgets(line, sizeof(line), f) != (char *)NULL) {
	if ((line[0] == '\r') || (line[0] == '\n'))
	    continue;
	if (line[0] == '[') {
	    /* key name */
	    if (hkey != HKEY_CLASSES_ROOT) {
		RegCloseKey(hkey);
		hkey = HKEY_CLASSES_ROOT;
	    }
	    rkey = strtok(line+1, "\\]\n\r");
	    if (rkey == (char *)NULL)
		return FALSE;
	    skey = strtok(NULL, "]\n\r");
	    if (strcmp(rkey, "HKEY_CLASSES_ROOT")==0)
		hrkey = HKEY_CLASSES_ROOT;
#ifdef __WIN32__
	    else if (strcmp(rkey, "HKEY_CURRENT_USER")==0)
		hrkey = HKEY_CURRENT_USER;
	    else if (strcmp(rkey, "HKEY_LOCAL_MACHINE")==0)
		hrkey = HKEY_LOCAL_MACHINE;
	    else if (strcmp(rkey, "HKEY_USERS")==0)
		hrkey = HKEY_USERS;
#endif
	    else
		return FALSE;
	    if (skey == (char *)NULL)
		return FALSE;
#ifdef __WIN32__
	    if (!is_win32s) {
	        if (RegCreateKeyEx(hrkey, skey, 0, "", 0, KEY_ALL_ACCESS, 
		    NULL, &hkey, &dwResult)
		    != ERROR_SUCCESS)
		    return FALSE;
	    }
	    else
#endif	    
	    {
	        if (RegCreateKey(hrkey, skey, &hkey) != ERROR_SUCCESS)
		    return FALSE;
	    }
	}
	else if (line[0] == '@') {
	    /* default value */
	    if (strlen(line) < 4)
		return FALSE;
	    value = strtok(line+3, "\042\r\n");
	    if (value) {
		registry_unquote(value);
#ifdef __WIN32__
		if (!is_win32s) {
		    if (RegSetValueEx(hkey, NULL, 0, REG_SZ, 
			    (CONST BYTE *)value, strlen(value)+1)
			!= ERROR_SUCCESS)
			return FALSE;
		}
		else
#endif
		{
		    if (RegSetValue(hkey, NULL, REG_SZ, 
			    (LPSTR)value, strlen(value)+1)
			!= ERROR_SUCCESS)
			return FALSE;
		}
	    }
	}
#ifdef __WIN32__
	else if ( (line[0] == '\042') && !is_win32s ) {
	    /* named value */
	    name = strtok(line+1, "\042\r\n");
	    strtok(NULL, "\042\r\n");
	    value = strtok(NULL, "\042\r\n");
	    registry_unquote(value);
	    if (RegSetValueEx(hkey, name, 0, REG_SZ, (CONST BYTE *)value, strlen(value)+1)
		!= ERROR_SUCCESS)
		return FALSE;
	}
#endif
    }
    if (hkey != HKEY_CLASSES_ROOT)
	RegCloseKey(hkey);
    return TRUE;
}

int
registry_import(char *filename)
{
FILE *rfile;
int flag;
    rfile = fopen(filename, "r");
    if (rfile == (FILE *)NULL)
	return NOTDONE;
    flag = registry_import2(rfile);
    fclose(rfile);
    if (!flag)
	return FAIL;
    return COMPLETED;
}

BOOL
registry_delete_key(FILE *f, char *line)
{
char keyname[MAXSTR];
HKEY hkey = HKEY_CLASSES_ROOT;
HKEY hrkey = HKEY_CLASSES_ROOT;
char *rkey, *skey;
#ifdef __WIN32__
char *name;
DWORD dwResult;
#endif
    keyname[0] = '\0';
    while (fgets(line, MAXSTR, f) != (char *)NULL) {
	if ((line[0] == '\0') || (line[0] == '\r') || (line[0] == '\n'))
	    break;
	if (line[0] == '[') {
	    /* key name */
	    rkey = strtok(line+1, "\\]\n\r");
	    if (rkey == (char *)NULL)
		return FALSE;
	    skey = strtok(NULL, "]\n\r");
	    if (strcmp(rkey, "HKEY_CLASSES_ROOT")==0)
		hrkey = HKEY_CLASSES_ROOT;
#ifdef __WIN32__
	    else if (strcmp(rkey, "HKEY_CURRENT_USER")==0)
		hrkey = HKEY_CURRENT_USER;
	    else if (strcmp(rkey, "HKEY_LOCAL_MACHINE")==0)
		hrkey = HKEY_LOCAL_MACHINE;
	    else if (strcmp(rkey, "HKEY_USERS")==0)
		hrkey = HKEY_USERS;
#endif
	    else
		return FALSE;
	    if (skey == (char *)NULL)
		return FALSE;
#ifdef __WIN32__
	    if (!is_win32s) {
		if (RegCreateKeyEx(hrkey, skey, 0, "", 0, KEY_ALL_ACCESS, 
		    NULL, &hkey, &dwResult)
		    != ERROR_SUCCESS)
		    return FALSE;
	    }
	    else
#endif
	    {
		if (RegCreateKey(hrkey, skey, &hkey) != ERROR_SUCCESS)
		    return FALSE;
	    }
	    strcpy(keyname, skey);
	}
#ifdef __WIN32__		
		/* Win16 doesn't have named values. */
		/* Default value will be deleted when key is deleted. */
	else if (line[0] == '@') {
	    /* default value */
	    RegDeleteValue(hkey, NULL);
	}
	else if (line[0] == '\042') {
	    /* named value */
	    name = strtok(line+1, "\042\r\n");
	    RegDeleteValue(hkey, name);
	}
#endif
    }
    /* close key */
    if (hkey != HKEY_CLASSES_ROOT)
	RegCloseKey(hkey);
    /* delete the key */
    if (strlen(keyname)) {
#ifdef __WIN32__
	if (!is_win32s)
	    RegOpenKeyEx(hrkey, NULL, 0, 0, &hkey);
	else
#endif
	    RegOpenKey(hrkey, NULL, &hkey);
	RegDeleteKey(hkey, keyname);
	RegCloseKey(hkey);
    }
    return TRUE;
}

int
registry_delete(char *filename)
{
long logindex;
char line[MAXSTR];
KEY *key;
FILE *rfile;
    rfile = fopen(filename, "r");
    if (rfile == (FILE *)NULL)
	return NOTDONE;
    /* scan log file */
    /* so we can remove keys in reverse order */
    logindex = 0;
    while (fgets(line, sizeof(line), rfile) != (char *)NULL) {
	KEY *key;
	if (line[0] == '[') {
	    if ((key = (KEY *)malloc(sizeof(KEY)))
		!= (KEY *)NULL) {
		key->previous = last_key;
		key->index = logindex;
		last_key = key;
	    }
	}
	logindex = ftell(rfile);
    }

    /* Remove keys */
    for (key = last_key; key != NULL; 
	key = key->previous) {
	if (key != last_key)
	    free(last_key);
	fseek(rfile, key->index, SEEK_SET);
	registry_delete_key(rfile, line);
	last_key = key;
    }
    free(last_key);

    fclose(rfile);
    return COMPLETED;
}

void
read_ini(void)
{
char gsview_version[64];
UINT gs_version;
char buf[MAXSTR];
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
	struct stat st;
	if (p && *p) {
	    strcpy(szIniFile, p); 
	    p = szIniFile + strlen(szIniFile) - 1;
	    if ((*p == '\\') || (*p == '/'))
		*p = '\0';
	    /* check if USERPROFILE contains a directory name */
	    if ( (stat(szIniFile, &st) == 0) && (st.st_mode & S_IFDIR) ) {
		strcat(szIniFile, "\\");
	    }
	    else {
		/* If we didn't succeed, use the Windows directory */
		szIniFile[0] = '\0';
	    }
	}
    }
#endif
   
    if (szIniFile[0] == '\0') {
	/* If we didn't succeed, use the Windows directory */
	GetWindowsDirectory(szIniFile, sizeof(szIniFile));
	strcat(szIniFile, "\\");
    }
    strcat(szIniFile, INIFILE);

    GetPrivateProfileString("Options", "Language", "en", 
	buf, sizeof(buf), szIniFile);
    if (strcmp(buf, "de") == 0)
	language = ID_GERMAN;
    else if (strcmp(buf, "fr") == 0)
	language = ID_FRENCH;

    GetPrivateProfileString("Options", "Version", GSVIEW_VERSION, 
	gsview_version, sizeof(gsview_version), szIniFile);
    gs_version = GetPrivateProfileInt("Options", "GSversion", GS_REVISION, 
	szIniFile);
    
    LoadString(phInstance, language+IDS_PROG, buf, sizeof(buf));
    sprintf(title, buf, gsview_version, gs_version / 100, gs_version % 100);

    sprintf(szGSversion, "%d.%02d", gs_version/100, gs_version % 100); 
}

int
remove_inifile(void)
{
FILE *f;
    Sleep(500);
    if ( (f = fopen(szIniFile, "r")) == (FILE *)NULL)
	return NOTDONE;
    fclose(f);
    if (unlink(szIniFile))
	return FAIL;
    return COMPLETED;
}

void
do_message(void)
{
MSG msg;
    while (hDlgModeless && PeekMessage(&msg, (HWND)NULL, 0, 0, PM_REMOVE)) {
	if ((hDlgModeless == 0) || !IsDialogMessage(hDlgModeless, &msg)) {
	    TranslateMessage(&msg);
	    DispatchMessage(&msg);
	}
    }
}

void
set_dlg_text(HWND hwnd, int ctrlid, int id)
{
    char buf[MAXSTR];
    LoadString(phInstance, language+id, buf, sizeof(buf));
    SetDlgItemText(hwnd, ctrlid, buf);
}

int
fixup(int flag, HWND hwnd, int ctrlid, int nextid)
{
    if (flag == COMPLETED)
	SetDlgItemText(hwnd, ctrlid, "+");
    if (flag == NOTDONE)
	SetDlgItemText(hwnd, ctrlid, " ");
    if (flag == FAIL) {
	SetDlgItemText(hwnd, ctrlid, "X");
	set_dlg_text(hwnd, IDC_T6, IDS_T7);
	PostMessage(hwnd, WM_COMMAND, IDC_DONE, 0L);
    }
    else
	PostMessage(hwnd, WM_COMMAND, nextid, 0L);
    return flag;
}


#ifdef __BORLANDC__
#pragma argsused
#endif
BOOL CALLBACK _export
RemoveDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
switch(message) {
case WM_INITDIALOG:
set_dlg_text(hDlg, IDOK, IDS_OK);
	    set_dlg_text(hDlg, IDCANCEL, IDS_CANCEL);
	    set_dlg_text(hDlg, IDC_PRESSOK, IDS_PRESSOK);
	    SetDlgItemText(hDlg, IDC_PROG, title);
	    set_dlg_text(hDlg, IDC_T1, IDS_T1);
	    set_dlg_text(hDlg, IDC_T2, IDS_T2);
	    set_dlg_text(hDlg, IDC_T3, IDS_T3);
	    set_dlg_text(hDlg, IDC_T4, IDS_T4);
	    set_dlg_text(hDlg, IDC_T5, IDS_T5);
	    {
	    char buf[MAXSTR];
	    LoadString(phInstance, language+IDS_CAPTION, buf, sizeof(buf));
	    SetWindowText(hDlg, buf);
	    }
	    return TRUE;
	case WM_COMMAND:
	    switch(LOWORD(wParam)) {
		case IDC_T1:
		    SetDlgItemText(hDlg, IDC_Y1, "-");
		    do_message();
		    fixup(remove_registry(), hDlg, IDC_Y1, IDC_T2); 
		    return TRUE;
		case IDC_T2:
		    SetDlgItemText(hDlg, IDC_Y2, "-");
		    do_message();
		    fixup(restore_registry(), hDlg, IDC_Y2, IDC_T3); 
		    return TRUE;
		case IDC_T3:
		    SetDlgItemText(hDlg, IDC_Y3, "-");
		    do_message();
		    fixup(remove_progman(), hDlg, IDC_Y3, IDC_T4); 
		    return TRUE;
		case IDC_T4:
		    SetDlgItemText(hDlg, IDC_Y4, "-");
		    do_message();
		    fixup(remove_inifile(), hDlg, IDC_Y4, IDC_T5); 
		    return TRUE;
		case IDC_T5:
		    SetDlgItemText(hDlg, IDC_Y5, "-");
		    do_message();
		    if (fixup(remove_files(), hDlg, IDC_Y5, IDC_DONE)) 
			set_dlg_text(hDlg, IDC_T6, IDS_T6);
		    return TRUE;
		case IDC_DONE:
#ifdef __WIN32__
		    /* delete registry entries for uninstall */
		    if (is_win4) {
			HKEY hkey;
			char buf[MAXSTR];
		        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
				UNINSTALLKEY, 0, KEY_ALL_ACCESS, &hkey) 
			    == ERROR_SUCCESS) {
			    LoadString(phInstance, language+IDS_TITLE,
				 buf, sizeof(buf));
			    RegDeleteKey(hkey, buf);
			    RegCloseKey(hkey);
			}
		    }
#endif
		    EnableWindow(GetDlgItem(hDlg, IDCANCEL), TRUE);
		    EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
		    SetFocus(GetDlgItem(hDlg, IDCANCEL));
		    set_dlg_text(hDlg, IDCANCEL, IDS_EXIT);
		    return TRUE;
		case IDOK:
		    /* Start removal */
		    EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
		    PostMessage(hDlg, WM_COMMAND, IDC_T1, 0L);
		    EnableWindow(GetDlgItem(hDlg, IDC_T1), TRUE);
		    EnableWindow(GetDlgItem(hDlg, IDC_T2), TRUE);
		    EnableWindow(GetDlgItem(hDlg, IDC_T3), TRUE);
		    EnableWindow(GetDlgItem(hDlg, IDC_T4), TRUE);
		    EnableWindow(GetDlgItem(hDlg, IDC_T5), TRUE);
		    return TRUE;
		case IDCANCEL:
		    DestroyWindow(hDlg);
		    hDlgModeless = 0;
		    return TRUE;
	    }
	case WM_CLOSE:
	    DestroyWindow(hDlg);
	    hDlgModeless = 0;
	    return TRUE;
    }
    return FALSE;
}


int PASCAL 
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int cmdShow)
{
#ifdef __WIN32__
DWORD version = GetVersion();
#else
DLGPROC lpfnRemoveDlgProc;
#endif
MSG msg;

#ifdef __WIN32__
    if ( ((HIWORD(version) & 0x8000)!=0) && ((HIWORD(version) & 0x4000)==0) )
        is_win32s = TRUE;
    if (LOBYTE(LOWORD(version)) >= 4)
	is_win4 = TRUE;
#endif
    phInstance = hInstance;
    lstrcpy(path, lpszCmdLine);
    if (lstrlen(path) == 0) {
	char *p;
	GetModuleFileName(hInstance, path, sizeof(path));
	if ((p = strrchr(path,'\\')) != (char *)NULL)
	    *p = '\0';
	else
	    path[0] = '\0';
    }

    read_ini();

#ifdef __WIN32__
    hDlgModeless = CreateDialogParam(hInstance, 
	    MAKEINTRESOURCE(IDD_UNSET),
	    HWND_DESKTOP, RemoveDlgProc, (LPARAM)NULL);
#else
    lpfnRemoveDlgProc = (DLGPROC)MakeProcInstance((FARPROC)RemoveDlgProc, 
	    hInstance);
    hDlgModeless = CreateDialogParam(hInstance, 
	    MAKEINTRESOURCE(IDD_UNSET), 
	    HWND_DESKTOP, lpfnRemoveDlgProc, (LPARAM)NULL);
#endif

    SetWindowPos(hDlgModeless, HWND_TOP, 0, 0, 0, 0, 
	SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);

    while (hDlgModeless && GetMessage(&msg, (HWND)NULL, 0, 0)) {
	if ((hDlgModeless == 0) || !IsDialogMessage(hDlgModeless, &msg)) {
	    TranslateMessage(&msg);
	    DispatchMessage(&msg);
	}
    }

#ifndef __WIN32__
    FreeProcInstance((FARPROC)lpfnRemoveDlgProc);
#endif
    return 0;
}

