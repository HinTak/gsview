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

#include "setup.h"
#include "gvcrc.h"

#define BASEDIR "\\gs3.53"
#define UNZIPEXE "winunzip.exe"
#define GSVIEWZIP "gsview.zip"
#define GSINIZIP  "gs353ini.zip"
#define GSW32ZIP  "gs353w32.zip"
#define GSWINZIP  "gs353win.zip"
#define GSFNTZIP  "gs353fn1.zip"

BOOL CALLBACK _export GeneralDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK _export InputDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK _export ModelessDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

char workdir[MAXSTR];
char bootdrive[MAXSTR];
char sourcedir[MAXSTR];
char destdir[MAXSTR];
char unzipname[MAXSTR];
char winsetup[MAXSTR];
char includepath[MAXSTR];
HINSTANCE phInstance;
char get_string_answer[MAXSTR];
char szAppName[]="GSview Install";
char szIniName16[]="gsview.ini";
char szIniName32[]="gsview32.ini";
char error_message[MAXSTR];
char no_error[] = "";
int atm_present;
int atm_fonts;
char atm_dir[MAXSTR];
int use_atm_fonts;
int is_win32;

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
_chdir2(char *dirname)
{
#ifdef __WIN32__
	SetCurrentDirectory(dirname);
#else
	if (isalpha(dirname[0]) && (dirname[1]==':'))
		(void) setdisk(toupper(dirname[0])-'A');
	if (!((strlen(dirname)==2) && isalpha(dirname[0]) && (dirname[1]==':')))
		chdir(dirname);
#endif
	return 0;
}

int copyfile(char *dname, char *sname)
{
FILE *dfile, *sfile;
char *buffer;
int count;
#define COPY_BUF_SIZE 16384
    sfile = fopen(sname, "rb");
    if (sfile == (FILE *)NULL) {
	sprintf(error_message, "Can't open %s for reading", sname);
	return 1;
    }
    dfile = fopen(dname, "wb");
    if (dfile == (FILE *)NULL) {
	sprintf(error_message, "Can't open %s for writing", dname);
	fclose(sfile);
	return 1;
    }
    if ( (buffer = malloc(COPY_BUF_SIZE)) == (char *)NULL ) {
	fclose(sfile);
	fclose(dfile);
	sprintf(error_message, "Can't allocate memory for copy buffer");
	return 1;
    }

    while ( (count = fread(buffer, 1, COPY_BUF_SIZE, sfile)) != 0 )
	fwrite(buffer, 1, count, dfile);

    free(buffer);
    fclose(dfile);
    fclose(sfile);
    return 0;
}

int
intro(void)
{
int flag;
    /* Introduction */
    if (dialog(IDD_INTRO, GeneralDlgProc) != DID_OK) {
	strcpy(error_message, no_error);
	return 1;
    }

    /* Copyright */
    if (dialog(IDD_COPYRIGHT, GeneralDlgProc) != DID_OK) {
	strcpy(error_message, no_error);
	return 1;
    }

    return 0; /* success */
}

int
getdest(void)
{
int valid;
int i;
    valid = 0;
    while (!valid) {
        /* Destination directory */
        strcpy(get_string_answer, bootdrive);
        strcat(get_string_answer,"\\"); 
        if (dialog(IDD_DIR, InputDlgProc) != DID_OK) {
	    strcpy(error_message, no_error);
	    return 1;
	}
        strcpy(destdir, get_string_answer);
        if (_chdir2(destdir))
	    message_box("Directory does not exist.  Please enter a directory name that does exist.",
		 MB_MOVEABLE | MB_OK);
	else
	    valid = 1;
	_chdir2(workdir);
    }
    /* remove trailing \ from destination directory */
    i = strlen(destdir) - 1;
    if ( (i >= 0) && (destdir[i] == '\\') )
	destdir[i] = '\0';
    return 0;
}



int
cleanup(void)
{
    return 0;
}


