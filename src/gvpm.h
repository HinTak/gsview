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

/* gvpm.h */
/* Header includes for PM GSview */

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_SPL
#define INCL_SPLDOSPRINT
#define INCL_SPLERRORS
#define INCL_DOSERRORS
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#ifdef __BORLANDC__
#include <dir.h>
#include <process.h>
#endif
#define NeedFunctionPrototypes 1
#include "ver.h"
#include "gvcrc.h"
#include "gsdll.h"


#ifndef RC_INVOKED

typedef unsigned short WORD;
typedef unsigned long DWORD;
#include "ps.h"

#define MAXSTR 256	/* maximum file name length and general string length */
#define PROFILE_SIZE 2048
#define DEVICENAME "os2dll"
#define DEFAULT_GSCOMMAND "gsos2.exe"
#define DEFAULT_RESOLUTION 96.0
#define DEFAULT_ZOOMRES 300.0
#define INISECTION "Options"
#define DEVSECTION "Devices"
#define EOLSTR "\r\n"
#define CW_USEDEFAULT 32768
#define COPY_BUF_SIZE 4096
/* don't have to worry about segments/selectors */
#define GVFAR
#define GVHUGE
#define LPSTR char *
#define IDYES MBID_YES
#define IDNO MBID_NO
#define IDOK  MBID_OK
#define IDCANCEL  MBID_CANCEL
#define HINSTANCE HMODULE

#define ID_STATUSBAR   100
#define ID_BUTTONBAR   101
#define SB_TOP 20
#define SB_BOTTOM 21

#ifndef min
#define min(x,y)  ( (x) < (y) ? (x) : (y) )
#endif
#ifndef max
#define max(x,y)  ( (x) > (y) ? (x) : (y) )
#endif

#include "gvceps.h"
#include "gvcprf.h"

/* program details */
typedef struct tagPROG {
    BOOL	valid;
    ULONG	session_id;
    PID		process_id;
} PROG;

/* bitmap details */
typedef struct tagBM {
    BOOL	valid;
    BOOL	old_bmp;	/* bitmap type */
    PBITMAPINFO2 pbmi;		/* pointer to bitmap info */
    PBYTE	bits;		/* pointer to bitmap bits */
    int		width;
    int		height;
    int		planes;
    int		depth;
    int		palsize;
    int 	palimportant;
    int		old_width;
    int		old_height;
    int		old_planes;
    int		old_depth;
    int		old_palsize;
    int		old_palimportant;
} BMAP;

typedef struct document PSDOC;

typedef struct tagPDFLINK {
    PSBBOX bbox;
    int page;
    float border_xr;
    float border_yr;
    float border_width;
    float colour_red;
    float colour_green;
    float colour_blue;
    BOOL  colour_valid;
    struct tagPDFLINK *next;
    /* need to add View */
} PDFLINK;

typedef struct tagPAGELIST {
	int current;	/* index of current selection */
	BOOL multiple;	/* true if multiple selection allowed */
	BOOL *select;	/* array of selection flags */
	BOOL reverse;	/* reverse pages when extracting or printing */
} PAGELIST;

typedef struct tagPSFILE {
	char 	name[MAXSTR];	/* name of selected document file */
	char	tname[MAXSTR];	/* name of temporary file (gunzipped) */
	FILE 	*file;		/* selected file */
	PSDOC	*doc;		/* DSC structure.  NULL if not DSC */
	PAGELIST page_list;	/* selected page list */
	BOOL	locked;		/* To prevent two threads using the file */
	BOOL	ignore_dsc;	/* true if DSC to be ignored */
	BOOL	ignore_special;	/* true if %%PageOrder: Special to be ignored */
	int 	pagenum;	/* current page number */
	BOOL	ctrld;		/* TRUE if file starts with ^D */
	BOOL	pjl;		/* TRUE if file starts with HP LaserJet PJL prologue */
	BOOL	gzip;		/* TRUE if file compressed with gzip */
	BOOL	bzip2;		/* TRUE if file compressed with bzip2 */
	int 	preview;	/* preview type IDS_EPSF, IDS_EPSI, etc. */
#ifdef _Windows
	struct	ftime datetime;	/* time/date of selected file */
#else
	time_t	datetime;	/* time/date of selected file */
#endif
	long	length;		/* length of selected file */
	BOOL	ispdf;		/* true if PDF document */
	char 	text_name[MAXSTR];  /* name of file containing extracted text */
	unsigned long text_offset;  /* file offset after last text search match */
	int	text_page;	    /* page of last text search match */
	BOOL	text_extract;       /* TRUE=extracting, FALSE=searching */
	PSBBOX	text_bbox;	    /* bbox of found word */
} PSFILE;

