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

/* gvpdisp.c */
/* Display GSview routines for PM */
#include "gvpm.h"


/* execute program */
BOOL
exec_pgm(char *name, char *arg, BOOL withpipe, PROG* prog)
{
	STARTDATA sdata;
	APIRET rc;
	char buf[256];
	CHAR progname[256];
	PROG pg;
	int pipe_handle[2];
	PTIB pptib;
	PPIB pppib;

	if (prog->valid)
		stop_pgm(prog);
	memset(&pg, 0, sizeof(PROG));

	if (DosGetInfoBlocks(&pptib, &pppib)) {
		error_message("\nexec_pgm: Couldn't get environment\n");
		return FALSE;
	}
	/* Look for program in same directory as this EXE */
	progname[0] = '\0';
	if (!strchr(name, '\\'))
	    strcpy(progname, szExePath);
	strcat(progname, name);

	if (withpipe) {
	    /* redirect stdin of new program */
#ifdef __BORLANDC__
	    if (_pipe(pipe_handle, 4096, O_BINARY))
#else
	    if (pipe(pipe_handle))		/* create a pipe */
#endif
 	    {
		error_message("exec_pgm: error opening pipe");
		return FALSE;
	    }
	    if (dup2(pipe_handle[0], 0) < 0) {	/* duplicate read handle to stdin handle */
		error_message("error duplicating pipe");
		return FALSE;
	    }
	    close(pipe_handle[0]);			/* close surplus handle */
	    pg.input = fdopen(pipe_handle[1], "w");	/* open a stream to the write handle */
/*
sprintf(buf,"exec_pgm: input stream = %p, handle = %d", pg.input, pipe_handle[1]);
rmsg(buf);
*/
	    if (pg.input == (FILE *)NULL) {
		error_message("exec_pgm: error opening write stream for pipe");
		return FALSE;
	    }
	}
	
	/* because new program is a different EXE type, 
	 * we must use start session not DosExecPgm() */
	sdata.Length = sizeof(sdata);
	sdata.Related = SSF_RELATED_CHILD;	/* to be a child  */
	sdata.FgBg = SSF_FGBG_BACK;		/* start in background */
	sdata.TraceOpt = 0;
	sdata.PgmTitle = name;
	sdata.PgmName = progname;
	sdata.PgmInputs = arg;
	sdata.TermQ = gsview.term_queue_name;
	sdata.Environment = pppib->pib_pchenv;	/* use Parent's environment */
	/* with pipe needs to inherit redirected stdin */
	sdata.InheritOpt = withpipe ? SSF_INHERTOPT_PARENT : 0;
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

/*
sprintf(buf,"exec_pgm: %s %s\n",sdata.PgmName, sdata.PgmInputs);
message_box(buf, 0);
*/
	rc = DosStartSession(&sdata, &pg.session_id, &pg.process_id);
	if (rc == ERROR_FILE_NOT_FOUND) {
	    /* didn't find it in same directory as this EXE so try PATH */
	    sdata.PgmName = name;
	    rc = DosStartSession(&sdata, &pg.session_id, &pg.process_id);
	}
	if (rc) {
	    if (withpipe)
	        fclose(pg.input);
	    sprintf(buf,"\"%s %s\", rc = %d\n", sdata.PgmName, sdata.PgmInputs, rc);
	    gserror(IDS_CANNOTRUN, buf, MB_ICONHAND, SOUND_ERROR);
	    load_string(IDS_TOPICINSTALL, szHelpTopic, sizeof(szHelpTopic));
	    get_help();
	    return FALSE;
	}
	pg.valid = TRUE;
	*prog = pg;	/* give details back to caller */
	return TRUE;
}


