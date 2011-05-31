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
#include <dir.h>
#include <io.h>
#include <time.h>
#define NeedFunctionPrototypes 1
#include "ps.h"
#include "gvcrc.h"


#ifndef RC_INVOKED

/* messages used between gsview and gswin */
#define WM_GSVIEW WM_USER+0
/* from gswin to gsview */
#define HWND_TEXT	0
#define HWND_IMGCHILD	1
#define GSWIN_CLOSE	2
#define SYNC_OUTPUT	3
#define OUTPUT_PAGE	4
#define SCROLL_POSITION 5
#define PIPE_REQUEST	6
#define BEGIN		7
#define END		8
/* from gsview to gswin image window */
#define NEXT_PAGE	10
#define COPY_CLIPBOARD	11
/* from gsview to gswin text window */
#define PIPE_DATA	12


#define MAXSTR 80	/* maximum file name length and general string length */
#define PROFILE_SIZE 2048
#ifdef WIN32
#define DEFAULT_GSCOMMAND "gswin32.exe"
#define INIFILE "gsview32.ini"
#else
#define DEFAULT_GSCOMMAND "gswin"
#define INIFILE "gsview.ini"
#endif
#define DEFAULT_RESOLUTION 96.0
#define DEFAULT_ZOOMRES 300.0
#define INISECTION "Options"
#define DEVSECTION "Devices"
#define EOLSTR "\r\n"
#define COPY_BUF_SIZE 4096
/* don't have to worry about segments/selectors */
#define GVFAR FAR
#define GVHUGE _huge


/* program details */
typedef struct tagPROG {
    BOOL	valid;
    HINSTANCE	hinst;
    FILE	*input;		/* pipe to stdin */
} PROG;

typedef struct document PSDOC;

typedef struct tagPSBBOX {
	int	llx;
	int	lly;
	int	urx;
	int	ury;
	int	valid;
} PSBBOX;

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
	
} PSFILE;

/* options that are saved in INI file */
typedef struct tagOPTIONS {
	char	gscommand[MAXSTR];
	int	gsversion;
	POINT	img_origin;
	POINT	img_size;
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
	BOOL	abort;
	BOOL	busy;
	int	width;
	int	height;
	int	planes;
	int	bitcount;
	BOOL	do_endfile;	
	BOOL	do_resize;
	BOOL	do_display;
	BOOL	epsf_clipped;	/* display is clipped to bbox */
	int	zoom_xoffset;	/* in 1/72" */
	int	zoom_yoffset;
	BOOL	saved;		/* interpreter state saved */
	BOOL	page;		/* GS_PAGE received */
	BOOL	sync;		/* GS_SYNC received */
} DISPLAY;

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
typedef BOOL (WINAPI *FPSPS)(LPCSTR, UINT);
extern HINSTANCE hlib_mmsystem;	/* DLL containing sndPlaySound function */
extern FPSPS lpfnSndPlaySound;	/* pointer to sndPlaySound function if loaded */

extern const char szClassName[];
extern const char szScratch[];  /* temporary filename prefix */
extern char szAppName[MAXSTR];
extern char szHelpTopic[MAXSTR];
extern char szWait[MAXSTR];
extern char szExePath[MAXSTR];
extern char szIniFile[MAXSTR];
extern char szFindText[MAXSTR];
extern char szHelpName[MAXSTR];		/* buffer for building help filename */
extern char previous_filename[MAXSTR];	/* to remember name between file dlg boxes */
extern UINT help_message;		/* message sent by OFN_SHOWHELP */
extern HWND hwndimg;			/* gsview main window */
extern HWND hDlgModeless;		/* any modeless dialog box */
extern HWND hwndtext;			/* gswin text window */
extern HWND hwndimgchild;		/* gswin image child window */
extern HINSTANCE phInstance;		/* instance of gsview */
extern BOOL is_win31;			/* To allow selective use of win 3.1 features */
extern BOOL is_winnt;			/* To allow selective use of Windows NT features */
extern HMENU hmenu;			/* main menu */
extern HACCEL haccel;			/* menu accelerators */
extern HCURSOR hcWait;
extern int bitmap_scrollx;	/* offset from bitmap to origin of child window */
extern int bitmap_scrolly;
extern POINT img_offset;		/* offset to gswin child window */
extern POINT info_file;		/* position of file information */
extern POINT info_page;		/* position of page information */
extern RECT  info_rect;		/* position and size of brief info area */
extern RECT  info_coord;		/* position and size of coordinate information */
extern RECT  button_rect;		/* position and size of button area */
#define CLOSE_TIMEOUT    20	/* 20 seconds */
extern OPENFILENAME ofn;
extern BOOL waiting;		/* true when 'wait' to be displayed in info area */
extern BOOL bTimeout;		/* true if timeout occured */
extern WNDPROC lpfnButtonWndProc;

