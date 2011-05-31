/* Copyright (C) 2000, 2001, Ghostgum Software Pty Ltd.  All rights reserved.
  
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

/* gvx.h */
/* Header includes for X11 GSview */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
/*
#include <X11/Xatom.h>
*/
#include <semaphore.h>
#define __USE_GNU	/* we need recursive mutex */
#include <pthread.h>

#define NeedFunctionPrototypes 1
#define GSDLL_SYNC 4    /* sync_output for device str */ 
#include "gsvver.h"
#include "gvcrc.h"
#include "gvxres.h"


typedef int BOOL;
typedef guint16 WORD;
typedef guint32 DWORD;
typedef guint32 ULONG;
typedef guint32 UINT;
typedef gint32 LONG;
typedef guint8 BYTE;
typedef GtkWidget *HWND;
#define stricmp strcasecmp

/* some of these should be private to gvx.cpp */
extern Display *dpy;
extern GtkWidget *window;
extern GtkWidget *main_vbox;
extern GtkWidget *menubar;
extern GtkWidget *buttonbar;
extern GtkWidget *scroll_window;
extern GtkWidget *img;
extern GtkWidget *statusbar;
extern GtkWidget *statusfile;
extern GtkWidget *statuscoord;
extern GtkWidget *statuspage;
extern char *pszLocale;

#ifdef NOTUSED
extern GdkPixmap *pixmap;

extern int gs_pid;
extern Atom ghostview_atom;
extern Atom page_atom;
extern Atom done_atom;
extern Atom next_atom;
extern int busy;
extern Window gs_window;
extern int gs_pipe_stdin[2];
extern int gs_pipe_stdout[2];
extern int gs_pipe_stderr[2];
extern int infile;
#endif


#define P0() void
#define P1(t1) t1
#define P2(t1,t2) t1,t2
#define P3(t1,t2,t3) t1,t2,t3
#define P4(t1,t2,t3,t4) t1,t2,t3,t4
#define P5(t1,t2,t3,t4,t5) t1,t2,t3,t4,t5
#define P6(t1,t2,t3,t4,t5,t6) t1,t2,t3,t4,t5,t6

#include "errors.h"
#include "iapi.h"
#include "gdevdsp.h"

#include "dscparse.h"

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

#define MAXSTR 256	/* maximum file name length and general string length */
#define PROFILE_SIZE 2048
#define DEVICENAME "display"
#define DEFAULT_GSCOMMAND "gs"
#define DEFAULT_RESOLUTION 96.0
#define DEFAULT_ZOOMRES 300.0
#define INISECTION "Options"
#define DEVSECTION "Devices"
#define CONVERTSECTION "Convert"
#define EOLSTR "\r\n"
#define CW_USEDEFAULT 32768
#define COPY_BUF_SIZE 4096
#define PATHSEP "/"
#define LPSTR char *

/* Windows IDs */
#define IDOK  1
#define IDCANCEL  2
#define IDYES 6
#define IDNO 7

#define MB_OK 		0x0
#define MB_OKCANCEL 	0x1
#define MB_YESNO 	0x4
#define MB_TYPEMASK	0xf
#define MB_ICONHAND	   0x10
#define MB_ICONQUESTION	   0x20
#define MB_ICONEXCLAMATION 0x30
#define MB_ICONASTERISK	   0x40

#define WM_CLOSE	0x0010
#define WM_QUIT		0x0012
#define WM_COMMAND	0x0111
#define WM_USER		0x0400

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
    int		pid;
} PROG;

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
	CDSC	*dsc;		/* DSC structure.  NULL if not DSC */
	PAGELIST page_list;	/* selected page list */
//	int	print_from;
//	int	print_to;
#define ALL_PAGES 0
#define ODD_PAGES 1
#define EVEN_PAGES 2
//	int	print_oddeven;
	BOOL	print_ignoredsc;
	int	print_copies;
	BOOL	locked;		/* To prevent two threads using the file */
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

extern pthread_mutex_t hmutex_ps; 	/* for protecting psfile and pending */

