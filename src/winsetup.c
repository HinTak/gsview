/* Copyright (C) 1993-1996, Russell Lang.  All rights reserved.;
  
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
/* rjl 1996-01-02 */
/* rjl 1996-06-21 */

#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <ddeml.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dir.h>
#include <ctype.h>
#include <io.h>

#define MAXSTR 256

#include "gvcver.h"
#include "gvcbeta.h"
#include "gvcrc.h"
#include "setup.h"
#include "gvclang.h"

int unzip(char *zipname);
int load_unzip(LPSTR lpszDllName, HINSTANCE hInstance, HWND hmain, HWND hlist);
int free_unzip(void);
HWND gs_showmess_modeless(void);
void gs_showmess_destroy(void);
void gs_addmess(char *str);
void gs_addmess_update(HWND hwnd);

BOOL CALLBACK _export GeneralDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK _export InputDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK _export ModelessDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

char workdir[MAXSTR];
char bootdrive[MAXSTR];
char sourcedir[MAXSTR];
char destdir[MAXSTR];
char unzipname[MAXSTR];
char winsetup[MAXSTR];
char gsviewbase[MAXSTR];
HINSTANCE phInstance;
char get_string_answer[MAXSTR];
char szAppName[]="GSview Install";
#ifdef __WIN32__
char szUnzipDll[] = "wizunz32.dll";
char szIniName[]="gsview32.ini";
#else
char szUnzipDll[] = "wizunz16.dll";
char szIniName[]="gsview16.ini";
#endif
char error_message[MAXSTR];
char no_error[] = "";
int is_win32s;
int is_win4;
HWND hwndmess;
int batch;

#define DID_OK IDOK
#define DID_CANCEL IDCANCEL
#define MBID_YES IDYES
#define MBID_NO IDNO
#define MB_MOVEABLE 0

int
dialog(int resource, DLGPROC dlgproc) 
{
int flag;
#ifndef __WIN32__
DLGPROC lpProcDlg;
#endif
#ifdef __WIN32__
    flag = DialogBoxParam( phInstance, MAKEINTRESOURCE(resource), HWND_DESKTOP, dlgproc, (LPARAM)NULL);
#else
    lpProcDlg = (DLGPROC)MakeProcInstance((FARPROC)dlgproc, phInstance);
    flag = DialogBoxParam( phInstance, MAKEINTRESOURCE(resource), HWND_DESKTOP, lpProcDlg, (LPARAM)NULL);
    FreeProcInstance((FARPROC)lpProcDlg);
#endif
    return flag;
}

int message_box(char *str, int icon)
{
    return MessageBox(HWND_DESKTOP, str, szAppName, icon);
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
	return LoadString(phInstance, id, str, len);
}

/* INCLUDE COMMON CODE */
#include "setup.c"




int
cleanup(void)
{
    return 0;
}


int
unzip_to_dir(char *filename, char *destination)
{
    /* start unzip session */  
    char fullname[256];
    FILE *f;
    int file_exists = 0;
    char cwd[256];
    int rc;

    /* prompt for disk to be installed */
    strcpy(fullname, sourcedir);
    strcat(fullname, filename);
    while (!file_exists) {
        if ( (f = fopen(fullname, "r")) == (FILE *)NULL ) {
	    char buf[256];
	    sprintf(buf, "Insert disk containing %s", fullname);
	    strcpy(get_string_answer, fullname);
	    if (dialog(IDD_FILE, InputDlgProc) != DID_OK) {
		strcpy(error_message, no_error);
		return 1;
	    }
	    strcpy(fullname, get_string_answer);
	}
	else {
	    file_exists = TRUE;
	    fclose(f);
	}
    }

    getcwd(cwd, sizeof(cwd));
    gs_chdir(destination);
    gs_addmess("Unzipping ");
    gs_addmess(fullname);
    gs_addmess("\n");
    rc = unzip(fullname);
    gs_chdir(cwd);

    if (!rc) {
        if (!IsWindow(hwndmess)) {
	    strcpy(error_message, "Unzip cancelled");
	    return -1;
	}
    }
    return rc;
}


