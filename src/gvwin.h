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

/* gvwin.h */
/* Header includes for Windows GSview */

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
#ifdef _MSC_VER
#include <direct.h>
#define _export
#else
#include <dir.h>
#include <dirent.h>
#endif
#include <sys/stat.h>
#include <io.h>
#include <time.h>
#include <process.h>
#include "gsvver.h"
#include "gvcrc.h"
#ifdef _MSC_VER
#define DLLEXPORT __declspec(dllimport)
#endif
#include "gsdll.h"

#ifndef RC_INVOKED

#include "dscparse.h"
#include "gvcfile.h"

#ifndef NODEBUG_MALLOC
void * debug_malloc(size_t size);
void  * debug_realloc(void *block, size_t size);
void debug_free(void *block);
void debug_memory_report(void);
#define malloc(size) debug_malloc(size)
#define calloc(nitems, size) debug_calloc(nitems, size)
#define realloc(block, size) debug_realloc(block, size)
#define free(block) debug_free(block)
extern long allocated_memory;
#endif

#ifdef DEBUG_MALLOC
extern FILE *malloc_file;
#endif

/* for gsv16spl.exe 16-bit spooler interface for Win32s */
#define WM_GSV16SPL WM_USER+1
extern HWND hwndspl;	/* window handle of gsv16spl.exe */

#define PROFILE_SIZE 2048
#define MAXSTR 256	/* maximum file name length and general string length */
#define DEVICENAME "mswindll"
#define DEFAULT_GSCOMMAND "gswin32.exe"
#define DEFAULT_RESOLUTION 96.0
#define DEFAULT_ZOOMRES 300.0
#define INISECTION "Options"
#define DEVSECTION "Devices"
#define CONVERTSECTION "Convert"
#define EOLSTR "\r\n"
#define COPY_BUF_SIZE 4096
#define PATHSEP "\\"

#include "gvceps.h"
#include "gvcprf.h"

/* program details */
typedef struct tagPROG {
    BOOL	valid;
    HINSTANCE	hinst;
} PROG;

typedef struct tagPRINTER {
	PROG	prog;		/* Ghostscript program doing printing */
	char	optname[MAXSTR];/* file storing command line options */
	char	psname[MAXSTR]; /* file storing temporary postscript */
} PRINTER;

/* bitmap details */
typedef struct tagBM {
    int		width;
    int		height;
    int		scrollx;
    int		scrolly;
    BOOL	changed;	/* if width or height changed by GS */
} BMAP;

/*
typedef struct document PSDOC;
*/

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
	CDSC	*dsc;		/* DSC structure.  NULL if not DSC */
	PAGELIST page_list;	/* selected page list */
	int	print_from;
	int	print_to;
#define ALL_PAGES 0
#define ODD_PAGES 1
#define EVEN_PAGES 2
	int	print_oddeven;
	BOOL	print_ignoredsc;
	int	print_copies;
	BOOL	locked;		/* To prevent two threads using the file */
/*	BOOL	ignore_dsc;	/* true if DSC to be ignored */
	BOOL	ignore_special;	/* true if %%PageOrder: Special to be ignored */
	int 	pagenum;	/* current page number */
	BOOL	ctrld;		/* TRUE if file starts with ^D */
	BOOL	pjl;		/* TRUE if file starts with HP LaserJet PJL prologue */
	BOOL	gzip;		/* TRUE if file compressed with gzip */
	BOOL	bzip2;		/* TRUE if file compressed with bzip2 */
	int 	preview;	/* preview type IDS_EPSF, IDS_EPSI, etc. */
#ifdef OLD
	/* Can't use this because VC++ 5.0 and Windows 95 OSR1 & 2
	 * give incorrect times for days when daylight savings
	 * changes occur.  Main thread gives correct time, second
	 * thread gives one hour earlier!
	 */
	time_t	datetime;	/* time/date of selected file */