/* State of GS DLL */
/* state transitions are between adjacent states only */
/* except IDLE can be skipped between UNLOADED and BUSY */
#define UNLOADED 0	/* DLL has not been loaded or has been unloaded */
#define IDLE     1	/* No input is being sent to DLL */
#define BUSY     2	/* Input is being sent to DLL */
#define PAGE     3      /* Waiting at showpage */

/* In the single threaded version, there are three places that
 * process the message loop:
 *  1. Main get message loop in UNLOADED state
 *  2. Peek message loop in BUSY state
 *  3. Get message loop in PAGE state
 * We must avoid recursive calls from the last two message loop handlers.
 * In the multithreaded version, the DLL is handled by a separate thread.
 * User input that affects the GS DLL sets some pending variables, 
 * which are processed later by the appropriate message loop or thread.
 * Changes to the pending structure must be made inside a critical section.
 */
/* pending structure */
typedef struct tagPENDING {
	BOOL	unload;		/* We want to unload the DLL */
	BOOL	abort;		/* ignore errors and restart the interpreter */
	BOOL	restart;	/* restart interpreter */
	BOOL	redisplay;	/* redisplay after interpreter restarted */
	BOOL	next;		/* move to next page */
	BOOL	now;		/* We want to do something now */
	/* if now set, at least one of the following six will be set */
	int	pagenum;	/* page number to display */
	PSFILE *psfile;		/* new document to display */
	BOOL	resize;		/* size, resolution or orientation change */
	BOOL	text;		/* extract text, don't display */
	BOOL	pdf2ps;		/* extract PS from PDF, don't display */
	BOOL	pstoedit;	/* extract using pstoedit, don't display */
} PENDING;

extern PENDING pending;

typedef struct tagGSINPUT {
    unsigned long ptr;
    unsigned long end;
    BOOL seek;
} GSINPUT;

/* main structure with info about the GS DLL */
typedef struct tagGSDLL {
	BOOL		valid;		/* true if loaded */
	HMODULE		hmodule;	/* handle to module */
	int		state;
	long		revision_number;

	/* pointers to DLL functions */
	PFN_gsdll_revision	revision;
	PFN_gsdll_init		init;
	PFN_gsdll_exit		exit;
	PFN_gsdll_execute_begin	execute_begin;
	PFN_gsdll_execute_cont	execute_cont;
	PFN_gsdll_execute_end	execute_end;
	PFN_gsdll_get_bitmap	get_bitmap;
	PFN_gsdll_lock_device	lock_device;
	GSDLL_CALLBACK		callback;

	/* pointer to os2dll or mswindll device */
	unsigned char	*device;

	int	input_count;
	int	input_index;
	GSINPUT input[5];	/* header, defaults, prolog, setup, page */
} GSDLL;

typedef struct tagMATRIX {
   float xx, xy, yx, yy, tx, ty;
} MATRIX;

typedef struct tagMEASURE {
   float tx, ty;	/* translation */
   float rotate;	/* rotation */
   float sx, sy;	/* scaling */
   int unit;		/* IDM_UNITPT .. IDM_UNITCUSTOM */
} MEASURE;


