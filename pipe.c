/*
 * pipe.c --   Pipe support for GSVIEW.EXE, a graphical interface for 
 *             MS-Windows Ghostscript
 * Copyright (C) 1993  Russell Lang
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Author: Russell Lang
 * Internet: rjl@monu1.cc.monash.edu.au
 */

#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <shellapi.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dir.h>
#include <io.h>
#define NeedFunctionPrototypes 1
#include "ps.h"
#include "gsview.h"

/* routines to be visible outside this module */
extern void pipeinit();
extern FILE *pipeopen(void);	/* open pipe for first time */
extern void pipeclose(void);	/* finished with pipe, close & delete temp files */
extern void pipereset(void);	/* pipe is empty, do cleanup */
extern void piperequest(void);	/* request from gswin for pipe data */
extern void pipeflush(void);	/* start sending data through pipe */
extern BOOL is_pipe_done(void);	/* true if pipe has just been reset */

/* internal to this module */

/* imitation pipes using SHAREABLE GLOBAL MEMORY */
#define PIPE_DATASIZE 16384	/* maximum block size */
/* Data is passed to gswin in a global shareable memory block.
 * The global handle is passed in the LOWORD of lParam
 * and the HIWORD contains the byte count.
 * The maximum number of bytes passed is PIPE_DATASIZE.
 * EOF is signified by count = 0 (hglobal must still be valid)
 */

char pipe_name[MAXSTR];		/* pipe filename */
FILE *pipe_file;		/* pipe file */
fpos_t pipe_wpos;
fpos_t pipe_rpos;
BOOL pipe_empty;
int piperead(char *buf, int size);
char *pipebuf;

/* this is called before gswin is started */
/* so we can tell when we get the first piperequest */
void
pipeinit(void)
{
	pipe_empty = FALSE;	/* so we wait for first request */
}

FILE *
pipeopen(void)
{
	if (pipe_file != (FILE *)NULL) {
	    fclose(pipe_file);
	    pipe_file = (FILE *)NULL;
	    unlink(pipe_name);
	    pipe_name[0] = '\0';
	}
	if ((pipe_file = gp_open_scratch_file(szScratch, pipe_name, "w+b")) == (FILE *)NULL) {
	    gserror(IDS_PIPEERR, NULL, NULL, SOUND_ERROR);
	    unlink(pipe_name);
	    pipe_name[0] = '\0';
	    return (FILE *)NULL;
	}
	pipebuf = malloc(PIPE_DATASIZE);
	pipereset();
	return pipe_file;
}

void
pipeclose(void)
{
HGLOBAL hglobal;
	if (pipebuf != (char *)NULL) {
	    free(pipebuf);
	    pipebuf = (char *)NULL;
	}
	if (pipe_file != (FILE *)NULL) {
	    fclose(pipe_file);
	    pipe_file = (FILE *)NULL;
	    unlink(pipe_name);
	    pipe_name[0] = '\0';
	}
	if (hwndtext != (HWND)NULL) {
	    /* send an EOF (zero length block) */
	    hglobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, 1);
	    if (hglobal == (HGLOBAL)NULL) {
	        gserror(IDS_PIPEERR, NULL, NULL, SOUND_ERROR);
	        return;
	    }
	    PostMessage(hwndtext, WM_GSVIEW, PIPE_DATA, MAKELPARAM(hglobal,0));
	}
	pipe_empty = TRUE;
}

/* rpos and wpos are now empty so reset the file */
void
pipereset(void)
{
	if ( (pipe_file == (FILE *)NULL) || (pipe_name[0] == '\0') )
	    return;
	if ((pipe_file = freopen(pipe_name, "w+b", pipe_file)) == (FILE *)NULL) {
	    gserror(IDS_PIPEERR, NULL, NULL, SOUND_ERROR);
	    unlink(pipe_name);
	    pipe_name[0] = '\0';
	    return;
	}
	fgetpos(pipe_file, &pipe_rpos);
	fgetpos(pipe_file, &pipe_wpos);
	pipe_empty = TRUE;
	info_wait(FALSE);
}

/* give another block of data to gswin */
/* called from WndImgProc */
void
piperequest(void)
{
HGLOBAL hglobal;
LPBYTE lpb;
UINT count;
	if (pipe_file == (FILE *)NULL) {
	    pipe_empty = TRUE;
	    return;
	}

	count = piperead(pipebuf, PIPE_DATASIZE);
	if (count==0)
	    return;

	hglobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, PIPE_DATASIZE);
	if (hglobal == (HGLOBAL)NULL) {
	    gserror(IDS_PIPEERR, NULL, NULL, SOUND_ERROR);
	    return;
	}
	lpb = GlobalLock(hglobal);
	_fmemcpy(lpb, pipebuf, count);
	GlobalUnlock(hglobal);
	/* we may be processing SendMessage so use PostMessage to avoid lockups */
	PostMessage(hwndtext, WM_GSVIEW, PIPE_DATA, MAKELPARAM(hglobal,count));
}

/* write pipe_file to pipe */
void
pipeflush(void)
{
	if (pipe_empty) {
	    pipe_empty = FALSE;
	    piperequest();	/* repeat the request */
	}
	info_wait(TRUE);
}

/* true  if pipereset was last called */
/* false if pipeflush was last called */
int
is_pipe_done(void)
{
	return pipe_empty;
}

/* read a block from pipe */
/* return count of characters read */
/* reset pipe if empty */
int
piperead(char *buf, int size)
{
int rcount;
	fflush(pipe_file);
	fgetpos(pipe_file,&pipe_wpos);
	fsetpos(pipe_file,&pipe_rpos);
	rcount = fread(buf, 1, size, pipe_file);
	fgetpos(pipe_file,&pipe_rpos);
	fsetpos(pipe_file,&pipe_wpos);
	if (rcount == 0)
	   pipereset();
	return rcount;
}