int
unzip(char *filename, char *destination)
{
    /* start unzip session */  
    char fullname[256];
    char arg[256];
    FILE *f;
    int file_exists = 0;
    HMODULE hmodule;
    MSG msg;
#ifndef __WIN32__
    DLGPROC lpUnzipDlgProc;
#endif
    int abort = 0;
    HWND hDlgModeless;

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

    /* set up unzip arguments */
    sprintf(arg, "%s -o %s -d %s%s", unzipname, fullname, destination,
	(strlen(destination) == 2) ? "\\" : "");

    hmodule = (HMODULE)WinExec(arg, SW_SHOWNOACTIVATE);

    if (hmodule == (HMODULE)NULL) {
	sprintf(error_message, "Can't run %s", arg);
	return 1;
    }


#ifdef __WIN32__
    hDlgModeless = CreateDialogParam(phInstance, MAKEINTRESOURCE(IDD_UNZIP), HWND_DESKTOP, ModelessDlgProc, (LPARAM)NULL);
#else
    lpUnzipDlgProc = (DLGPROC)MakeProcInstance((FARPROC)ModelessDlgProc, phInstance);
    hDlgModeless = CreateDialogParam(phInstance, MAKEINTRESOURCE(IDD_UNZIP), HWND_DESKTOP, lpUnzipDlgProc, (LPARAM)NULL);
#endif
    while (IsWindow(hDlgModeless) && GetModuleUsage(hmodule)) {
        while (PeekMessage(&msg, (HWND)NULL, 0, 0, PM_REMOVE)) {
	    if ((hDlgModeless==0) || !IsDialogMessage(hDlgModeless, &msg)) {
	        TranslateMessage(&msg);
	        DispatchMessage(&msg);
	    }
	}
    }

    /* display cancel dialog */
    if (!IsWindow(hDlgModeless))
	abort = TRUE;
    else
	DestroyWindow(hDlgModeless);

#ifndef __WIN32__
    FreeProcInstance((FARPROC)lpUnzipDlgProc);
#endif

    if (abort) {
	strcpy(error_message, "Unzip cancelled");
	return 1;
    }
    return 0;
}

int
update_config(void)
{
FILE *infile, *outfile;
char inname[MAXSTR], outname[MAXSTR];
char line[1024];
char tempname[MAXSTR];
char buf[MAXSTR];
int replace = 0;
    
    if (getenv("TEMP"))
	return 0;	/* assume TEMP is in autoexec.bat */

    strcpy(inname, bootdrive);
    strcat(inname, "\\autoexec.bat");

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
	if (message_box(buf, MB_YESNO) != MBID_YES) {
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
	message_box(buf, MB_MOVEABLE | MB_OK);
    }

    return 0;
}

int
update_fontmap(void)
{
char fontmap[MAXSTR];
char fontmap_atm[MAXSTR];
char fontmap_old[MAXSTR];
char buf[MAXSTR];
FILE *f, *infile;
int backup = 1;
    if (atm_present) {
      if (dialog(IDD_ATM, GeneralDlgProc) != DID_OK)
	return 0;
    }
    else
	return 0;

    use_atm_fonts = TRUE;

    strcpy(fontmap, destdir);
    strcat(fontmap, BASEDIR);
    strcpy(fontmap_atm, fontmap);
    strcpy(fontmap_old, fontmap);
    strcat(fontmap, "\\Fontmap");
    if (atm_fonts > 13)
       strcat(fontmap_atm, "\\Fontmap.ATB");
    else
       strcat(fontmap_atm, "\\Fontmap.ATM");
    strcat(fontmap_old, "\\Fontmap.old");
    if ( (f = fopen(fontmap_old, "r")) != (FILE *)NULL)  {
	fclose(f);
	sprintf(buf, "File %s exists.  Overwrite?", fontmap_old);
	if (message_box(buf, MB_MOVEABLE | MB_YESNO) != MBID_YES) {
	    backup = 0;
	}
    }
    if (backup) {
	unlink(fontmap_old);
        if (rename(fontmap, fontmap_old)) {
	    sprintf(error_message, "Error renaming %s to %s", fontmap, fontmap_old);
	    return 1;
	}
    }

    infile = fopen(fontmap_atm, "r");
    if (infile == (FILE *)NULL) {
	sprintf(error_message, "Can't open %s for reading", fontmap_atm);
	return 1;
    }
    f = fopen(fontmap, "w");
    if (f == (FILE *)NULL) {
	sprintf(error_message, "Can't create %s for writing", fontmap);
	fclose(infile);
	return 1;
    }
    while (fgets(buf, sizeof(buf), infile))
	fputs(buf, f);
    fclose(infile);
    fclose(f);
    return 0;
}