/* options that are saved in INI file */
typedef struct tagOPTIONS {
	int	language;
	int	gsversion;
	char	gsdll[MAXSTR];
	char	gsinclude[MAXSTR];
	char	gsother[MAXSTR];
	BOOL	configured;
	int	drawmethod;
	POINTL	img_origin;
	POINTL	img_size;
	BOOL	img_max;
	int	unit;
	BOOL	unitfine;
	int	pstotext;
	BOOL	quick_open;
	BOOL	settings;
	BOOL	button_show;
	BOOL	fit_page;
	BOOL	safer;
	int	media;
	char	medianame[32];
	BOOL	media_rotate;
	int	user_width;
	int	user_height;
	BOOL	epsf_clip;
	BOOL	epsf_warn;
	BOOL	redisplay;
	BOOL    ignore_dsc;
	BOOL	show_bbox;
	BOOL	auto_orientation;
	int	orientation;
	BOOL	swap_landscape;
	float	xdpi;
	float	ydpi;
	float	zoom_xdpi;
	float	zoom_ydpi;
	int	depth;
	int	alpha_text;
	int	alpha_graphics;
	BOOL	save_dir;
	char	device_name[32];
	char	device_resolution[32];
	char	printer_port[32];
	char	printer_queue[MAXSTR];
	BOOL	print_to_file;
	BOOL	psprinter;
	BOOL	print_reverse;
	BOOL	print_fixed_media;
	int	pdf2ps;
	BOOL	auto_bbox;
	MATRIX	ctm;
	MEASURE measure;
} OPTIONS;

typedef struct tagDISPLAY {
	int	width;
	int	height;
	LONG	planes;
	LONG	bitcount;
	BOOL	show_find;	/* highlight found text */
	BOOL	epsf_clipped;	/* display is clipped to bbox */
	int	zoom_xoffset;	/* in 1/72" */
	int	zoom_yoffset;
	POINTL	offset;	
	LONG	hasPalMan;	/* Palette Manager */
	BOOL	hpal_exists;
	HPAL	hpal;
	/* new ones for DLL */
	float	xdpi;
	float	ydpi;
	int	xoffset;
	int	yoffset;
	int	orientation;
	BOOL	init;		/* viewer initialised */
	BOOL	saved;		/* interpreter state saved */
	BOOL	need_header;
	BOOL	need_trailer;
	HEV	event;
	TID	tid;
} DISPLAY;

extern char last_files[4][MAXSTR];	/* last 4 files used */
extern int last_files_count;		/* number of files known */

#define HISTORY_MAX 32
typedef struct tagHISTORY {
    int index;  /* index of next page to store */
    int count;	/* number of valid pages in history */
    int pages[HISTORY_MAX];
} HISTORY;
extern HISTORY history;		/* history of pages displayed */

typedef struct tagTEXTINDEX {
    int word;	/* offset to word */
    int line;	/* line number on page */
    PSBBOX bbox;
} TEXTINDEX;
extern TEXTINDEX *text_index;
extern int text_index_count;	/* number of words in index */
extern char *text_words;	/* storage for words */

typedef struct tagPRINTER {
	PROG	prog;		 /* Ghostscript program doing printing */
	char	optname[MAXSTR]; /* file storing command line options */
	char	psname[MAXSTR];	 /* file storing temporary postscript */
} PRINTER;


/* button bar */
struct button {
	int	id;
	RECTL	rect;
	HBITMAP	hbitmap;
	char	str[16];
	struct button *next;
};
extern struct button *buttonhead, *buttontail;
extern BOOL button_down;
extern struct button *button_current;
extern POINTL button_shift;
extern POINTL button_size;

/* for pstotext DLL */
typedef int (GSDLLAPI *PFN_pstotextInit)(void **instance);
typedef int (GSDLLAPI *PFN_pstotextFilter)(void *instance, char *instr, 
    char **pre, char **word, char **post,
    int *llx, int *lly, int *urx, int *ury);
typedef int (GSDLLAPI *PFN_pstotextExit)(void *instance);
typedef int (GSDLLAPI *PFN_pstotextSetCork)(void *instance, int value);
extern HMODULE pstotextModule;
FILE *pstotextOutfile;
void *pstotextInstance;
extern PFN_pstotextInit pstotextInit;
extern PFN_pstotextFilter pstotextFilter;
extern PFN_pstotextExit pstotextExit;
extern PFN_pstotextSetCork pstotextSetCork;
char pstotextLine[2048];
int pstotextCount;

/* for zlib gunzip decompression */
extern HMODULE zlib_hmodule;
typedef void *gzFile ;
typedef gzFile (*PFN_gzopen)(const char *path, const char *mode);
typedef int (*PFN_gzread)(gzFile file, void *buf, unsigned len);
typedef int (*PFN_gzclose)(gzFile file);
extern PFN_gzopen gzopen;
extern PFN_gzread gzread;
extern PFN_gzclose gzclose;


extern BOOL debug;			/* /D command line option used */
extern FILE *debug_file;		/* for gs input logging */

