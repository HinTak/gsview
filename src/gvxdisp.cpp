/* Copyright (C) 2000, Ghostgum Software Pty Ltd.  All rights reserved.
  
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

/* gvxdisp.c */
/* Display GSview routines for X11 */
#include "gvx.h"


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