int
update_config(void)
{
FILE *infile, *outfile;
char inname[MAXSTR], outname[MAXSTR];
char line[1024];
char tempname[MAXSTR];
char buf[MAXSTR];
int replace;
    
    if (getenv("TEMP"))
	return 0;	/* assume TEMP is in autoexec.bat */

    strcpy(inname, bootdrive);
    strcat(inname, "\\autoexec.bat");

    if (batch)
	replace = TRUE;
    else 
        replace = (dialog(IDD_CONFIG, GeneralDlgProc) == DID_OK);

    strcpy(tempname, bootdrive);
    strcat(tempname, "\\GSXXXXXX");
    if (mktemp(tempname) == (char *)NULL) {
		strcpy(error_message, "Can't get temporary filename");
		return 1;
    }

    if ( (infile = fopen(inname, "r")) == (FILE *)NULL) {
	sprintf(error_message, "Can't open %s for reading", inname);
	return 1;
    }
    if ( (outfile = fopen(tempname, "w")) == (FILE *)NULL)  {
	sprintf(error_message, "Can't create %s for writing", outname);
	return 1;
    }
    while (fgets(line, sizeof(line), infile)) {
	fputs(line, outfile);
    }
    sprintf(line, "SET TEMP=%s\\\n", bootdrive);
    fputs(line, outfile);

    fclose(outfile);
    fclose(infile);

    strcpy(outname, bootdrive);
    strcat(outname, "\\autoexec.gs");
    if ( (outfile = fopen(outname, "r")) != (FILE *)NULL)  {
	fclose(outfile);
	sprintf(buf, "File %s exists.  Overwrite?", outname);
	if (!batch && (message_box(buf, MB_YESNO) != MBID_YES)) {
	    return 0;
	}
	unlink(outname);
    }
    if (replace) {
	/* modify autoexec.bat */
	if (rename(inname, outname)) {
	    sprintf(error_message, "Error renaming %s to %s", inname, outname);
	    return 1;
	}
	if (rename(tempname, inname)) {
	    sprintf(error_message, "Error renaming %s to %s", tempname, inname);
	    return 1;
	}
    }
    else {
	if (rename(tempname, outname)) {
	    sprintf(error_message, "Error renaming %s to %s", tempname, outname);
	    return 1;
	}
	sprintf(buf, "Changes were saved in %s", outname);
	if (!batch)
	    message_box(buf, MB_MOVEABLE | MB_OK);
    }

    return 0;
}