int
update_ini(void)
{
char dest[MAXSTR];
char buf[MAXSTR];

    strcpy(dest, destdir);
    strcat(dest, BASEDIR);
    strcpy(buf, dest);
    strcat(buf, "\\gswin.exe");
    WritePrivateProfileString("Options", "GhostscriptExe", buf, szIniName16);
    strcpy(buf, dest);
    strcat(buf, "\\gswin32.exe");
    WritePrivateProfileString("Options", "GhostscriptExe", buf, szIniName32);

    strcpy(includepath, dest);
    strcat(includepath, ";");
    strcat(includepath, dest);
    strcat(includepath, "\\fonts");
    if (use_atm_fonts) {
        strcat(includepath, ";");
	strcat(includepath, atm_dir);
    }
    WritePrivateProfileString("Options", "GhostscriptInclude", includepath, szIniName16);
    WritePrivateProfileString("Options", "GhostscriptInclude", includepath, szIniName32);
    WritePrivateProfileString("Options", "GhostscriptVersion", "351", szIniName16);
    WritePrivateProfileString("Options", "GhostscriptVersion", "351", szIniName32);
    WritePrivateProfileString("Options", "Version", GSVIEW_VERSION, szIniName16);
    WritePrivateProfileString("Options", "Version", GSVIEW_VERSION, szIniName32);
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
char buf[MAXSTR];
DWORD idInst = 0L;
FARPROC lpDdeProc;
HSZ hszServName;
HSZ hszSysTopic;
HCONV hConv;
HDDEDATA hData;
char setup[MAXSTR+MAXSTR];
DWORD dwResult;

    lpDdeProc = MakeProcInstance((FARPROC)DdeCallback, phInstance);
    if (DdeInitialize(&idInst, (PFNCALLBACK)lpDdeProc, CBF_FAIL_POKES, 0L)) {
	FreeProcInstance(lpDdeProc);
	return 1;
    }
    hszServName = DdeCreateStringHandle(idInst, "PROGMAN", CP_WINANSI);
    hszSysTopic = DdeCreateStringHandle(idInst, "PROGMAN", CP_WINANSI);
    hConv = DdeConnect(idInst, hszServName, hszSysTopic, (PCONVCONTEXT)NULL);
    if (hConv == NULL) {
	message_box("Couldn't open DDE connection to Program Manger\n", 0);
	return 1;
    }

#define DDEEXECUTE(str)\
    DdeClientTransaction((LPBYTE)str, strlen(str)+1, hConv,\
	NULL, CF_TEXT, XTYP_EXECUTE, 2000, &dwResult)

    sprintf(setup, "[CreateGroup(\042GS Tools\042,gstools.grp)][ShowGroup(\042GS Tools\042,1)]");
    DDEEXECUTE(setup);
    sprintf(setup, "[ReplaceItem(\042GSview\042)]");
    DDEEXECUTE(setup);
    sprintf(setup, "[AddItem(\042%s\\gsview\\gsview.exe\042,\042GSview\042)]", destdir);
    DDEEXECUTE(setup);
    sprintf(setup, "[ReplaceItem(\042GSview 32\042)]");
    DDEEXECUTE(setup);
    sprintf(setup, "[AddItem(\042%s\\gsview\\gsview32.exe\042,\042GSview 32\042,\042%s\\gsview\\gsview.exe\042)]", destdir, destdir);
    DDEEXECUTE(setup);
    sprintf(setup, "[ReplaceItem(\042GSview README\042)]");
    DDEEXECUTE(setup);
    sprintf(setup, "[AddItem(\042notepad.exe %s\\gsview\\README.GV\042,\042GSview README\042)]", destdir, BASEDIR);
    DDEEXECUTE(setup);
    sprintf(setup, "[ReplaceItem(\042Ghostscript\042)]");
    DDEEXECUTE(setup);
    sprintf(setup, "[AddItem(\042%s%s\\gswin.exe -I%s\042,\042Ghostscript\042)]", destdir, BASEDIR, includepath);
    DDEEXECUTE(setup);
    sprintf(setup, "[ReplaceItem(\042Ghostscript 32\042)]");
    DDEEXECUTE(setup);
    sprintf(setup, "[AddItem(\042%s%s\\gswin32.exe -I%s\042,\042Ghostscript 32\042,\042%s%s\\gswin.exe\042)]", destdir, BASEDIR, includepath, destdir, BASEDIR);
    DDEEXECUTE(setup);
    sprintf(setup, "[ReplaceItem(\042Ghostscript README\042)]");
    DDEEXECUTE(setup);
    sprintf(setup, "[AddItem(\042notepad.exe %s%s\\README.\042,\042Ghostscript README\042)]", destdir, BASEDIR);
    DDEEXECUTE(setup);
#undef DDEXECUTE

    DdeDisconnect(hConv);
    DdeUninitialize(idInst);


    return 0;
}

