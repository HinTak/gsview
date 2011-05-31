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

/* gvwpipe.c */
/* pipe support for Windows GSview */
#include "gvwin.h"

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
#ifdef __WIN32__
#define PIPE_DATASIZE 16380	/* maximum block size */
#else
#define PIPE_DATASIZE 4092	/* maximum block size */
#endif
/* Data is passed to gswin in a global shareable memory block.
 * The global handle is passed in lParam and the byte count
 * is stored in the first word of the global memory block.
 * The maximum number of bytes passed is PIPE_DATASIZE.
 * EOF is signified by count = 0 (hglobal must still be valid)
 */
/* In gsview 1.0, Ghostscript 2.6.1 and earlier, 
 * The global handle was passed in the LOWORD of lParam 
 * and the HIWORD contained the byte count.
 * This was changed in gsview 1.1, Ghostscript 3.0 so that a
 * 32 bit handle could be used for Win32  */
/* In gsview 1.2, Windows NT or 95 uses a memory mapped file */

char pipe_name[MAXSTR];		/* pipe filename */
FILE *pipe_file;		/* pipe file */
fpos_t pipe_wpos;
fpos_t pipe_rpos;
BOOL pipe_empty;
int piperead(char *buf, int size);
char *pipebuf;
HANDLE pipe_hmapfile;
LPBYTE pipe_mapptr;

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
#ifdef __WIN32__
	if (is_winnt || is_win95) {
	    char buf[64];
	    if (pipe_hmapfile != 0) {
		if (pipe_mapptr != NULL)
		    UnmapViewOfFile(pipe_mapptr);
		pipe_mapptr = NULL;
		CloseHandle(pipe_hmapfile);
		pipe_hmapfile = 0;
	    }
	    sprintf(buf,"gsview_%d", hwndimg);
	    pipe_hmapfile = CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, 
				PIPE_DATASIZE+sizeof(WORD), buf);
	    pipe_mapptr = MapViewOfFile(pipe_hmapfile, FILE_MAP_WRITE, 0, 0, 0);
	    if (pipe_mapptr == NULL) {
		message_box("Can't memory map file",0);
		return NULL;
	    }
	}
#endif
	if ((pipe_file = gp_open_scratch_file(szScratch, pipe_name, "w+b")) == (FILE *)NULL) {
	    gserror(IDS_PIPE_EOPEN, NULL, NULL, SOUND_ERROR);
	    unlink(pipe_name);
	    pipe_name[0] = '\0';
	    return (FILE *)NULL;
	}
	pipebuf = malloc(PIPE_DATASIZE);
	if (pipebuf == (char *)NULL) {
	    gserror(IDS_PIPE_EMEM, NULL, NULL, SOUND_ERROR);
	    fclose(pipe_file);
	    return (FILE *)NULL;
	}
	pipereset();
	return pipe_file;
}

void
pipeclose(void)
{
HGLOBAL hglobal;
LPBYTE lpb;
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
#if !defined(__WIN32__) && defined(GS261)
	  if (option.gsversion == IDM_GS261) {
	    /* send an EOF (zero length block) */
	    hglobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, 1);
	    if (hglobal == (HGLOBAL)NULL) {
	        gserror(IDS_PIPE_EMEM, NULL, NULL, SOUND_ERROR);
	        return;
	    }
	    PostMessage(hwndtext, WM_GSVIEW, PIPE_DATA, MAKELPARAM(hglobal,0));
	  }
	  else {
#endif
#ifdef __WIN32__
	    if (is_winnt || is_win95) {
		/* send an EOF (zero length block) */
		*((WORD *)pipe_mapptr) = 0;
		PostMessage(hwndtext, WM_GSVIEW, PIPE_DATA, (LPARAM)hwndimg);
	    }
	    else
#endif
	    {
		/* send an EOF (zero length block) */
		hglobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, sizeof(WORD));
		if (hglobal == (HGLOBAL)NULL) {
		    gserror(IDS_PIPE_EMEM, NULL, NULL, SOUND_ERROR);
		    return;
		}
		lpb = GlobalLock(hglobal);
		*((WORD FAR *)lpb) = 0;
		GlobalUnlock(hglobal);
		PostMessage(hwndtext, WM_GSVIEW, PIPE_DATA, (LPARAM)hglobal);
	    }
#if !defined(__WIN32__) && defined(GS261)
	  }
#endif
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
	    gserror(IDS_PIPE_EOPEN, NULL, NULL, SOUND_ERROR);
	    unlink(pipe_name);
	    pipe_name[0] = '\0';
	    return;
	}
	fgetpos(pipe_file, &pipe_rpos);
	fgetpos(pipe_file, &pipe_wpos);
	pipe_empty = TRUE;
	info_wait(IDS_NOWAIT);
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

#ifdef __WIN32__
	if (is_winnt || is_win95) {
	    memcpy(pipe_mapptr+sizeof(WORD), pipebuf, count);
	    *((WORD *)pipe_mapptr) = (WORD)count;
	    PostMessage(hwndtext, WM_GSVIEW, PIPE_DATA, (LPARAM)hwndimg);
	}
	else
#endif
 	{
	    hglobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, PIPE_DATASIZE + sizeof(WORD));
	    if (hglobal == (HGLOBAL)NULL) {
		gserror(IDS_PIPE_EMEM, NULL, NULL, SOUND_ERROR);
		return;
	    }
#if !defined(__WIN32__) && defined(GS261)
	  if (option.gsversion == IDM_GS261) {
	    lpb = GlobalLock(hglobal);
	    _fmemcpy(lpb, pipebuf, count);
	    GlobalUnlock(hglobal);
	    /* we may be processing SendMessage so use PostMessage to avoid lockups */
	    PostMessage(hwndtext, WM_GSVIEW, PIPE_DATA, MAKELPARAM(hglobal,count));
	  }
	  else {
#endif
	    lpb = GlobalLock(hglobal);
	    *((WORD FAR *)lpb) = (WORD)count;
	    _fmemcpy(lpb+sizeof(WORD), pipebuf, count);
	    GlobalUnlock(hglobal);
	    /* we may be processing SendMessage so use PostMessage to avoid lockups */
	    PostMessage(hwndtext, WM_GSVIEW, PIPE_DATA, (LPARAM)hglobal);
#if !defined(__WIN32__) && defined(GS261)
	  }
#endif
	}
}

/* write pipe_file to pipe */
void
pipeflush(void)
{
	if (pipe_empty) {
	    pipe_empty = FALSE;
	    piperequest();	/* repeat the request */
	}
	info_wait(IDS_WAIT);
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
