/* Copyright (C) 1993, 1994, 1995, Russell Lang.  All rights reserved.
  
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

/* gvwgs.h */

#define STRICT
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <dos.h>
#include "gsdll.h"
#include "gvcrc.h"
#include "gsdll.h"

#define GVWGS_VERSION	"0.1"

#ifndef DS_3DLOOK
#define DS_3DLOOK 0x0004L	/* for Windows 95 look */
#endif

#ifndef RC_INVOKED

#define WM_TEXTUPDATE	WM_USER+1
#define WM_PCUPDATE	WM_USER+2

#define MAXSTR 256


/* main structure with info about the GS DLL */
typedef struct tagGSDLL {
	HINSTANCE	hmodule;	/* handle to module */

	/* pointers to DLL functions */
	PFN_gsdll_revision	revision;
	PFN_gsdll_init		init;
	PFN_gsdll_exit		exit;
	PFN_gsdll_execute_begin	execute_begin;
	PFN_gsdll_execute_cont	execute_cont;
	PFN_gsdll_execute_end	execute_end;
	PFN_gsdll_lock_device	lock_device;
} GSDLL;


#endif
