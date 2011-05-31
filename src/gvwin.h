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
#define NeedFunctionPrototypes 1
#include "ps.h"
#include "gvcrc.h"
#include "gsdll.h"

#ifndef RC_INVOKED

/* for gsv16spl.exe 16-bit spooler interface for Win32s */
#define WM_GSV16SPL WM_USER+1
extern HWND hwndspl;	/* window handle of gsv16spl.exe */

#define PROFILE_SIZE 2048
#define MAXSTR 256	/* maximum file name length and general string length */
#define DEVICENAME "mswindll"
#define DEFAULT_GSCOMMAND "gswin32.exe"
#ifdef __WIN32__
#define INIFILE "gsview32.ini"
#else
#define INIFILE "gsview16.ini"
#endif
#define DEFAULT_RESOLUTION 96.0
#define DEFAULT_ZOOMRES 300.0
#define INISECTION "Options"
#define DEVSECTION "Devices"
#define EOLSTR "\r\n"
#define COPY_BUF_SIZE 4096
/* don't have to worry about segments/selectors */
#ifdef __WIN32__
#define GVFAR
#define GVHUGE
#else
#define GVFAR FAR
#define GVHUGE _huge
#endif

#include "gvceps.h"

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
	int 	preview;	/* preview type IDS_EPSF, IDS_EPSI, etc. */
#if defined(_Windows) && defined(OLD)
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
	/* if now set, at least one of the following five will be set */
	int	pagenum;	/* page number to display */
	PSFILE *psfile;		/* new document to display */
	BOOL	resize;		/* size, resolution or orientation change */
	BOOL	text;		/* extract text, don't display */
	BOOL	pdf2ps;		/* extract PS from PDF, don't display */
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


/* options that are saved in INI file */
typedef struct tagOPTIONS {
	int	language;
	int	gsversion;
	char	gsdll[MAXSTR];
	char	gsinclude[MAXSTR];
	char	gsother[MAXSTR];
	BOOL	configured;
	int	drawmethod;	/* OS/2 only */
	POINT	img_origin;
	POINT	img_size;
	BOOL	img_max;
	int	unit;
	int	pstotext;
	BOOL	quick_open;
	BOOL	settings;
	BOOL	button_show;
	BOOL	fit_page;
	BOOL	safer;
	int	media;
	char	medianame[32];
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
	int	pdf2ps;
	BOOL	auto_bbox;
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

struct prfentry {
	char *name;
	char *value;
	struct prfentry *next;
};

struct prfsection {
	char *name;
	struct prfentry *entry;
	struct prfsection *next;
};

struct prop_item_s {
	char	name[MAXSTR];
	char	value[MAXSTR];
};

typedef struct tagPROFILE {
	char *name;
	FILE *file;
	BOOL changed;
	struct prfsection *section;
} PROFILE;

typedef struct tagTEXTINDEX {
    int word;	/* offset to word */
    int line;	/* line number on page */
    PSBBOX bbox;
} TEXTINDEX;
extern TEXTINDEX *text_index;
extern int text_index_count;	/* number of words in index */
extern char *text_words;	/* storage for words */

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
typedef void GVFAR *gzFile ;
typedef gzFile (WINAPI *PFN_gzopen)(const char GVFAR *path, const char GVFAR *mode);
typedef int (WINAPI *PFN_gzread)(gzFile file, void GVFAR *buf, unsigned len);
typedef int (WINAPI *PFN_gzclose)(gzFile file);
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
typedef BOOL (WINAPI *FPSPS)(LPCSTR, UINT);
extern HINSTANCE hlib_mmsystem;	/* DLL containing sndPlaySound function */
extern FPSPS lpfnSndPlaySound;	/* pointer to sndPlaySound function if loaded */

extern const char szClassName[];
extern const char szImgClassName[];
extern const char szScratch[];  /* temporary filename prefix */
extern char *szSpoolPrefix;	/* usually \\spool\ */
extern char szAppName[MAXSTR];
extern char szHelpTopic[MAXSTR];
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
extern HWND hwndtext;			/* gswin text window */
extern HWND hwndimgchild;		/* gswin image child window */
extern HINSTANCE phInstance;		/* instance of gsview */
extern HINSTANCE hlanguage;		/* instance of language resources */
extern BOOL is_win31;			/* To allow selective use of win 3.1 features */
extern BOOL is_winnt;			/* To allow selective use of Windows NT features */
extern BOOL is_win95;			/* To allow selective use of Windows 95 features */
extern BOOL is_win32s;			/* To allow selective use of Win32s misfeatures */
extern BOOL is_win4;			/* To allow selective use of Windows 4.0 features */
extern BOOL multithread;		/* TRUE if running multithreaded */
#ifdef __WIN32__
extern CRITICAL_SECTION crit_sec;	/* for thread synchronization */
#endif
extern HANDLE hmutex_ps;		/* for protecting psfile and pending */
extern HMENU hmenu;			/* main menu */
extern HACCEL haccel;			/* menu accelerators */
extern HCURSOR hcWait;
extern HCURSOR hcCrossHair;
extern HCURSOR hcHand;
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


#ifdef __WIN32__
#define _huge
#define MoveTo(hdc,x,y) MoveToEx((hdc),(x),(y),(LPPOINT)NULL)
#define SetWindowOrg(hdc, x, y) SetWindowOrgEx(hdc, x, y, (LPPOINT)NULL)
#define	SetWindowExt(hdc, x, y) SetWindowExtEx(hdc, x, y, (LPSIZE)NULL)
#define SetClassCursor(hwnd, hcursor) SetClassLong((hwnd), GCL_HCURSOR, (LONG)(hcursor))
#define GetClassCursor(hwnd) ((HCURSOR)GetClassLong((hwnd), GCL_HCURSOR))
#define GetNotification(wParam,lParam) (HIWORD(wParam))
#define SendDlgNotification(hwnd, id, notice) \
    SendMessage((hwnd), WM_COMMAND, MAKELONG((id),(notice)), (LPARAM)GetDlgItem((hwnd),(id)))
#else
#define SetClassCursor(hwnd, hcursor) SetClassWord(hwnd, GCW_HCURSOR, (WORD)(hcursor))
#define GetClassCursor(hwnd) ((HCURSOR)GetClassWord(hwnd, GCW_HCURSOR))
#define GetNotification(wParam,lParam) (HIWORD(lParam))
#define SendDlgNotification(hwnd, id, notice) \
    SendMessage((hwnd), WM_COMMAND, id, MAKELPARAM(GetDlgItem((hwnd),(id)),(notice)))
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

/* in gvwprn.c */
BOOL get_portname(char *portname, char *port);
int gp_printfile(char *filename, char *port);
extern char not_defined[];
void start_gvwgs(void);

#endif
