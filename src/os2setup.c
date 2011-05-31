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

/* os2setup.c */
/* OS/2 installation program for GSview and Ghostscript */

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_DOSERRORS
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>

#define MAXSTR 256
#define GVFAR
#define IDYES MBID_YES

#include "gvcver.h"
#include "gvcbeta.h"
#include "gvcprf.h"
#include "gvcrc.h"
#include "setup.h"
#include "gvclang.h"


#define EMXZIP    "emxrt.zip"
#define UNZIPEXE  "os2unzip.exe"

MRESULT EXPENTRY GeneralDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY InputDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

char workdir[MAXSTR];
char bootdrive[MAXSTR];
char destdir[MAXSTR];
char gsviewbase[MAXSTR];
char unzipname[MAXSTR];
HAB hab;
HWND hwnd_dlg;
char get_string_answer[MAXSTR];
char szAppName[]="GSview Install";
char szExePath[MAXSTR];
char szUnzipDll[]="unzip2.dll";
char error_message[MAXSTR];
char no_error[] = "";
int emx;
int use_os2_fonts;
int batch;

HQUEUE		term_queue;	/* termination queue for child sessions */
char 		term_queue_name[MAXSTR];
TID 		term_tid;
VOID APIENTRY	term_thread(ULONG unused);

int
dialog(int resource, PFNWP dlgproc)
{
int flag;
    flag = WinDlgBox(HWND_DESKTOP, HWND_DESKTOP, dlgproc, 0, resource, 0);

    return flag;
}

int message_box(char *str, int icon)
{
    return WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,
                 str, szAppName, 0, icon);
}


int 
gs_chdir(char *dirname)
{
#ifdef __BORLANDC__
	if (isalpha(dirname[0]) && (dirname[1]==':'))
		(void) setdisk(toupper(dirname[0])-'A');
	if (!((strlen(dirname)==2) && isalpha(dirname[0]) && (dirname[1]==':')))
		chdir(dirname);
	return TRUE;
#else
#ifdef __IBMC__
	if (isalpha(dirname[0]) && (dirname[1]==':'))
	    if (_chdrive(toupper(dirname[0])-'A'+1))
		return -1;
	if (!((strlen(dirname)==2) && isalpha(dirname[0]) && (dirname[1]==':')))
	    return _chdir(dirname);
#else
	if (isalpha(dirname[0]) && (dirname[1]==':'))
	    if (_chdrive(dirname[0]))
		return -1;
	return _chdir2(dirname);
#endif
#endif
}

int
load_string(int id, char *str, int len)
{
	return WinLoadString(hab, 0, id, len, str);
}


/* INCLUDE COMMON CODE */
#include "setup.c"



int
init(void)
{
    char buf[256];
    char *tail, *env;
    APIRET rc = 0;

    PTIB pptib;
    PPIB pppib;
    HMODULE hmodule;

    /* get path to EXE */
    if ( (rc = DosGetInfoBlocks(&pptib, &pppib)) != 0 ) {
	sprintf(error_message, "DosGetInfoBlocks rc=%d", rc);
	return rc;
    }

    /* get path to EXE */
    if ( (rc = DosQueryModuleName(pppib->pib_hmte, sizeof(szExePath), szExePath)) != 0 ) {
	sprintf(error_message, "DosQueryModuleName rc=%d", rc);
	return rc;
    }
    if ((tail = strrchr(szExePath,'\\')) != (PCHAR)NULL) {
	tail++;
	*tail = '\0';
    }

    /* create termination queue */
    sprintf(term_queue_name, "\\QUEUES\\TERM_GSINSTALL_%u", pppib->pib_ulpid);
    if ( (rc = DosCreateQueue(&term_queue, QUE_FIFO, term_queue_name)) != 0 ) {
	sprintf(error_message, "DosCreateQueue rc=%d", rc);
	return rc;
    }
    /* start termination queue thread */
    if ( (rc = DosCreateThread(&term_tid, term_thread, 0, 0, 8192)) != 0 ) {
	sprintf(error_message, "DosCreateThread rc=%d", rc);
        return rc;
    }

    /* Inspect system, get boot drive */
    getcwd(workdir, sizeof(workdir));	/* remember the working directory */
    strcpy(bootdrive, getenv("USER_INI"));
    bootdrive[2] = '\0';
    if ( !(rc = DosLoadModule(buf, sizeof(buf), "EMX", &hmodule)) ) {
	emx = TRUE;
	DosFreeModule(hmodule);
    }

    /* don't bother checking for OS/2 fonts */
    /* since all versions of OS/2 should have 13 base fonts */
    
    return 0;
}

