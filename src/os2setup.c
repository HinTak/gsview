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

#include "setup.h"
#include "gvcprf.h"
#include "gvcrc.h"

#define BASEDIR "\\gs3.53"
#define UNZIPEXE "os2unzip.exe"
#define GSVIEWZIP "gsview.zip"
#define GSINIZIP  "gs353ini.zip"
#define GSOS2ZIP  "gs353os2.zip"
#define GSFNTZIP  "gs353fn1.zip"
#define EMXZIP    "emxrt.zip"

MRESULT EXPENTRY GeneralDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY InputDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

char workdir[MAXSTR];
char bootdrive[MAXSTR];
char destdir[MAXSTR];
char unzipname[MAXSTR];
HAB hab;
HWND hwnd_dlg;
char get_string_answer[MAXSTR];
char szAppName[]="GSview Install";
char szExePath[MAXSTR];
char error_message[MAXSTR];
char no_error[] = "";
int emx;
int use_os2_fonts;

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
	    message_box( "Directory does not exist.  Please enter a directory name that does exist.",
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
	replace = (dialog(IDD_CONFIG, GeneralDlgProc) == DID_OK);
	strcpy(outname, bootdrive);
	strcat(outname, "\\config.gs");
	if ( (outfile = fopen(outname, "r")) != (FILE *)NULL)  {
	    fclose(outfile);
	    sprintf(buf, "File %s exists.  Overwrite?", outname);
	    if (message_box(buf, MB_MOVEABLE | MB_YESNO) != MBID_YES) {
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
	    sprintf(buf, "Changes were saved in %s", outname);
	    message_box(buf, MB_MOVEABLE | MB_OK);
	}
    }
    else {
	unlink(tempname);
    }

    return 0;
}

int
update_fontmap(void)
{
char fontmap[MAXSTR];
char fontmap_os2[MAXSTR];
char fontmap_old[MAXSTR];
char buf[MAXSTR];
FILE *f, *infile;
int backup = 1;
    if ( message_box("OS/2 has some Type 1 fonts of better quality\
 than those distributed with Ghostscript.  Do you want to use them?",
	    MB_MOVEABLE | MB_YESNO) != MBID_YES ) {
	return 0;
    }

    use_os2_fonts = TRUE;

    strcpy(fontmap, destdir);
    strcat(fontmap, BASEDIR);
    strcpy(fontmap_os2, fontmap);
    strcpy(fontmap_old, fontmap);
    strcat(fontmap, "\\Fontmap");
    strcat(fontmap_os2, "\\Fontmap.OS2");
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

    infile = fopen(fontmap_os2, "r");
    if (infile == (FILE *)NULL) {
	sprintf(error_message, "Can't open %s for reading", fontmap_os2);
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
}

int
update_ini(void)
{
char szIniName[MAXSTR];
PROFILE *prf;
char dest[MAXSTR];
char buf[MAXSTR];
    strcpy(szIniName, bootdrive);
    strcat(szIniName, "\\os2\\gvpm.ini");
    prf = profile_open(szIniName);
    if (prf == (PROFILE *)NULL) {
	sprintf(error_message, "Can't open %s", szIniName);
	return 1;
    }
    strcpy(dest, destdir);
    strcat(dest, BASEDIR);
    strcpy(buf, dest);
    strcat(buf, "\\gsos2.exe");
    profile_write_string(prf, "Options", "GhostscriptEXE", buf);
    strcpy(buf, dest);
    strcat(buf, ";");
    strcat(buf, dest);
    strcat(buf, "\\fonts");
    if (use_os2_fonts) {
        strcat(buf, ";");
	strcat(buf, bootdrive);
	strcat(buf, "\\psfonts");
    }
    profile_write_string(prf, "Options", "GhostscriptInclude", buf);
    profile_write_string(prf, "Options", "GhostscriptVersion", "351");
    profile_write_string(prf, "Options", "Version", GSVIEW_VERSION);
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
    strcat(buf, "\\gsview\\gvpm.exe");
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
	rc = unzip(GSVIEWZIP, destdir);
    }
    if (!rc) {
	/* install EMX if needed */
	sprintf(buf, "%s\\", bootdrive);
	if (!emx)
	    rc = unzip(EMXZIP, buf);
    }
    if (!rc)
	rc = unzip(GSINIZIP, destdir);
    if (!rc)
	rc = unzip(GSOS2ZIP, destdir);
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


    if (!rc) {
	message_box("Installation successful.\012A GSview program object has been created on the desktop", 
		MB_MOVEABLE | MB_OK);
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