/* stop specified program */
void
stop_pgm(PROG* prog)
{
QMSG q_mess;		/* queue message */
int i = 0;
	if (!prog->valid) {
	    cleanup_pgm(prog);
	    return;
	}
	DosStopSession(STOP_SESSION_SPECIFIED, prog->session_id);
	while (prog->valid && term_tid && (i < 100)) {
	    /* wait for termination queue message to cause cleanup_pgm() to be called */
  	    if (WinGetMsg(hab, &q_mess, 0L, 0, 0))
	        WinDispatchMsg(hab, &q_mess);
	    i++;
	}
	if (i >= 100)
	    gserror(0, "can't stop program", MB_ICONHAND, SOUND_ERROR);
/* cleanup should already have occurred */
	cleanup_pgm(prog);
}


/* cleanup after program has stopped */
void
cleanup_pgm(PROG* prog)
{
	prog->valid = FALSE;
	if (prog->input)
	    fclose(prog->input);
	prog->input = (FILE *)NULL;
	prog->session_id = 0;
	prog->process_id = (PID)0;
}



BOOL
gs_open()
{
char progname[256];
char progargs[256];
char *args;
BOOL flag;
ULONG count;
	/* return if already open */
	if (gsprog.valid)
		return TRUE;

	gs_size();
	args = strchr(option.gscommand, ' ');
	if (args) {
	    strncpy(progname, option.gscommand, (int)(args-option.gscommand));
	    progname[(int)(args-option.gscommand)] = '\0';
	    args++;
	}
	else {
	    strncpy(progname, option.gscommand, MAXSTR);
	    args = "";
	}
	
	sprintf(progargs,"%s -dBitsPerPixel=%d %s -r%gx%g -g%ux%u -sGSVIEW=%s -",
		args, (option.depth ? option.depth : display.bitcount*display.planes), 
		option.safer ? "-dSAFER" : "",
		option.xdpi, option.ydpi, 
                display.width, display.height,
		gsview.id);
	display.saved = FALSE;
	display.page = FALSE;
	display.sync = FALSE;
	zoom = FALSE;
	display.do_endfile = FALSE;
	display.do_resize = FALSE;
	load_string(IDS_WAITGSOPEN, szWait, sizeof(szWait));
	info_wait(TRUE);
	flag = exec_pgm(progname, progargs, TRUE, &gsprog);
	if (!flag)
	    display.do_display = FALSE;
	return flag;
}

/* don't create any windows in this routine - can be called from display thread */
/* close Ghostscript */
BOOL
gs_close()
{
	if (display.busy) {
	    display.abort = TRUE;
	    DosWaitEventSem(display.done, 60000);
	}
	stop_pgm(&gsprog);
	display.saved = FALSE;
	display.epsf_clipped = FALSE;
	display.page = FALSE;
	display.sync = FALSE;
	/* mark bitmap as unused */
	if (DosRequestMutexSem(gsview.bmp_mutex, 10000) == ERROR_TIMEOUT)
	    message_box("gs_close: mutex timeout", 0);
	DosEnterCritSec();
	DosFreeMem((PVOID)bitmap.pbmi);
	bitmap.valid = FALSE;
	DosExitCritSec();
	DosReleaseMutexSem(gsview.bmp_mutex);
	return TRUE;
}

/* send a NEXT_PAGE message to Ghostscript */
void
next_page()
{
	DosPostEventSem(gsview.next_event);
	display.page = FALSE;
}

/* return TRUE if file length or modification time changed */
BOOL
psfile_changed(void)
{
time_t thisftime;
long thisflength;
struct stat fstatus;
	fstat(fileno(psfile.file), &fstatus);
	thisftime = fstatus.st_mtime;
	thisflength = fstatus.st_size;
	return ( (thisflength != psfile.length) ||
		memcmp(&thisftime, &psfile.datetime, sizeof(thisftime)) );
}

void
psfile_savestat(void)
{
struct stat fstatus;
	fstat(fileno(psfile.file), &fstatus);
	psfile.datetime = fstatus.st_mtime;
	psfile.length = fstatus.st_size;
}


/* true if pipe has just been reset */
BOOL is_pipe_done(void)
{
	if (display.end)
	   return TRUE;
/*
	if (display.sync)
	   return TRUE;
*/
	if (!gsprog.valid)
	   return TRUE;
	return FALSE;
}