int
cleanup(void)
{
    /* close queue to make termination thread exit */
    if (term_queue)
        DosCloseQueue(term_queue);
    if (term_tid)
        DosKillThread(term_tid);
    return 0;
}

int
eajoin(char *filename, char *eaname)
{
    STARTDATA sdata;
    APIRET rc;
    PTIB pptib;
    PPIB pppib;
    ULONG session_id; 
    PID process_id;
    char *command = "eautil.exe";
    char arg[256];

    sprintf(arg, "%s %s /j", filename, eaname);

    if (DosGetInfoBlocks(&pptib, &pppib)) {
	sprintf(error_message, "DosGetInfoBlocks rc=%d", rc);
	return 1;
    }

    /* because new program is a different EXE type, 
     * we must use start session not DosExecPgm() */
    sdata.Length = sizeof(sdata);
    sdata.Related = SSF_RELATED_CHILD;	/* to be a child  */
    sdata.FgBg = SSF_FGBG_BACK;		/* start in background */
    sdata.TraceOpt = 0;
    sdata.PgmTitle = command;
    sdata.PgmName = command;
    sdata.PgmInputs = arg;
    sdata.TermQ = term_queue_name;
    sdata.Environment = pppib->pib_pchenv;	/* use Parent's environment */
    /* with pipe needs to inherit redirected stdin */
    sdata.InheritOpt = 0;
    sdata.SessionType = SSF_TYPE_DEFAULT;		/* default is text */
    sdata.IconFile = NULL;
    sdata.PgmHandle = 0;
    sdata.PgmControl = 0;
    sdata.InitXPos = 0;
    sdata.InitYPos = 0;
    sdata.InitXSize = 0;
    sdata.InitYSize = 0;
    sdata.ObjectBuffer = NULL;
    sdata.ObjectBuffLen = 0;

    if ( (rc = DosStartSession(&sdata, &session_id, &process_id)) != 0) {
	if (rc == 2)
	    sprintf(error_message, "Can't run %s", command);
	else 
	    sprintf(error_message, "DosStartSession rc=%d", rc);
	return rc;
    }

    if (dialog(IDD_EAUTIL, GeneralDlgProc) != DID_OK) {
	strcpy(error_message, "eajoin cancelled");
	return 1;
    }
    /* cancel dialog will be sent a DID_OK by termination queue thread */
    return 0;
}