int
update_ini(char *ininame)
{
    WritePrivateProfileString("Options", "Configured", "0", ininame);
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
create_object(void)
{
DWORD idInst = 0L;
FARPROC lpDdeProc;
HSZ hszServName;
HSZ hszSysTopic;
HCONV hConv;
char setup[MAXSTR+MAXSTR];
DWORD dwResult;

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
	message_box("Couldn't open DDE connection to Program Manager\n", 0);
	return 1;
    }

#define DDEEXECUTE(str)\
    DdeClientTransaction((LPBYTE)str, strlen(str)+1, hConv,\
	NULL, CF_TEXT, XTYP_EXECUTE, 2000, &dwResult)

    sprintf(setup, "[CreateGroup(\042GS Tools\042,gstools.grp)][ShowGroup(\042GS Tools\042,1)]");
    DDEEXECUTE(setup);
    sprintf(setup, "[ReplaceItem(\042GSview\042)]");
    DDEEXECUTE(setup);
    if (!is_win4)
       sprintf(setup, "[AddItem(\042%s\\%s\\%s\042,\042GSview\042, \042%s\\%s\\gsview32.ico\042)]", 
	  destdir, gsviewbase, GSVIEW_EXENAME, destdir, gsviewbase);
    else
       sprintf(setup, "[AddItem(\042%s\\%s\\%s\042,\042GSview\042)]", 
	  destdir, gsviewbase, GSVIEW_EXENAME);
    DDEEXECUTE(setup);

/* Win3.1 documentation says you must put quotes around names */
/* with embedded spaces. */
/* In Win95, it appears you must put quotes around the EXE name */
/* and options separately */

    sprintf(setup, "[ReplaceItem(\042GSview README\042)]");
    DDEEXECUTE(setup);
    if (!is_win4)
	sprintf(setup, "[AddItem(\042notepad.exe %s\\%s\\README.TXT\042,\042GSview README\042)]", 
	    destdir, gsviewbase);
    else
	sprintf(setup, "[AddItem(\042notepad.exe\042 \042%s\\%s\\README.TXT\042,\042GSview README\042,\042notepad.exe\042,1)]", 
	    destdir, gsviewbase);
    DDEEXECUTE(setup);

    sprintf(setup, "[ReplaceItem(\042Ghostscript\042)]");
    DDEEXECUTE(setup);
    if (!is_win4)
        sprintf(setup, "[AddItem(\042%s\\%s\\%s -I%s\\%s;%s\\%s\\fonts\042,\042Ghostscript\042, \042%s\\%s\\gstext.ico\042)]", 
	    destdir, GS_BASEDIR, GS_EXENAME, destdir, GS_BASEDIR, destdir, GS_BASEDIR,  destdir, GS_BASEDIR);
    else
        sprintf(setup, "[AddItem(\042%s\\%s\\%s\042 \042-I%s\\%s;%s\\%s\\fonts\042,\042Ghostscript\042)]", 
	    destdir, GS_BASEDIR, GS_EXENAME, destdir, GS_BASEDIR, destdir, GS_BASEDIR);
    DDEEXECUTE(setup);

    sprintf(setup, "[ReplaceItem(\042Ghostscript README\042)]");
    DDEEXECUTE(setup);
    if (!is_win4)
        sprintf(setup, "[AddItem(\042notepad.exe %s\\%s\\README.\042,\042Ghostscript README\042)]", 
	     destdir, GS_BASEDIR);
    else
        sprintf(setup, "[AddItem(\042notepad.exe\042 \042%s\\%s\\README.\042,\042Ghostscript README\042, \042notepad.exe\042,1)]", 
	     destdir, GS_BASEDIR);
    DDEEXECUTE(setup);
#undef DDEXECUTE

    DdeDisconnect(hConv);
    DdeUninitialize(idInst);


    return 0;
}


