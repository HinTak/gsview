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

/* gvwdisp.c */
/* Display GSview routines for Windows */
#include "gvwin.h"

/* handle messages while we are waiting */
void
do_message(void)
{
    MSG msg;
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
	if ((hDlgModeless == 0) || !IsDialogMessage(hDlgModeless, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}




#ifdef OLD
/* return TRUE if file length or modification time changed */
BOOL
psfile_changed(void)
{
struct ftime thisftime;
long thisflength;
struct ftime *ft;
	getftime(fileno(psfile.file), &thisftime);
	thisflength = filelength(fileno(psfile.file));
	ft = &psfile.datetime;
	if (ft->ft_year==0 && ft->ft_month==0 && ft->ft_day==0 && 
	    ft->ft_hour==0 && ft->ft_min==0 && ft->ft_tsec==0) {
	    gs_addmess("psfile_changed: ignoring file time, getftime() is returning garbage\n");
	    return (thisflength != psfile.length);
	}
	else
	    return ( (thisflength != psfile.length) ||
		memcmp(&thisftime, &psfile.datetime, sizeof(thisftime)) );
}

void
psfile_savestat(PSFILE *psf)
{
	if (getftime(fileno(psf->file), &psf->datetime)) {
	    char buf[MAXSTR];
	    sprintf(buf, "psfile_savestat: getftime() returns error. errno=%d\n", errno);
	    gs_addmess(buf);
	}
	psf->length = filelength(fileno(psf->file));
}

#else
/* return TRUE if file length or modification time changed */
BOOL
psfile_changed(PSFILE *psf)
{
time_t thisftime;
long thisflength;
struct stat fstatus;
        if (psf == (PSFILE *)NULL)
	   psf = &psfile;
	fstat(fileno(psf->file), &fstatus);
	thisftime = fstatus.st_mtime;
	thisflength = fstatus.st_size;
	return ( (thisflength != psf->length) ||
		memcmp(&thisftime, &psf->datetime, sizeof(thisftime)) );
}

void
psfile_savestat(PSFILE *psf)
{
struct stat fstatus;
	fstat(fileno(psf->file), &fstatus);
	psf->datetime = fstatus.st_mtime;
	psf->length = fstatus.st_size;
}
#endif


BOOL 
exec_pgm(char *name, char *arg, PROG* prog)
{
char command[MAXSTR*2];
	prog->valid = FALSE;
	command[0] = '\0';
#ifdef __WIN32__
	if (!is_win32s)
	    strcat(command, "\042");
#endif
	strcat(command, name);
#ifdef __WIN32__
	if (!is_win32s)
	    strcat(command, "\042");
#endif
	strcat(command, " ");
	strcat(command, arg);
	prog->hinst = (HINSTANCE)WinExec(command, SW_SHOWMINNOACTIVE);
	if (prog->hinst > (HINSTANCE)31)
	    prog->valid = TRUE;
	return prog->valid;
}

/* stop specified program */
void
stop_pgm(PROG* prog)
{
	if (!prog->valid) {
	    cleanup_pgm(prog);
	    return;
	}
#ifdef __WIN32__
/* should really stop program by sending it a WM_CLOSE */
/* figuring out which window is messy */
	{
	    int i = 0;
	    TerminateProcess(prog->hinst, 1);
	    while (prog->valid && (i < 100)) {
		/* wait for termination message to cause cleanup_pgm() to be called */
		Sleep(100);
		peek_message();
		i++;
	    }
	    if (i >= 100)
		gserror(0, "can't stop program", MB_ICONHAND, SOUND_ERROR);
	}
#endif
/* cleanup should already have occurred */
	cleanup_pgm(prog);
}


/* cleanup after program has stopped */
void
cleanup_pgm(PROG* prog)
{
	prog->valid = FALSE;
	prog->hinst = 0;
}

