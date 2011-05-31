/* Copyright (C) 1993-1998, Ghostgum Software Pty Ltd.  All rights reserved.
  
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
 *   README.TXT
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
#define BETA_YEAR    1998
#define BETA_MONTH   12
#define BETA_DAY     31
*/


/* GSview */

#define GSVIEW_PRODUCT "GSview"
#define GSVIEW_VERSION "2.7"
#define GSVIEW_DATE    "1998-12-23"
#ifdef _Windows
#ifdef __WIN32__
#ifdef DECALPHA
#define GSVIEW_ZIP     "win32da.zip"
#define GSVIEW_BASEDIR "gsviewda"
#else
#define GSVIEW_ZIP     "win32.zip"
#define GSVIEW_BASEDIR "gsview"
#endif
#else
#define GSVIEW_ZIP     "win16.zip"
#define GSVIEW_BASEDIR "gsview16"
#endif
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
#define GS_REVISION	  550
#define GS_REVISION_MAX   599
#endif
#ifdef _Windows
#ifdef __WIN32__
#define GSVIEW_EXENAME "gsview32.exe"
#define GS_EXENAME  "gswin32.exe"
#define GS_DLLNAME  "gsdll32.dll"
#define INIFILE "gsview32.ini"
#else
#define GSVIEW_EXENAME "gsview16.exe"
#define GS_EXENAME  "gswin16.exe"
#define GS_DLLNAME  "gsdll16.dll"
#define INIFILE "gsview16.ini"
#endif
#else
#define GS_DLLNAME  "gsdll2.dll"
#define INIFILE "gvpm.ini"
#endif

/* General */

#define EMX_NEEDED "0.9c"
#define INSTALL_DIR "\\gstools"