int
unzip(char *filename, char *destination)
{
    /* start unzip session */  
    STARTDATA sdata;
    APIRET rc;
    char fullname[256];
    char arg[256];
    PTIB pptib;
    PPIB pppib;
    ULONG session_id; 
    PID process_id;
    FILE *f;
    int file_exists = 0;

    /* prompt for disk to be installed */
    strcpy(fullname, szExePath);
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

    if (DosGetInfoBlocks(&pptib, &pppib)) {
	sprintf(error_message, "DosGetInfoBlocks rc=%d", rc);
	return 1;
    }

    /* set up unzip arguments */
    sprintf(arg, "-o %s -d %s%s", fullname, destination,
	(strlen(destination) == 2) ? "\\" : "");

    /* because new program is a different EXE type, 
     * we must use start session not DosExecPgm() */
    sdata.Length = sizeof(sdata);
    sdata.Related = SSF_RELATED_CHILD;	/* to be a child  */
    sdata.FgBg = SSF_FGBG_BACK;		/* start in background */
    sdata.TraceOpt = 0;
    sdata.PgmTitle = unzipname;
    sdata.PgmName = unzipname;
    sdata.PgmInputs = arg;
    sdata.TermQ = term_queue_name;
    sdata.Environment = pppib->pib_pchenv;	/* use Parent's environment */
    /* with pipe needs to inherit redirected stdin */
    sdata.InheritOpt = 0;
    sdata.SessionType = SSF_TYPE_DEFAULT;		/* default is text */
    sdata.IconFile = NULL;
    sdata.PgmHandle = 0;
    sdata.PgmControl = 0;
    sdata.InitXPos = 0;
    sdata.InitYPos = 0;
    sdata.InitXSize = 0;
    sdata.InitYSize = 0;
    sdata.ObjectBuffer = NULL;
    sdata.ObjectBuffLen = 0;

    if ( (rc = DosStartSession(&sdata, &session_id, &process_id)) != 0) {
	if (rc == 2)
	    sprintf(error_message, "Can't run %s", unzipname);
	else 
	    sprintf(error_message, "DosStartSession rc=%d", rc);
	return rc;
    }

    
    /* display cancel dialog */
    if (dialog(IDD_UNZIP, GeneralDlgProc) != DID_OK) {
	strcpy(error_message, "Unzip cancelled");
	return 1;
    }
    /* cancel dialog will be sent a DID_OK by termination queue thread */
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
int got_temp = 0;
int changed = 0;
int replace = 0;
    
    strcpy(inname, bootdrive);
    strcat(inname, "\\config.sys");

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
        if (strncmp(line, "LIBPATH=", 8) == 0) {
	    if (!emx) {
		sprintf(line+strlen(line)-1, ";%s\\emx\\dll\n", bootdrive);
		changed = TRUE;
	    }
	}
        if (strncmp(line, "SET PATH=", 9) == 0) {
	    if (!emx) {
		sprintf(line+strlen(line)-1, ";%s\\emx\\bin\n", bootdrive);
		changed = TRUE;
	    }
	}
	if (strncmp(line, "SET TEMP=", 9) == 0) {
	    got_temp = TRUE;
	}
	fputs(line, outfile);
    }
    if (!got_temp) {
	sprintf(line, "SET TEMP=%s\\\n", bootdrive);
	fputs(line, outfile);
	changed = TRUE;
    }

    fclose(outfile);
    fclose(infile);

    if (changed) {
	if (batch)
	    replace = TRUE;
	else 
	    replace = (dialog(IDD_CONFIG, GeneralDlgProc) == DID_OK);
	strcpy(outname, bootdrive);
	strcat(outname, "\\config.gs");
	if ( (outfile = fopen(outname, "r")) != (FILE *)NULL)  {
	    fclose(outfile);
	    sprintf(buf, "File %s exists.  Overwrite?", outname);
	    if (!batch && (message_box(buf, MB_MOVEABLE | MB_YESNO) != MBID_YES)) {
		strcpy(error_message, no_error);
		return 1;
	    }
	    unlink(outname);
	}
	if (replace) {
	    /* modify config.sys */
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
	    sprintf(buf, "Config.sys unchanged.  Suggested changes were saved in %s", outname);
	    if (!batch)
	        message_box(buf, MB_MOVEABLE | MB_OK);
	}
    }
    else {
	unlink(tempname);
    }

    return 0;
}

int
update_ini(char *ininame)
{
PROFILE *prf;
    prf = profile_open(ininame);
    if (prf == (PROFILE *)NULL) {
	sprintf(error_message, "Can't open %s", ininame);
	return 1;
    }
    profile_write_string(prf, "Options", "Configured", "0");
    profile_close(prf);
    return 0;
}

int
create_object(void)
{
char buf[MAXSTR];
char setup[MAXSTR];
APIRET rc;
    strcpy(buf, destdir);
    strcat(buf, "\\");
    strcat(buf, gsviewbase);
    strcat(buf, "\\gvpm.exe");
    sprintf(setup, "EXENAME=%s;ASSOCFILTER=*.ps,*.eps,*.pdf", buf);
    rc = !WinCreateObject("WPProgram", "GSview", setup, "<WP_DESKTOP>",
	CO_REPLACEIFEXISTS);
    if (rc) {
	sprintf(error_message, "Couldn't create desktop program object");
    }
    return rc;
}


int
install(void)
{
char buf[MAXSTR];
int rc;
    rc = init();	/* inspect system */
    
    if (!rc)
        rc = intro();	/* display intro dialog boxes */

    if (!rc)
	rc = getdest();	/* get destination directory */

    if (!rc) {
	/* copy unzip program for faster loading */
	strcpy(unzipname, destdir);
	strcat(unzipname, "\\gsview");
	mkdir(unzipname, 0);
	strcat(unzipname, "\\");
	strcat(unzipname, UNZIPEXE);
	strcpy(buf, szExePath);
	strcat(buf, UNZIPEXE);
	rc = copyfile(unzipname, buf);
    }

    /* unzip GSview and Ghostscript */
    if (!rc) {
	strcpy(buf, destdir);
	strcat(buf, "\\");
	strcat(buf, gsviewbase);
	mkdir(buf, 0);
	rc = unzip(GSVIEW_ZIP, buf);
    }
    if (!rc) {
	char eabuf[MAXSTR];
	strcpy(buf, destdir);
	strcat(buf, "\\");
	strcat(buf, gsviewbase);
	strcat(buf, "\\");
	strcpy(eabuf, buf);
	strcat(buf,   "gvpm.exe");
	strcat(eabuf, "gvpm.eas");
	eajoin(buf, eabuf);	/* rejoin the EAs lost by DOS */
    }
    if (!rc) {
	/* install EMX if needed */
	sprintf(buf, "%s\\", bootdrive);
	if (!emx)
	    rc = unzip(EMXZIP, buf);
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
		rc = unzip(GS_INIZIP, destdir);
	    if (!rc)
		rc = unzip(GS_OS2ZIP, destdir);
	    if (!rc) {
		strcpy(buf, destdir);
		strcat(buf, "\\");
		strcat(buf, GS_BASEDIR);
		rc = unzip(GS_FN1ZIP, buf);
	    }
	}
    }

    /* remove unneeded unzip */
    unlink(unzipname);

    if (!rc) {
	rc = update_config();
    }

    if (!rc) {
	char szIniName[MAXSTR];
	strcpy(szIniName, bootdrive);
	strcat(szIniName, "\\os2\\gvpm.ini");
	rc = update_ini(szIniName);
    }

    if (!rc) {
	rc = create_object();
    }

    if (!rc) {
	if (!batch) {
            load_string(IDS_SETUPOK, buf, sizeof(buf));
	    message_box(buf, MB_MOVEABLE | MB_OK);
	}
    }
    return rc;
}