extern PROG gsprog;
extern OPTIONS option;
extern PSFILE psfile; 
extern DISPLAY display;
extern PSDOC *doc;
extern int page_extra;
extern int page_skip;
extern BOOL changed_version;
extern BOOL zoom;

extern PSBBOX bbox;


#ifdef __WIN32__
#define _huge
#define MoveTo(hdc,x,y) MoveToEx((hdc),(x),(y),(LPPOINT)NULL)
#define SetWindowOrg(hdc, x, y) SetWindowOrgEx(hdc, x, y, (LPPOINT)NULL);
#define	SetWindowExt(hdc, x, y) SetWindowExtEx(hdc, x, y, (LPSIZE)NULL);
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

/* in gvwin.c */
LRESULT CALLBACK _export MenuButtonProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK _export WndImgProc(HWND, UINT, WPARAM, LPARAM);
BOOL in_child_client_area(void);
BOOL set_timer(UINT);
void clear_timer(void);
void copy_clipboard(void);
BOOL get_cursorpos(float *x, float *y);

/* in gvcmisc.c */
void error_message(char *str);
void info_init(HWND hwnd);
void read_profile(void);
void write_profile(void);

/* in gvwmisc.c */
/* BOOL SetDlgItemText(HWND hwnd, int id, char *str); */
void post_close(void);
void get_help(void);
int message_box(char *str, int icon);
void check_menu_item(int menuid, int itemid, BOOL checked);
int get_menu_string(int menuid, int itemid, char *str, int len);
int load_string(int id, char *str, int len);
void play_sound(int i);
void info_wait(BOOL flag);
int _chdir(char *dirname);
char * _getcwd(char *dirname, int size);
void send_prolog(FILE *f, int resource);
void profile_create_section(PROFILE *prf, char *section, int id);

/* in gvcdisp.c */
void transform_cursorpos(float *x, float *y);
void transform_point(float *x, float *y);
void iransform_point(float *x, float *y);
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

/* in gvwdisp.c */
void do_message(void);
BOOL exec_pgm(char *name, char *arg, char *id, PROG* prog);
void stop_pgm(PROG* prog);
void cleanup_pgm(PROG *prog);
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

/* in gvwdlg.c */
BOOL get_filename(char *filename, BOOL save, int filter, int title, int help);
BOOL get_string(char *, char *);
BOOL get_page(int *, BOOL);
void show_info(void);
void show_about(void);
void change_sounds(void);

/* in gvwpipe.c */
void pipeinit(void);	/* prepare pipe for opening */
FILE *pipeopen(void);	/* open pipe for first time */
void pipeclose(void);	/* finished with pipe, close & delete temp files */
void pipereset(void);	/* pipe is empty, do cleanup */
void piperequest(void);	/* request from gswin for pipe data */
void pipeflush(void);	/* start sending data through pipe */
BOOL is_pipe_done(void);	/* true if pipe has just been reset */

/* in gvwinit.c */
void gsview_init0(LPSTR lpszCmdLine);
void gsview_init1(LPSTR lpszCmdLine);
void gsview_create(void);
void show_buttons(void);

/* in gvcprn.c */
struct prop_item_s * get_properties(char *device);
void gsview_spool(char *, char *);
void psfile_extract(FILE *f);
char *get_devices(void);
void print_cleanup(void);
void gsview_saveas(void);
void gsview_extract(void);
BOOL gsview_cprint(BOOL to_file, char *cfname, char *optfname);

/* in gvwprn.c */
int gp_printfile(char *filename, char *port);
void gsview_print(BOOL);
extern char not_defined[];

/* in gvceps.c */
void extract_doseps(int);
void make_eps_metafile(void);
void make_eps_tiff(int);
void make_eps_interchange(BOOL calc_bbox);
void ps_to_eps(void);
BOOL get_bbox(void);

/* in gvwclip.c */
void paste_to_file(void);
void clip_convert(void);

/* in gvwprf.c */
PROFILE * profile_open(char *filename);
int profile_read_string(PROFILE *prf, char *section, char GVFAR *entry, char *def, char *buffer, int len);
BOOL profile_write_string(PROFILE *prf, char *section, char *entry, char *value);
BOOL profile_close(PROFILE *prf);

/* in gvctext.c */
void gsview_text_extract(void);
void gsview_text_find(void);
void gsview_text_findnext(void);

#endif