int
update_registry(void)
{
HKEY hkey;
char buf[MAXSTR];
char *psname="psfile";
char *psvalue="PostScript";
LONG rc;

    rc = RegSetValue(HKEY_CLASSES_ROOT, psname, REG_SZ, psvalue, strlen(psvalue));
    if (rc == ERROR_SUCCESS)
	rc = RegSetValue(HKEY_CLASSES_ROOT, ".ps", REG_SZ, psname, strlen(psname));
    if (rc == ERROR_SUCCESS)
	rc = RegSetValue(HKEY_CLASSES_ROOT, ".eps", REG_SZ, psname, strlen(psname));

    if (rc == ERROR_SUCCESS)
	rc = RegCreateKey(HKEY_CLASSES_ROOT, "psfile\\shell\\open", &hkey);
    sprintf(buf, "%s\\gsview\\gsview%s.exe %%1", destdir, (is_win32 ? "32" : ""));
    if (rc == ERROR_SUCCESS)
	rc = RegSetValue(hkey, "command", REG_SZ, buf, strlen(buf));
    RegCloseKey(hkey);

    if (rc == ERROR_SUCCESS)
	rc = RegCreateKey(HKEY_CLASSES_ROOT, "psfile\\shell\\print", &hkey);
    sprintf(buf, "%s\\gsview\\gsview%s.exe /p %%1", destdir, (is_win32 ? "32" : ""));
    if (rc == ERROR_SUCCESS)
	rc = RegSetValue(hkey, "command", REG_SZ, buf, strlen(buf));
    RegCloseKey(hkey);

    if (rc != ERROR_SUCCESS) {
	strcpy(error_message, "Error while updating registry");
	return 1;
    }
    return 0;
}

int
install(void)
{
char buf[MAXSTR];
int rc;
char *p;
HMODULE hmodule;
DWORD version = GetVersion();

    /* find out if we are running under Windows 95 or NT */
    if ((LOBYTE(LOWORD(version))<<8) + HIBYTE(LOWORD(version)) > 0x30b)
	is_win32 = TRUE;

    /* get path to EXE */
    GetModuleFileName(phInstance, sourcedir, sizeof(sourcedir));
    if ((p = strrchr(sourcedir,'\\')) != (char *)NULL)
	p++;
    else
	p = sourcedir;
    *p = '\0';

    /* Inspect system, get boot drive */
    getcwd(workdir, sizeof(workdir));	/* remember the working directory */
    strcpy(bootdrive, "C:");

    /* check for active Adobe Type Manager */
    GetPrivateProfileString("Setup", "PFB_Dir", "", atm_dir, sizeof(atm_dir), "atm.ini");
    if (strlen(atm_dir) != 0) {
	atm_present = TRUE;
	atm_fonts = 13;    /* assume 13 base fonts */
	GetPrivateProfileString("fonts", "AvantGarde", "", buf, sizeof(buf), "atm.ini");
	if (strlen(buf) != 0)
	    atm_fonts = 39;	   /* assume 39 fonts - same as LaserWriter Plus */
	GetPrivateProfileString("fonts", "Tekton", "", buf, sizeof(buf), "atm.ini");
	if (strlen(buf) != 0)
	    atm_fonts = 65;	   /* assume Adobe Type Basics */
    }

    
    if (!rc)
        rc = intro();	/* display intro dialog boxes */

    if (!rc)
	rc = getdest();	/* get destination directory */


    if (!rc) {
	/* copy unzip program for faster loading */
	strcpy(unzipname, destdir);
	strcat(unzipname, "\\gsview");
	mkdir(unzipname);
	strcat(unzipname, "\\");
	strcat(unzipname, UNZIPEXE);
	strcpy(buf, sourcedir);
	strcat(buf, UNZIPEXE);
	rc = copyfile(unzipname, buf);
    }

    /* unzip GSview and Ghostscript */
    if (!rc) {
	rc = unzip(GSVIEWZIP, destdir);
    }
    if (!rc)
	rc = unzip(GSINIZIP, destdir);
    if (!rc)
	rc = unzip(GSW32ZIP, destdir);
    if (!rc)
	rc = unzip(GSWINZIP, destdir);
    if (!rc) {
	strcpy(buf, destdir);
	strcat(buf, BASEDIR);
	rc = unzip(GSFNTZIP, buf);
    }

    /* remove unneeded unzip */
    unlink(unzipname);

    if (!rc) {
	rc = update_config();
    }

    if (!rc) {
	rc = update_fontmap();
    }

    if (!rc) {
	rc = update_ini();
    }

    if (!rc) {
	rc = create_object();
    }

    if (!rc)
	rc = update_registry();

    if (!rc) {
	sprintf(buf, "Installation successful.\r\
A Program Manager group named \042GS Tools\042 has been created.\r\
File Manager associations have been created for .ps and .eps files.");
	message_box(buf, MB_MOVEABLE | MB_OK);
    }
    return rc;
}


#pragma argsused	/* ignore warning for next function */
int PASCAL 
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int cmdShow)
{
    int rc = 0;
    LPSTR p;
    /* copy the hInstance into a variable so it can be used */
    phInstance = hInstance;

    rc = install();

    if (rc) {
        char buf[256];
	sprintf(buf, "Installation aborted\012%s", error_message);
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