#define SOUND_PAGE 0
#define SOUND_NOPAGE 1
#define SOUND_NONUMBER 2
#define SOUND_NOTOPEN 3
#define SOUND_ERROR 4
#define SOUND_START 5
#define SOUND_EXIT 6
#define SOUND_BUSY 7
#define NUMSOUND 8
struct sound_s {
	char *entry;		/* profile entry */
	int title;		/* Resource string */
	char file[MAXSTR];	/* empty, "beep", or .WAV sound to play */
};
extern struct sound_s sound[NUMSOUND];
#define BEEP "beep"		/* profile entry for a speaker beep */
typedef ULONG (* PFN_MciPlayFile)(HWND hwndOwner, PSZ pszFile, ULONG ulFlags,
	PSZ pszTitle, HWND hwndViewport);
extern PFN_MciPlayFile pfnMciPlayFile;

extern char szAppName[MAXSTR];
extern char szHelpTopic[MAXSTR];
extern char szExePath[MAXSTR];
extern char szHelpName[MAXSTR];
extern char szWait[MAXSTR];
extern char szFindText[MAXSTR];
extern char szIniFile[MAXSTR];
extern char szMMini[MAXSTR];
extern char previous_filename[MAXSTR];	/* to remember name between file dlg boxes */
extern char selectname[MAXSTR];		/* for IDM_SELECT */
extern const char szScratch[];	/* temporary filename prefix */
extern char *szSpoolPrefix;	/* usually \\spool\ */
extern ULONG os_version;
extern BOOL multithread;		/* TRUE if running multithreaded */
extern HMTX hmutex_ps;
extern HAB hab;
extern HWND hwnd_frame;
extern HWND hwnd_bmp;
extern HWND hwnd_image;
extern HWND hwnd_fullscreen;
#define hwndimg hwnd_bmp		/* to look like Windows */
extern HWND hwnd_status;
extern HWND hwnd_button;
extern HWND hwnd_help;
extern HWND hwnd_modeless;		/* any modeless dialog box */
extern HWND hwnd_measure;		/* measure modeless dialog box */
extern HWND hptr_crosshair;
extern HWND hptr_hand;
extern HWND hwnd_menu;
extern HACCEL haccel;
extern HMODULE hlanguage;
extern POINTL buttonbar;
extern POINTL statusbar;
extern POINTL info_file;
extern POINTL info_page;
extern RECTL info_coord;
extern int on_link;			/* TRUE if we were or are over link */
extern int on_link_page;		/* page number of link target */
MRESULT EXPENTRY ClientWndProc(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY FrameWndProc(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY StatusWndProc(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY ButtonWndProc(HWND, ULONG, MPARAM, MPARAM);
extern PFNWP OldFrameWndProc;
extern int percent_done;		/* percentage of document processed */
extern int percent_pending;		/* TRUE if WM_GSPERCENT is pending */
extern BOOL ignore_sync;		/* ignore next GSDLL_SYNC */
extern BOOL fit_page_enabled;		/* next WM_SIZE is allowed to resize window */


extern PROG pdfconv;
extern OPTIONS option;
extern PSFILE psfile; 
extern DISPLAY display;
extern PRINTER printer;
extern BMAP bitmap;
extern int page_extra;
extern int page_skip;
extern BOOL changed_version;
extern BOOL zoom;

extern PSBBOX bbox;


#include "gvcfn.h"	/* common function prototypes */
#include "gvcbeta.h"    /* common function prototypes */

/* in gvpm.c */
void update_scroll_bars(void);

/* gvpdlg.c */
MRESULT EXPENTRY PageDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY PageMultiDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* in gvpinit.c */
APIRET gsview_init(int argc, char *argv[]);

/* in gvpmisc.c */
BOOL SetDlgItemText(HWND hwnd, int id, char *str);

/* in gvpdisp.c */
BOOL exec_pgm(char *name, char *arg, PROG* prog);
void stop_pgm(PROG* prog);
void cleanup_pgm(PROG *prog);
BOOL gs_open(void);
BOOL gs_close(void);

/* in gvpprn.c */
#ifndef NERR_BufTooSmall
#define NERR_BufTooSmall 2123	/* For SplEnumQueue */
#endif
BOOL get_portname(char *portname, char *port);
int gp_printfile(char *filename, char *port);
extern char not_defined[];

#endif