#else
	FILETIME filetime;	/* time/date of selected file */
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
	HINSTANCE	hmodule;	/* handle to module */
	int		state;
	long		revision_number;

	/* pointers to DLL functions */
	PFN_gsdll_revision	 revision;
	PFN_gsdll_init		 init;
	PFN_gsdll_exit		 exit;
	PFN_gsdll_execute_begin	 execute_begin;
	PFN_gsdll_execute_cont	 execute_cont;
	PFN_gsdll_execute_end	 execute_end;
	PFN_gsdll_lock_device	 lock_device;
	PFN_gsdll_copy_dib	 copy_dib;
	PFN_gsdll_copy_palette	 copy_palette;
	PFN_gsdll_draw		 draw;
	PFN_gsdll_get_bitmap_row get_bitmap_row;
	GSDLL_CALLBACK		 callback;

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
	int	drawmethod;	/* OS/2 only */
	char	helpcmd[MAXSTR];
	POINT	img_origin;
	POINT	img_size;
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
	int	dsc_warn;	/* level of DSC error warnings */
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
        /* for printing to GS device */
	char	printer_device[64];	/* Ghostscript device for printing */
	char	printer_resolution[64];
	int	print_fixed_media;
	/* for converting with GS device */
	char	convert_device[64];
	char	convert_resolution[64];	/* Ghostscript device for converting */
	int	convert_fixed_media;
	/* for printing to GDI device */
	int	print_gdi_depth;	/* IDC_MONO, IDC_GREY, IDC_COLOUR */
	int	print_gdi_fixed_media;
        /* general printing */
#define PRINT_GDI 0
#define PRINT_GS 1
#define PRINT_PS 2
#define PRINT_CONVERT 3
	int	print_method;		/* GDI, GS, PS */
	BOOL	print_reverse;		/* pages to be in reverse order */
	BOOL	print_to_file;
	char	printer_port[MAXSTR];	/* for Win32s */
	char	printer_queue[MAXSTR];	/* for Win32 */
	int	pdf2ps;
	BOOL	auto_bbox;
	MATRIX	ctm;
	MEASURE measure;
} OPTIONS;