int
main(int argc, char *argv[])
{
    HMQ hand_mq;		/* message queue */
    QMSG q_mess;		/* queue message */
    APIRET rc = 0;

    hab = WinInitialize(0);	/* Get the Anchor Block */

    hand_mq = WinCreateMsgQueue(hab, 0); /* start a queue */

    if (argc==2) {
	strncpy(destdir, argv[1], sizeof(destdir)-1);
	batch = TRUE;
    }
    load_string(IDS_GSVIEWBASE, gsviewbase, sizeof(gsviewbase));

    if (beta_warn())
	rc = 1;

    if (!rc)
        rc = install();

    if (rc) {
        char buf[256];
	sprintf(buf, "Installation aborted\012%s", error_message);
	message_box(buf, MB_MOVEABLE | MB_OK);
    }

    rc = cleanup();
    WinDestroyMsgQueue(hand_mq);
    WinTerminate(hab);
    return rc;
}

/* termination queue thread */
/* This thread waits for termination messages */
/* This thread must NOT call C library functions */
VOID APIENTRY term_thread(ULONG unused)
{
	REQUESTDATA Request;
	ULONG DataLength;
	PVOID DataAddress;
	BYTE ElemPriority;
	unused = unused;	/* to shut up warning */
	while ( !DosReadQueue(term_queue, &Request, &DataLength, &DataAddress, 
			0, DCWW_WAIT, &ElemPriority, (HEV)NULL) ) {
	    if (hwnd_dlg)
		WinPostMsg(hwnd_dlg, WM_COMMAND, MPFROM2SHORT(DID_OK, 0), MPFROM2SHORT(CMDSRC_OTHER,FALSE));
	    if (DataAddress != (PVOID)NULL)
	        DosFreeMem(DataAddress);
	}
	term_tid = 0;
}


/* General Dialog Box */
MRESULT EXPENTRY GeneralDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch(msg) {
    case WM_INITDLG:
        hwnd_dlg = hwnd;
	break;
    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case DID_OK:
	  hwnd_dlg = (HWND)NULL;
          WinDismissDlg(hwnd, DID_OK);
          return (MRESULT)TRUE;
	case DID_CANCEL:
	  hwnd_dlg = (HWND)NULL;
          WinDismissDlg(hwnd, DID_CANCEL);
          return (MRESULT)TRUE;
      }
      break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}	

MRESULT EXPENTRY InputDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch(msg) {
    case WM_INITDLG:
	WinSendMsg( WinWindowFromID(hwnd, ID_ANSWER),
	    	EM_SETTEXTLIMIT, MPFROM2SHORT(MAXSTR, 0), MPFROMLONG(0) );
	WinSetWindowText( WinWindowFromID(hwnd, ID_ANSWER),
	  	get_string_answer );
    	break;
    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case DID_OK:
	  WinEnableWindow(WinWindowFromID(hwnd, DID_OK), FALSE);
	  WinQueryWindowText(WinWindowFromID(hwnd, ID_ANSWER),
		MAXSTR, get_string_answer);
          WinDismissDlg(hwnd, DID_OK);
          return (MRESULT)TRUE;
	case DID_CANCEL:
          WinDismissDlg(hwnd, DID_CANCEL);
          return (MRESULT)TRUE;
      }
      break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}	
