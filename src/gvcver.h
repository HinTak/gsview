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

/* version strings for GSview and Ghostscript */
/* These strings are used by both code and resources */

/* Version number changes must be made to
 *   gvcver.h
 *   README.TXT
 *   FILE_ID.DIZ
 *   gvcnews.txt
 *   gvc.doc
 */


/* GSview */

#define GSVIEW_PRODUCT "GSview"
#define GSVIEW_VERSION "2.0"
#define GSVIEW_DATE    "1996-08-11"
#define GSVIEW_BASEDIR "gsview"
#define GSVIEW_ZIP     "gsview.zip"
#define GSVIEW_COPYRIGHT1 "Copyright (C) 1993-1996 Russell Lang."
#define GSVIEW_COPYRIGHT2 "All rights reserved."
#define GSVIEW_COPYRIGHT3 "See the file LICENCE for more details."

/* #define BETA	/* undefine this for a final release */
#define BETA_YEAR    1996
#define BETA_MONTH   9
#define BETA_DAY     1


/* Ghostscript */
#define GS_PRODUCT  "Aladdin Ghostscript"
#define GS_REVISION 401
#define GS_VERSION  "4.01"
#define GS_BASEDIR  "gs4.01"
#define GS_INIZIP   "gs401ini.zip"
#define GS_W32ZIP   "gs401w32.zip"
#define GS_OS2ZIP   "gs401os2.zip"
#define GS_FN1ZIP   "gs401fn1.zip"
#ifdef __WIN32__
#define GS_DLLNAME  "gsdll32.dll"
#else
#define GS_DLLNAME  "gsdll2.dll"
#endif
#define GS_COPYRIGHT1 "Copyright (C) 1994-1996 Aladdin Enterprises,"
#define GS_COPYRIGHT2 "Menlo Park, California, U.S.A.  All rights reserved."
#define GS_COPYRIGHT3 "See the file PUBLIC for more details."

/* General */

#define EMX_NEEDED "0.9b"
#define INSTALL_DIR "\\gstools"