typedef struct tagDISPLAY {
	int	width;		/* in pixels */
	int	height;
	int	planes;
	int	bitcount;
	BOOL	show_find;	/* highlight found text */
	BOOL	epsf_clipped;	/* display is clipped to bbox */
	int	zoom_xoffset;	/* in 1/72" */
	int	zoom_yoffset;
	POINT	offset;		/* offset of child window */
	float	xdpi;
	float	ydpi;
	int	xoffset;	/* offset of page in pixels */
	int	yoffset;
	int	orientation;	/* 0-3 = 0, 90, 180 or 270 degrees */
	BOOL	init;		/* viewer initialised */
	BOOL	saved;		/* interpreter state saved */
	BOOL	need_header;
	BOOL	need_trailer;
	HANDLE	event;
	unsigned long tid;
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
extern unsigned int text_index_count;	/* number of words in index */
extern char *text_words;	/* storage for words */

/* all the external DLL use "C", not C++ */
extern "C" {

/* for pstotext DLL */
typedef int (GSDLLAPI *PFN_pstotextInit)(void **instance);
typedef int (GSDLLAPI *PFN_pstotextFilter)(void *instance, char *instr, 
    char **pre, char **word, char **post,
    int *llx, int *lly, int *urx, int *ury);
typedef int (GSDLLAPI *PFN_pstotextExit)(void *instance);
typedef int (GSDLLAPI *PFN_pstotextSetCork)(void *instance, int value);
extern HMODULE pstotextModule;
extern FILE *pstotextOutfile;
extern void *pstotextInstance;
extern PFN_pstotextInit pstotextInit;
extern PFN_pstotextFilter pstotextFilter;
extern PFN_pstotextExit pstotextExit;
extern PFN_pstotextSetCork pstotextSetCork;
extern char pstotextLine[2048];
extern int pstotextCount;


/* for zlib gunzip decompression */
extern HINSTANCE zlib_hinstance;
typedef void *gzFile ;
typedef gzFile (WINAPI *PFN_gzopen)(const char *path, const char *mode);
typedef int (WINAPI *PFN_gzread)(gzFile file, void *buf, unsigned len);
typedef int (WINAPI *PFN_gzclose)(gzFile file);
extern PFN_gzopen gzopen;
extern PFN_gzread gzread;
extern PFN_gzclose gzclose;

/* for bzip2 decompression */
extern HINSTANCE bzip2_hinstance;
typedef void *bzFile ;
typedef bzFile (WINAPI *PFN_bzopen)(const char *path, const char *mode);
typedef int (WINAPI *PFN_bzread)(bzFile file, void *buf, unsigned len);
typedef int (WINAPI *PFN_bzclose)(bzFile file);
extern PFN_bzopen bzopen;
extern PFN_bzread bzread;
extern PFN_bzclose bzclose;

}


extern BOOL print_silent;	/* /P or /F command line option used */
extern BOOL print_exit;		/* exit on completion of printing */
extern int print_count;		/* number of current print jobs */
				/* It is safe to exit GSview when this is 0 */

extern int debug;		/* /D command line option used */
extern FILE *debug_file;	/* for gs input logging */

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
typedef BOOL (WINAPI *FPSPS)(LPCSTR, UINT);
extern HINSTANCE hlib_mmsystem;	/* DLL containing sndPlaySound function */
extern FPSPS lpfnSndPlaySound;	/* pointer to sndPlaySound function if loaded */

extern const char szClassName[];
extern const char szImgClassName[];
extern const char szScratch[];  /* temporary filename prefix */
extern char *szSpoolPrefix;	/* usually \\spool\ */
extern char szAppName[MAXSTR];
extern int nHelpTopic;
extern char szWait[MAXSTR];
extern char szExePath[MAXSTR];
extern char szIniFile[MAXSTR];
extern char szFindText[MAXSTR];
extern char szHelpName[MAXSTR];		/* buffer for building help filename */
extern char previous_filename[MAXSTR];	/* to remember name between file dlg boxes */
extern char selectname[MAXSTR];		/* for IDM_SELECT */
extern UINT help_message;		/* message sent by OFN_SHOWHELP */
extern HWND hwndimg;			/* gsview main window */
extern HWND hDlgModeless;		/* any modeless dialog box */
extern HWND hwnd_measure;		/* measure modeless dialog box */
extern HWND hwndimgchild;		/* gswin image child window */
extern HWND hwnd_fullscreen;		/* full screen popup of child window */
extern HWND hwnd_image;			/* full screen or image child window */	
extern HINSTANCE phInstance;		/* instance of gsview */
extern HINSTANCE hlanguage;		/* instance of language resources */
extern BOOL is_win31;			/* To allow selective use of win 3.1 features */
extern BOOL is_winnt;			/* To allow selective use of Windows NT features */
extern BOOL is_win95;			/* To allow selective use of Windows 95 features */
extern BOOL is_win98;			/* To allow selective use of Windows 98 features */
extern BOOL is_win32s;			/* To allow selective use of Win32s misfeatures */
extern BOOL is_win4;			/* To allow selective use of Windows 4.0 features */
extern BOOL multithread;		/* TRUE if running multithreaded */
extern CRITICAL_SECTION crit_sec;	/* for thread synchronization */
extern HANDLE hmutex_ps;		/* for protecting psfile and pending */
extern HMENU hmenu;			/* main menu */
extern HACCEL haccel;			/* menu accelerators */
extern HCURSOR hcWait;
extern HCURSOR hcCrossHair;
extern HCURSOR hcHand;
extern HPEN hpen_btnshadow;		/* button shadow */
extern HPEN hpen_btnhighlight;		/* button highlight */
extern HBRUSH hbrush_window;		/* Window background */
extern HBRUSH hbrush_menu;		/* menu background */
extern int bitmap_scrollx;	/* offset from bitmap to origin of child window */
extern int bitmap_scrolly;
extern HFONT info_font;
extern POINT img_offset;		/* offset to gswin child window */
extern POINT info_file;		/* position of file information */
extern POINT info_page;		/* position of page information */
extern RECT  info_rect;		/* position and size of brief info area */
extern RECT  info_coord;		/* position and size of coordinate information */
extern RECT  button_rect;		/* position and size of button area */
extern int on_link;			/* TRUE if we were or are over link */
extern int on_link_page;		/* page number of link target */
extern long gsbytes_size;		/* number of bytes for this page */
extern long gsbytes_done;		/* number of byte written */
extern OPENFILENAME ofn;
extern WNDPROC lpfnButtonWndProc;
extern int percent_done;		/* percentage of document processed */
extern int percent_pending;		/* TRUE if WM_GSPERCENT is pending */
extern BOOL ignore_sync;		/* ignore next GSDLL_SYNC */
extern BOOL fit_page_enabled;		/* next WM_SIZE is allowed to resize window */

extern PROG gsprog;
extern BMAP bitmap;
extern OPTIONS option;
extern PSFILE psfile;
extern PRINTER printer;
extern BOOL win32s_printer_pending;
extern DISPLAY display;
/* extern PSDOC *doc;	/* points to psfile->doc */
extern int page_skip;
extern BOOL changed_version;
extern BOOL zoom;

extern PSBBOX bbox;

extern char registration_name[MAXSTR];
extern unsigned int registration_receipt;

/* PRINT_GDI */
extern int print_gdi_width;
extern int print_gdi_height;
extern int print_gdi_xdpi;
extern int print_gdi_ydpi;
extern HANDLE print_gdi_read_handle;
extern HANDLE print_gdi_write_handle;

#define MoveTo(hdc,x,y) MoveToEx((hdc),(x),(y),(LPPOINT)NULL)
#define SetWindowOrg(hdc, x, y) SetWindowOrgEx(hdc, x, y, (LPPOINT)NULL)
#define	SetWindowExt(hdc, x, y) SetWindowExtEx(hdc, x, y, (LPSIZE)NULL)
#define SetClassCursor(hwnd, hcursor) SetClassLong((hwnd), GCL_HCURSOR, (LONG)(hcursor))
#define GetClassCursor(hwnd) ((HCURSOR)GetClassLong((hwnd), GCL_HCURSOR))
#define GetNotification(wParam,lParam) (HIWORD(wParam))
#define SendDlgNotification(hwnd, id, notice) \
    SendMessage((hwnd), WM_COMMAND, MAKELONG((id),(notice)), (LPARAM)GetDlgItem((hwnd),(id)))
/* early versions of Win32s don't support lstrcpyn */
#undef lstrcpyn
#define lstrcpyn(d,s,n) strncpy(d,s,n)


#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#include "gvcfn.h"    /* common function prototypes */
#include "gvcbeta.h"  /* common function prototypes */

/* in gvwin.c */
LRESULT CALLBACK _export MenuButtonProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK _export WndImgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK _export WndImgChildProc(HWND, UINT, WPARAM, LPARAM);
BOOL in_child_client_area(void);

/* in gvwinit.c */
void gsview_init0(LPSTR lpszCmdLine);
BOOL gsview_init1(LPSTR lpszCmdLine);
void gsview_create(void);

/* in gvwdisp.c */
void do_message(void);
BOOL exec_pgm(char *name, char *arg, PROG* prog);
void stop_pgm(PROG* prog);
void cleanup_pgm(PROG *prog);

/* in gvwdlg.c */
BOOL CALLBACK _export PageDlgProc(HWND hDlg, UINT wmsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK _export PageMultiDlgProc(HWND hDlg, UINT wmsg, WPARAM wParam, LPARAM lParam);

/* in gvwprn.c */
BOOL get_portname(char *portname, char *port);
int gp_printfile(char *filename, char *port);
extern char not_defined[];
void start_gvwgs(void);

#endif