typedef struct tagGSINPUT {
    unsigned long ptr;
    unsigned long end;
    BOOL seek;
} GSINPUT;

typedef struct tagGSDLL_INPUT {
    int	count;
    int	index;
    GSINPUT section[5];	/* header, defaults, prolog, setup, page */
} GSDLL_INPUT;


/* main structure with info about the GS DLL */
#include "cdll.h"
#include "cimg.h"
#include "cview.h"


typedef struct tagMATRIX {
   float xx, xy, yx, yy, tx, ty;
} MATRIX;

typedef struct tagMEASURE {
   float tx, ty;	/* translation */
   float rotate;	/* rotation */
   float sx, sy;	/* scaling */
   int unit;		/* IDM_UNITPT .. IDM_UNITCUSTOM */
} MEASURE;

struct POINT {
    int x;
    int y;
};


/* options that are saved in INI file */
typedef struct tagOPTIONS {
	int	language;
	int	gsversion;
	char	gsdll[MAXSTR];
	char	gsinclude[MAXSTR];
	char	gsother[MAXSTR];
	BOOL	configured;
	int	drawmethod;
	char	helpcmd[MAXSTR];
	POINT	img_origin;
	POINT	img_size;
	BOOL	img_max;
	int	unit;
	BOOL	unitfine;
	int	pstotext;
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
	char	printer_device[32];	/* Ghostscript device for printing */
	char	printer_resolution[32];
	int	print_fixed_media;
	/* for converting with GS device */
	char	convert_device[32];
	char	convert_resolution[32];	/* Ghostscript device for converting */
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
	char	printer_port[32];	/* for Win32s */
	char	printer_queue[MAXSTR];	/* for Win32 */
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
	POINT	offset;	
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
	sem_t	event;
	pthread_t tid;
} DISPLAY;

extern GtkWidget *last_file_widget[4];
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

typedef struct tagPRINTER {
	int	pid;	 	 /* Ghostscript program doing printing */
	char	optname[MAXSTR]; /* file storing command line options */
	char	psname[MAXSTR];	 /* file storing temporary postscript */
} PRINTER;

extern int print_count;		/* number of current print jobs */

extern BOOL getting_bbox;	/* PS to EPS get Bounding Box dialog is shown */

extern int debug;			/* /D command line option used */
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
	const char *entry;	/* profile entry */
	int title;		/* Resource string */
	char file[MAXSTR];	/* empty, "beep", or .WAV sound to play */
};
extern struct sound_s sound[NUMSOUND];
#define BEEP "beep"		/* profile entry for a speaker beep */

extern char szAppName[MAXSTR];
extern int nHelpTopic;
extern char szDocPath[MAXSTR];
extern char szEtcPath[MAXSTR];
extern char szHelpName[MAXSTR];
extern char szWait[MAXSTR];
extern char szFindText[MAXSTR];
extern char szIniFile[MAXSTR];
extern char previous_filename[MAXSTR];	/* to remember name between file dlg boxes */
extern char selectname[MAXSTR];		/* for IDM_SELECT */
extern const char szScratch[];	/* temporary filename prefix */
extern const char *szSpoolPrefix;	/* usually \\spool\ */
extern ULONG os_version;
extern BOOL multithread;		/* TRUE if running multithreaded */
extern int geometry_width;
extern int geometry_height;
extern int geometry_xoffset;
extern int geometry_yoffset;

/*
extern POINT buttonbar;
extern POINT statusbar;
extern POINT info_file;
extern POINT info_page;
extern RECT info_coord;
*/
extern int on_link;			/* TRUE if we were or are over link */
extern int on_link_page;		/* page number of link target */
extern long gsbytes_size;		/* number of bytes for this page */
extern long gsbytes_done;		/* number of byte written */
	/* percent_done = gsbytes_done * 100.0 / gsbytes_size */
extern int percent_done;		/* percentage of document processed */
extern int percent_pending;		/* TRUE if WM_GSPERCENT is pending */
extern BOOL fit_page_enabled;		/* next WM_SIZE is allowed to resize window */
extern BOOL fit_page_enabled;		/* next WM_SIZE is allowed to resize window */
extern BOOL quitnow;	/* Used to cause exit from nested message loops */


