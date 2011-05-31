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
#include "gvcrc.h"
#include "gdevpm.h"


#ifndef RC_INVOKED

typedef unsigned short WORD;
typedef unsigned long DWORD;
#include "ps.h"

#define MAXSTR 256	/* maximum file name length and general string length */
#define PROFILE_SIZE 2048
#define DEFAULT_GSCOMMAND "gsos2.exe"
#define DEFAULT_RESOLUTION 96.0
#define DEFAULT_ZOOMRES 300.0
#define HELPFILE "gvpm.hlp"
#define INIFILE "gvpm.ini"
#define INISECTION "Options"
#define DEVSECTION "Devices"
#define EOLSTR "\r\n"
#define CW_USEDEFAULT 32768
#define COPY_BUF_SIZE 4096
/* don't have to worry about segments/selectors */
#define GVFAR
#define GVHUGE
#define IDYES MBID_YES
#define IDOK  MBID_OK
#define IDCANCEL  MBID_CANCEL

#define WM_GSUPDATING  WM_USER+GS_UPDATING
#define WM_GSSYNC      WM_USER+GS_SYNC
#define WM_GSPAGE      WM_USER+GS_PAGE
#define WM_GSCLOSE     WM_USER+GS_CLOSE
#define WM_GSERROR     WM_USER+GS_ERROR
#define WM_GSPALCHANGE WM_USER+GS_PALCHANGE
#define WM_GSBEGIN     WM_USER+GS_BEGIN
#define WM_GSEND       WM_USER+GS_END
#define WM_GSPRNEXIT   WM_USER+GS_PALCHANGE+10

#define ID_STATUSBAR   100
#define ID_BUTTONBAR   101

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
    FILE	*input;		/* pipe to stdin */
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

typedef struct tagPSBBOX {
	int	llx;
	int	lly;
	int	urx;
	int	ury;
	int	valid;
} PSBBOX;

typedef struct tagGV {
	PID		pid;		/* process id of GSview */
	char		id[MAXSTR];	/* string id of GSview */
	HQUEUE		queue;		/* queue for messages from GS */
	HEV		next_event;	/* semaphore for Next Page */
	HMTX		bmp_mutex;	/* mutex for access to bitmap */
	HQUEUE		term_queue;	/* termination queue for child sessions */
	char		term_queue_name[MAXSTR];  /* termination queue name */
} GVIEW;

typedef struct tagPSFILE {
	BOOL	ignore_dsc;	/* true if DSC to be ignored */
	PSDOC	*doc;		/* DSC structure.  NULL if not DSC */
	int 	pagenum;	/* current page number */
	char 	name[MAXSTR];	/* name of selected document file */
	FILE 	*file;		/* selected file */
	BOOL	ctrld;		/* TRUE if file starts with ^D */
	int 	preview;	/* preview type IDS_EPSF, IDS_EPSI, etc. */
#ifdef _Windows
	struct	ftime datetime;	/* time/date of selected file */
#else
	time_t	datetime;	/* time/date of selected file */
#endif
	long	length;		/* length of selected file */
	char	previous_name[MAXSTR];
	BOOL	previous_was_dsc;
	long	previous_begintrailer;
	long	previous_endtrailer;
	BOOL	ispdf;		/* true if PDF document */
	char 	pdftemp[MAXSTR]; /* name of temporary DSC file */
} PSFILE;

/* options that are saved in INI file */
typedef struct tagOPTIONS {
	/* char	gscommand[MAXSTR]; */ /* no longer used */
	char	gsexe[MAXSTR];
	char	gsinclude[MAXSTR];
	char	gsother[MAXSTR];
	int	gsversion;
	int	drawmethod;
	POINTL	img_origin;
	POINTL	img_size;
	BOOL	img_max;
	int	unit;
	BOOL	quick;
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
	int	orientation;
	BOOL	swap_landscape;
	float	xdpi;
	float	ydpi;
	float	zoom_xdpi;
	float	zoom_ydpi;
	int	depth;
	BOOL	save_dir;
	char	device_name[32];
	char	device_resolution[32];
	char	printer_port[32];
} OPTIONS;

