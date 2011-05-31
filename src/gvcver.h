/* Copyright (C) 1993-2000, Ghostgum Software Pty Ltd.  All rights reserved.
  
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

/* version strings for GSview and Ghostscript */
/* These strings are used by both code and resources */

/* Version number changes must be made to
 *   gvcver.h
 *   gvcver.mak
 *   Readme.htm
 *   FILE_ID.DIZ
 *   gvcnews.txt
 *   language/gvclang.txt
 *   language/gvclang.h    (for Copyright dates)
 *   gvpm.mak
 *   gvwin.mak
 *   gvwinvc.mak
 */


/* undefine BETA for a final release */
/*
#define BETA
#define BETA_YEAR    2000
#define BETA_MONTH   7
#define BETA_DAY     30
*/


/* GSview */
#include "gsvver.h"

#define GSVIEW_PRODUCT "GSview"

#ifdef _Windows
#ifdef DECALPHA
#define GSVIEW_BASEDIR "gsviewda"
#else
#define GSVIEW_BASEDIR "gsview"
#endif
#define GSVIEW_ZIP     "win32.zip"
#else
#define GSVIEW_ZIP     "os2.zip"
#define GSVIEW_BASEDIR "gsview"
#endif


/* Ghostscript */
#define GS_PRODUCT  "Aladdin Ghostscript"
#define GS_REVISION_MIN   403
#if defined(_Windows) && !defined(__WIN32__)
#define GS_REVISION	  403		/* this is the last 16-bit version */
#define GS_REVISION_MAX   403
#else
#define GS_REVISION	  600
#define GS_REVISION_MAX   699
#endif
#ifdef _Windows
#ifdef DECALPHA
#define GSVIEW_EXENAME "gsviewda.exe"
#else
#define GSVIEW_EXENAME "gsview32.exe"
#endif
#define GS_EXENAME  "gswin32.exe"
#define GS_DLLNAME  "gsdll32.dll"
#define INIFILE "gsview32.ini"
#else
#define GS_DLLNAME  "gsdll2.dll"
#define INIFILE "gvpm.ini"
#endif

/* General */

#define EMX_NEEDED "0.9d"
#define INSTALL_DIR "\\gstools"