extern PROG pdfconv;
extern OPTIONS option;
extern PSFILE psfile; 
extern DISPLAY display;
extern PRINTER printer;
extern int page_extra;
extern int page_skip;
extern BOOL changed_version;
extern BOOL zoom;

extern PSBBOX bbox;

extern char registration_name[MAXSTR];
extern unsigned int registration_receipt;

#include "gvcfn.h"	/* common function prototypes */
#include "gvcbeta.h"    /* common function prototypes */

/* for pstotext DLL */
#define HMODULE int	/* pid */
extern HMODULE pstotextModule;
extern FILE *pstotextOutfile;
extern void *pstotextInstance;
extern char pstotextLine[2048];
extern int pstotextCount;

/* all the external DLL use "C", not C++ */
extern "C" {
/* for zlib gunzip decompression */
extern void * zlib_hinstance;
typedef void *gzFile ;
typedef gzFile (*PFN_gzopen)(const char *path, const char *mode);
typedef int (*PFN_gzread)(gzFile file, void *buf, unsigned len);
typedef int (*PFN_gzclose)(gzFile file);
extern PFN_gzopen gzopen;
extern PFN_gzread gzread;
extern PFN_gzclose gzclose;

/* for bzip2 decompression */
extern void * bzip2_hinstance;
typedef void *bzFile ;
typedef bzFile (*PFN_bzopen)(const char *path, const char *mode);
typedef int (*PFN_bzread)(bzFile file, void *buf, unsigned len);
typedef int (*PFN_bzclose)(bzFile file);
extern PFN_bzopen bzopen;
extern PFN_bzread bzread;
extern PFN_bzclose bzclose;

}

#ifdef NOTUSED
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


/* in gvx.cpp */
int init_img_message(void);
void close_img_message(void);
void quit_gsview( GtkWidget *w, gpointer   data);
gboolean gs_client_event(GtkWidget *widget, GdkEventClient *event, gpointer data);
void gsview_wcmd( GtkWidget *w, gpointer data);
gint button_press_event(GtkWidget *widget, GdkEventButton *event);
gint button_release_event(GtkWidget *widget, GdkEventButton *event);
gint motion_notify_event(GtkWidget *widget, GdkEventMotion *event);
gint configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data);
gint expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gint size_event(GtkWidget *widget, GtkAllocation *allocation, gpointer user_data);
void statusbar_update(void);
void set_last_used(void);
void set_scroll(int hscroll, int vscroll);

/* in gvxinit.cpp */
int gsview_init(void);
void enable_menu_item(int menuid, int itemid, BOOL enabled);

/* in gvxdlg.cpp */
/* dialog callbacks */
gint modal_delete(GtkWidget *widget, GdkEvent event, gpointer data);
void modal_ok(GtkWidget *w, gpointer data);
void modal_cancel(GtkWidget *w, gpointer data);
int modal_dialog(GtkWidget *w, GtkWidget *okw, GtkWidget *cancelw) ;
void modal_help(GtkWidget *w, gpointer data);
void selpage_all(GtkButton *button, gpointer data);
void selpage_odd(GtkButton *button, gpointer data);
void selpage_even(GtkButton *button, gpointer data);
void bbox_click(void);
void width_percent(GtkWidget *w, GtkRequisition *req, gpointer data);


/* in gvxmeas.cpp */
void measure_close(void);

/* in gvxmisc.cpp */
void exec_program(const char *prog, const char *arg);
void SetDlgItemText(HWND hwnd, int id, const char *str);

/* in gvxprn.cpp */
int gp_printfile(char *filename, char *port);
extern char not_defined[];
/* PRINT_GDI - NOT IMPLEMENTED FOR UNIX */
extern int print_gdi_width;
extern int print_gdi_height;
extern int print_gdi_xdpi;
extern int print_gdi_ydpi;
extern ULONG print_gdi_read_handle;
extern ULONG print_gdi_write_handle;

/* gvxres.cpp */
const char *get_string(int id);