int
install(void)
{
char buf[MAXSTR];
int rc = 0;
char *p;
DWORD version = GetVersion();

    /* find out if we are running under Win32s */
    /* Win32s */
    if ( ((HIWORD(version) & 0x8000)!=0) && ((HIWORD(version) & 0x4000)==0) )
	    is_win32s = TRUE;
    /* Windows 4.0 */
    if (LOBYTE(LOWORD(version)) >= 4)
	is_win4 = TRUE;

    /* get path to EXE */
    GetModuleFileName(phInstance, sourcedir, sizeof(sourcedir));
    if ((p = strrchr(sourcedir,'\\')) != (char *)NULL)
	p++;
    else
	p = sourcedir;
    *p = '\0';

    /* Inspect system, get boot drive */
    getcwd(workdir, sizeof(workdir));	/* remember the working directory */
    strcpy(bootdrive, "c:");

    if (!rc)
        rc = intro();	/* display intro dialog boxes */

    if (!rc)
	rc = getdest();	/* get destination directory */


    if (!rc) {
	/* copy unzip program for faster loading */
	strcpy(unzipname, destdir);
	strcat(unzipname, "\\");
	strcat(unzipname, gsviewbase);
	mkdir(unzipname);
	strcat(unzipname, "\\");
	strcat(unzipname, szUnzipDll);
	strcpy(buf, sourcedir);
	strcat(buf, szUnzipDll);
	rc = copyfile(unzipname, buf);
    }

    if (rc)
	return rc;

    /* unzip GSview and Ghostscript */
    if (!rc) {
	hwndmess = gs_showmess_modeless();
	load_unzip(unzipname, phInstance, hwndmess, (HWND)NULL);
	strcpy(buf, destdir);
	if (strlen(buf) == 2)
	    strcat(buf, "\\");	/* is root directory */
	if (!rc) {
	    char buf2[MAXSTR];
	    strcpy(buf2, buf);
	    if (strlen(buf2) && (buf2[strlen(buf2)-1] != '\\'))
	        strcat(buf2, "\\");
	    strcat(buf2, gsviewbase);
	    mkdir(buf2);
	    rc = unzip_to_dir(GSVIEW_ZIP, buf2);
	}
	if (!rc) {
	    int skip_gs = FALSE;
	    if (already_installed()) {
		char buf3[MAXSTR];
		char buf4[MAXSTR];
		load_string(IDS_SKIPGSINSTALL, buf3, sizeof(buf3));
		sprintf(buf4, buf3, GS_VERSION);
		if (message_box(buf4, MB_YESNO) == MBID_YES)
		    skip_gs = TRUE;
	    }
		
	    if (!skip_gs) {
		if (!rc)
		    rc = unzip_to_dir(GS_INIZIP, buf);
		if (!rc)
#ifdef __WIN32__
		    rc = unzip_to_dir(GS_W32ZIP, buf);
#else
		    rc = unzip_to_dir(GS_W16ZIP, buf);
#endif
		if (!rc) {
		    if (strlen(buf) && (buf[strlen(buf)-1] != '\\'))
			strcat(buf, "\\");
		    strcat(buf, GS_BASEDIR);
		    rc = unzip_to_dir(GS_FN1ZIP, buf);
		}
	    }
	}
	gs_chdir(workdir);
	if (rc) {
	    MSG msg;
	    /* wait for user to read error message */
	    gs_addmess("unzip error");
	    gs_addmess_update(hwndmess);
	    while (hwndmess && IsWindow(hwndmess) && 
		GetMessage(&msg, (HWND)NULL, 0, 0)) {
		    TranslateMessage(&msg);
		    DispatchMessage(&msg);
	    }
	}
	gs_showmess_destroy();
	free_unzip();
    }

    /* remove unneeded unzip DLL */
    unlink(unzipname);

    if (!rc)
	rc = update_config();

    if (!rc)
	rc = update_ini(szIniName);

    if (!rc)
	rc = create_object();

    if (!rc) {
        load_string(IDS_SETUPOK, buf, sizeof(buf));
	if (!batch)
	    message_box(buf, MB_MOVEABLE | MB_OK);
    }
    return rc;
}


#pragma argsused	/* ignore warning for next function */
int PASCAL 
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int cmdShow)
{
    int rc;
    /* copy the hInstance into a variable so it can be used */
    phInstance = hInstance;
    if (lpszCmdLine[0] != '\0') {
	LPSTR d, s;
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
    load_string(IDS_GSVIEWBASE, gsviewbase, sizeof(gsviewbase));

    if (beta_warn())
	return 1;

    rc = install();

    if (rc) {
	char mess[256];
        char buf[256];
	load_string(IDS_INSTALLABORT, mess, sizeof(mess));
	sprintf(buf, mess, error_message);
	message_box(buf, MB_MOVEABLE | MB_OK);
    }

    rc = cleanup();
    return rc;
}


#pragma argsused	/* ignore warning for next function */
/* General Dialog Box */
BOOL CALLBACK _export
GeneralDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
        case WM_INITDIALOG:
            return( TRUE);
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case IDOK:
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

#pragma argsused	/* ignore warning for next function */
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


#pragma argsused	/* ignore warning for next function */
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
		    DestroyWindow(hDlg);
                    return(TRUE);
                case IDCANCEL:
		    DestroyWindow(hDlg);
                    return(TRUE);
                default:
                    return(FALSE);
            }
	case WM_CLOSE:
	    DestroyWindow(hDlg);
	    return TRUE;
    }
    return FALSE;
}