typedef struct tagDISPLAY {
	char	optname[MAXSTR]; /* file storing command line options */
	BOOL	abort;
	BOOL	busy;
	ULONG	id;
	HEV	done;
	int	width;
	int	height;
	LONG	planes;
	LONG	bitcount;
	BOOL	do_endfile;	
	BOOL	do_resize;
	BOOL	do_display;
	BOOL	epsf_clipped;	/* display is clipped to bbox */
	int	zoom_xoffset;	/* in 1/72" */
	int	zoom_yoffset;
	POINTL	offset;	
	BOOL	saved;		/* interpreter state saved */
	BOOL	page;		/* GS_PAGE received */
	BOOL	sync;		/* GS_SYNC received */
	BOOL	end;		/* GS_END received */
	LONG	hasPalMan;	/* Palette Manager */
	BOOL	hpal_exists;
	HPAL	hpal;
} DISPLAY;

typedef struct tagPRINTER {
	PROG	prog;		/* Ghostscript program doing printing */
	char	optname[MAXSTR]; /* file storing command line options */
	char	psname[MAXSTR];	/* file storing temporary postscript */
	ULONG	tid;		/* Thread ID which waits for GS to exit */
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

extern BOOL debug;			/* /D command line option used */
extern FILE *debug_file;		/* for gs input logging */
struct page_list_s {
	int current;	/* index of current selection */
	BOOL multiple;	/* true if multiple selection allowed */
	BOOL *select;	/* array of selection flags */
};
extern struct page_list_s page_list;

#define SOUND_PAGE 0
#define SOUND_NOPAGE 1
#define SOUND_NONUMBER 2
#define SOUND_NOTOPEN 3
#define SOUND_ERROR 4
#define SOUND_TIMEOUT 5
#define SOUND_START 6
#define SOUND_EXIT 7
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
extern char szHelpFile[MAXSTR];
extern char szWait[MAXSTR];
extern char szFindText[MAXSTR];
extern char szIniFile[MAXSTR];
extern char szMMini[MAXSTR];
extern char previous_filename[MAXSTR];	/* to remember name between file dlg boxes */
extern const char szScratch[];	/* temporary filename prefix */
extern char szSpoolPrefix[];	/* usually \\spool\ */
extern ULONG os_version;
extern HAB hab;
extern HWND hwnd_frame;
extern HWND hwnd_bmp;
extern HWND hwnd_status;
extern HWND hwnd_button;
extern HWND hwnd_help;
extern HWND hwnd_modeless;		/* any modeless dialog box */
extern HWND hptr_crosshair;
extern POINTL buttonbar;
extern POINTL statusbar;
extern POINTL info_file;
extern POINTL info_page;
extern RECTL info_coord;
extern BOOL waiting;
extern TID queue_tid;
extern TID term_tid;
VOID APIENTRY queue_thread(ULONG unused);
VOID APIENTRY term_thread(ULONG unused);
MRESULT EXPENTRY ClientWndProc(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY FrameWndProc(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY StatusWndProc(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY ButtonWndProc(HWND, ULONG, MPARAM, MPARAM);
extern PFNWP OldFrameWndProc;


extern PROG gsprog;
extern PROG pdfconv;
extern OPTIONS option;
extern PSFILE psfile; 
extern GVIEW gsview;
extern DISPLAY display;
extern PRINTER printer;
extern BMAP bitmap;
extern PSDOC *doc;
extern int page_extra;
extern int page_skip;
extern BOOL changed_version;
extern BOOL zoom;

extern PSBBOX bbox;

/* in gvpm.c */
void copy_clipboard(void);
void update_scroll_bars(void);
BOOL get_cursorpos(float *x, float *y);

/* in gvpinit.c */
APIRET gsview_init(int argc, char *argv[]);
void show_buttons(void);
char * install_default(int id);

/* in gvcmisc.c */
void error_message(char *str);
void info_init(HWND hwnd);
void read_profile(void);
void write_profile(void);

/* in gvpmisc.c */
BOOL SetDlgItemText(HWND hwnd, int id, char *str);
void post_close(void);
void get_help(void);
int message_box(char *str, int icon);
void check_menu_item(int menuid, int itemid, BOOL checked);
int get_menu_string(int menuid, int itemid, char *str, int len);
int load_string(int id, char *str, int len);
void play_system_sound(char *id);
void play_sound(int i);
void info_wait(int id);
void send_prolog(FILE *f, int resource);
void profile_create_section(PROFILE *prf, char *section, int id);
int gs_chdir(char *dirname);
char * gs_getcwd(char *dirname, int size);

/* in gvcdisp.c */
void transform_cursorpos(float *x, float *y);
void transform_point(float *x, float *y);
void itransform_point(float *x, float *y);
int get_paper_size_index(void);
void gs_size(void);
void gs_resize(void);
void gs_magnify(float scale);
void gsview_orientation(int new_orientation);
void gsview_media(int new_media);
void gsview_unit(int new_unit);
void gsview_endfile(void);
void gsview_openfile(char *filename);
void gsview_select(void);
void gsview_selectfile(char *filename);
void gsview_display(void);
void gsview_displayfile(char *filename);
void send_orientation_prolog(FILE *f);
void send_epswarn_prolog(FILE *f);
void fix_orientation(FILE *f);
FILE * gp_open_scratch_file(const char *prefix, char *fname, const char *mode);
BOOL dfreopen(void);
void dfclose(void);
BOOL dsc_scan(char *filename);
void dsc_getpages(FILE *f, int first, int last);
void dsc_header(FILE *f);
void dsc_dopage(void);
void dsc_skip(int skip);
int map_page(int page);
BOOL do_output(void);

/* in gvpdisp.c */
BOOL exec_pgm(char *name, char *arg, BOOL withpipe, PROG* prog);
void stop_pgm(PROG* prog);
void cleanup_pgm(PROG *prog);
BOOL pdf_convert(char *name, char *arg, PROG* prog);
BOOL gs_open(void);
BOOL gs_close(void);
void next_page(void);
BOOL psfile_changed(void);
void psfile_savestat(void);
BOOL is_pipe_done(void);	/* true if pipe has just been reset */

/* in gvccmd.c */
int gsview_command(int command);
BOOL not_open(void);
BOOL not_dsc(void);
void gserror(UINT id, char *str, UINT icon, int sound);
void pserror(char *str);
int not_implemented(void);
void gsview_check_usersize(void);
int gsview_depth_to_menu(int depth);

/* in gvpdlg.c */
BOOL get_filename(char *filename, BOOL save, int filter, int title, int help);
BOOL get_string(char *, char *);
BOOL get_page(int *, BOOL);
void show_info(void);
void show_about(void);
BOOL get_bbox(void);
void change_sounds(void);
void gs_showmess(void);
int gs_addmess(char *str, int count);

/* in gvceps.c */
void make_eps_metafile(void);
void ps_to_eps(void);
void paste_to_file(void);
void clip_convert(void);

/* in gvcprn.c */
void gsview_spool(char *, char *);
void psfile_extract(FILE *f);
char *get_devices(void);
void print_cleanup(void);
void gsview_saveas(void);
void gsview_extract(void);
struct prop_item_s * get_properties(char *device);
BOOL gsview_cprint(BOOL to_file, char *cfname, char *optfname);

/* in gvpprn.c */
#ifndef NERR_BufTooSmall
#define NERR_BufTooSmall 2123	/* for SplEnumQueue */
#endif
BOOL get_portname(char *portname, char *port);
int gp_printfile(char *filename, char *port);
void gsview_print(BOOL);
extern char not_defined[];

/* in gvctext.c */
void gsview_text_extract(void);
void gsview_text_find(void);
void gsview_text_findnext(void);

#endif
