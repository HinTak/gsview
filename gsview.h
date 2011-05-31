/*
 * gsview.h -- Include file for GSVIEW.EXE 
 * Copyright (C) 1993  Russell Lang
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Author: Russell Lang
 * Internet: rjl@monu1.cc.monash.edu.au
 */

#define GSVIEW_VERSION "1.0a  1993-11-10"

#define ID_ANSWER	51
#define ID_PROMPT	52
#define ID_HELP		53

#define IDM_OPEN	101
#define IDM_CLOSE	102
#define IDM_NEXT	103
#define IDM_NEXTSKIP	104
#define IDM_REDISPLAY   105
#define IDM_PREV	106
#define IDM_PREVSKIP	107
#define IDM_GOTO	108
#define IDM_INFO	109
#define IDM_SELECT	110
#define IDM_PRINT	111
#define IDM_PRINTTOFILE 112
#define IDM_SPOOL	113
#define IDM_EXTRACT	114
#define IDM_PSTOEPS	115
#define IDM_EXIT	116
#define IDM_DROP	117
#define IDM_SKIP	118

#define IDM_COPYCLIP	151
#define IDM_PASTETO	152
#define IDM_CONVERT	153
#define IDM_MAKEEPSI	154
#define IDM_MAKEEPST4	155
#define IDM_MAKEEPST	156
#define IDM_MAKEEPSW	157
#define IDM_EXTRACTPS	158
#define IDM_EXTRACTPRE	159

#define IDM_GSCOMMAND	  175
#define IDM_SOUNDS	  176
#define IDM_SETTINGS	  177
#define IDM_SAVESETTINGS  178
#define IDM_SAFER         179  
#define IDM_SAVEDIR	  180
#define IDM_BUTTONSHOW	  181
#define IDM_QUICK	  182
#define IDM_AUTOREDISPLAY 183
#define IDM_EPSFCLIP	  184
#define IDM_EPSFWARN	  185

#define IDM_PORTRAIT	201
#define IDM_LANDSCAPE   202
#define IDM_UPSIDEDOWN  203
#define IDM_SEASCAPE	204
#define IDM_SWAPLANDSCAPE 205

#define IDM_RESOLUTION  251

#define IDM_LETTER	301
#define IDM_LETTERSMALL	302
#define IDM_TABLOID	303
#define IDM_LEDGER	304
#define IDM_LEGAL	305
#define IDM_STATEMENT	306
#define IDM_EXECUTIVE	307
#define IDM_A3		308
#define IDM_A4		309
#define IDM_A4SMALL	310
#define IDM_A5		311
#define IDM_B4		312
#define IDM_B5		313
#define IDM_FOLIO	314
#define IDM_QUARTO	315
#define IDM_10X14	316
#define IDM_USERSIZE	317
#define IDM_MEDIALAST	318

#define IDM_HELPCONTENT 351
#define IDM_HELPSEARCH	352
#define IDM_ABOUT	353

#define INFO_FILE	401
#define INFO_TYPE	402
#define INFO_TITLE	403
#define INFO_DATE	404
#define INFO_BBOX	405
#define INFO_ORIENT	406
#define INFO_ORDER	407
#define INFO_DEFMEDIA	408
#define INFO_NUMPAGES	409
#define INFO_PAGE	410
#define INFO_BITMAP	411
#define INFO_ICON	412

#define ABOUT_ICON	451
#define ABOUT_VERSION	452

#define SOUND_EVENT	501
#define SOUND_FILE	502
#define SOUND_PATH	503
#define SOUND_TEST	504

#define SPOOL_PORT	525

#define CANCEL_PCDONE	541

#define PAGE_LIST	551
#define PAGE_ALL	552
#define PAGE_ODD	553
#define PAGE_EVEN	554

#define DEVICE_NAME	561
#define DEVICE_RES	562
#define DEVICE_RESTEXT	563
#define DEVICE_PROP	564

#define PROP_NAME	571
#define PROP_VALUE	572

#define EPS_INTERCHANGE	581
#define EPS_WINDOWS	582
#define EPS_TIFF	583

#define BB_PROMPT	591
#define BB_CLICK	592

#define FILTER_PS	1
#define FILTER_EPS	2
#define FILTER_EPI	3
#define FILTER_ALL	4
#define FILTER_BMP	5
#define FILTER_TIFF	6
#define FILTER_WMF	7
#define IDS_FILTER	601
#define IDS_TITLE	602
#define IDS_HELPFILE	603
#define IDS_WRONGGS	604
#define IDS_BUSY	605

#define IDS_FILE	610
#define IDS_NOFILE	611
#define IDS_PAGE	612
#define IDS_NOPAGE	613
#define IDS_LANDSCAPE	614
#define IDS_PORTRAIT	615
#define IDS_ASCEND	616
#define IDS_DESCEND	617
#define IDS_SPECIAL	618
#define IDS_EPSF	619
#define IDS_EPSI	620
#define IDS_EPST	621
#define IDS_EPSW	622
#define IDS_DSC		623
#define IDS_NOTDSC	624
#define IDS_PAGEINFO	625

#define IDS_OUTPUTFILE	630
#define IDS_PRINTINGALL	631
#define IDS_PRINTFILE	632
#define IDS_NOSPOOL	633
#define IDS_SELECTPAGE	634
#define IDS_SELECTPAGES	635
#define IDS_TIMEOUT	636
#define IDS_NOTIMER	637
#define IDS_NOTOPEN	638
#define IDS_CANNOTRUN	639
#define IDS_TOOLONG	640
#define IDS_WAIT	641
#define IDS_NOMORE	642
#define IDS_GSCOMMAND	643
#define IDS_RES		644
#define	IDS_USERWIDTH	645
#define	IDS_USERHEIGHT	646
#define IDS_BADEPS	647
#define IDS_NOPREVIEW	648
#define IDS_NOTDFNAME   649
#define IDS_PIPEERR	650
#define IDS_CANCELDONE	651
#define IDS_BADCLI      652

#define IDS_SOUNDNAME	670
#define IDS_SNDPAGE	671
#define IDS_SNDNOPAGE	672
#define IDS_SNDNONUMBER 673
#define IDS_SNDNOTOPEN	674
#define IDS_SNDERROR	675
#define IDS_SNDTIMEOUT	676
#define IDS_SNDSTART	677
#define IDS_SNDEXIT	678
#define IDS_SOUNDNOMM	679
#define IDS_NONE	680
#define IDS_SPKR	681

/* help topics */
#define IDS_TOPICROOT	701
#define IDS_TOPICOPEN	702
#define IDS_TOPICPRINT	703
#define IDS_TOPICEDIT	704
#define IDS_TOPICGSCMD	705
#define IDS_TOPICSOUND	706
#define IDS_TOPICMEDIA  707
#define IDS_TOPICPSTOEPS 708

/* ps_to_eps */
#define IDS_BBPROMPT	750
#define IDS_BBPROMPT1	751
#define IDS_BBPROMPT2	752
#define IDS_BBPROMPT3	753
#define IDS_EPSONEPAGE	754
#define IDS_EPSQPAGES	755
#define IDS_EPSNOBBOX	756
#define IDS_EPSREAD     757

/* now the stuff that the resource compiler shouldn't see */
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
/* from gsview to gswin image window */
#define NEXT_PAGE	10
#define COPY_CLIPBOARD	11
/* from gsview to gswin text window */
#define PIPE_DATA	12

#define MAXSTR 80
#define PROFILE_SIZE 2048
#define DEFAULT_GSCOMMAND "gswin"
#define DEFAULT_RESOLUTION 96.0
#define INIFILE "gsview.ini"
#define INISECTION "Options"
#define DEVSECTION "Devices"

extern char szAppName[MAXSTR];		/* application name - for title bar */
extern const char szClassName[];	/* window class */
extern const char szScratch[];		/* temporary filename prefix */

/* initialised in init.c */
extern BOOL is_win31;			/* To allow selective use of win 3.1 features */
extern char szHelpName[MAXSTR];		/* buffer for building help filename */
extern char szHelpTopic[48];		/* topic for OFN_SHOWHELP */
extern UINT help_message;		/* message sent by OFN_SHOWHELP */
extern HMENU hmenu;			/* main menu */
extern HACCEL haccel;			/* menu accelerators */
extern HCURSOR hcWait;
extern POINT img_offset;		/* offset to gswin child window */
extern POINT info_file;			/* position of file information */
extern POINT info_page;			/* position of page information */
extern RECT  info_rect;			/* position and size of brief info area */
extern RECT  info_coord;		/* position and size of coordinate information */
extern RECT  button_rect;		/* position and size of button area */

extern HINSTANCE phInstance;		/* instance of gsview */
extern HINSTANCE gswin_hinst;		/* instance of gswin */
extern HWND hwndimg;			/* gsview main window */
extern HWND hDlgModeless;		/* any modeless dialog box */
extern HWND hwndtext;			/* gswin text window */
extern HWND hwndimgchild;		/* gswin image child window */
extern int bitmap_scrollx;		/* offset from bitmap to origin of child window */
extern int bitmap_scrolly;
extern int bitmap_width;		/* size of gswin bitmap in pixels */
extern int bitmap_height;

extern struct document *doc;		/* DSC structure.  NULL if not DSC */
extern int pagenum;			/* current page number */
extern char dfname[MAXSTR];		/* name of selected document file */
extern FILE *dfile;			/* selected file */
extern FILE *cfile;			/* command file */
extern char efname[MAXSTR];		/* name of temp file extracted from DOS EPS file */
extern char pcfname[MAXSTR];		/* name of temp command file for printing */
extern char pfname[MAXSTR];		/* name of temp file for printing options */
extern BOOL is_ctrld;			/* TRUE if DSC except for ctrl+D at start of file */
extern int preview;			/* preview type IDS_EPSF, IDS_EPSI, etc. */
extern BOOL page_ready;			/* true when gswin has sent an OUTPUT_PAGE and is waiting for NEXT_PAGE */
extern BOOL page_extra;			/* extra pages to display */
extern BOOL at_prompt;			/* true if at prompt */
extern BOOL saved;			/* true if interpreter state currently saved in /gssave */
extern int bitmap_scrollx;		/* offset from bitmap to origin of child window */
extern int bitmap_scrolly;
extern OPENFILENAME ofn;
extern char szOFilename[MAXSTR];	/* filename for OFN */
#define DEFAULT_TIMEOUT 300		/* 300 seconds = 5 minutes per page */
#define CLOSE_TIMEOUT    20
extern BOOL bTimeout;			/* true if timeout occured */
extern WNDPROC lpfnButtonWndProc;	/* default button WndProc */
extern HINSTANCE hlib_mmsystem;		/* DLL containing sndPlaySound function */
typedef BOOL (WINAPI *FPSPS)(LPCSTR, UINT);
extern FPSPS lpfnSndPlaySound;		/* pointer to sndPlaySound function if loaded */
extern BOOL epsf_clipped;		/* clipping this page? */
#define BEEP "beep"			/* profile entry for a speaker beep */
extern BOOL debug;			/* /D command line option used */
struct page_list_s {
	int current;	/* index of current selection */
	BOOL multiple;	/* true if multiple selection allowed */
	BOOL *select;	/* array of selection flags */
};
extern struct page_list_s page_list;

/* these can be saved in the INI file */
extern char szGSwin[128];
extern POINT img_origin;
extern POINT img_size;
extern BOOL quick;
extern BOOL settings;
extern BOOL button_show;
extern BOOL safer;
extern int media;
extern char medianame[32];
extern int user_width, user_height;
extern BOOL epsf_clip;
extern BOOL epsf_warn;
extern BOOL redisplay;
extern int orientation;
extern BOOL swap_landscape;
extern float xdpi, ydpi;
extern UINT timeout;
extern BOOL save_dir;
extern char device_name[32];
extern char device_resolution[32];
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

/* in gsview.c */
BOOL get_cursorpos(int *x, int *y);
void info_wait(BOOL);
void play_sound(int);
BOOL set_timer(UINT);
void clear_timer(void);
void gserror(UINT, char *, UINT, int);
void pserror(char *);
LRESULT CALLBACK _export MenuButtonProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK _export WndImgProc(HWND, UINT, WPARAM, LPARAM);

/* in init.c */
void gsview_init0(LPSTR);
void gsview_init1(LPSTR);
void gsview_create(void);
void show_buttons(void);
void read_profile(void);
void write_profile(void);

/* in display.c */
int get_papersizes_index(void);
void gswin_size(void);
void gswin_resize(void);
void gsview_orientation(int);
void gsview_media(int);
int gswin_open(void);
int gswin_close(void);
void next_page(void);
void do_message(void);
void gsview_endfile(void);
void gsview_openfile(char *);
void gsview_select(void);
void gsview_selectfile(char *);
void gsview_display(void);
void gsview_displayfile(char *);
void fix_orientation(FILE *f);
FILE * gp_open_scratch_file(const char *prefix, char *fname, const char *mode);
BOOL dfreopen(void);
void dfclose(void);
BOOL dsc_scan(char *fname);
void dsc_getpages(FILE *f, int first, int last);
void dsc_header(FILE *f);
void dsc_dopage(void);
void dsc_skip(int);
int map_page(int page);

/* in dialog.c */
#define OPEN FALSE
#define SAVE TRUE
BOOL getfilename(LPSTR, BOOL, UINT, UINT, UINT);
BOOL get_string(char *, char *);
BOOL CALLBACK _export InputDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK _export AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK _export SoundDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK _export InfoDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL get_page(int *, BOOL);
BOOL CALLBACK _export PageDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL gsview_usersize(void);

/* in pipe.c */
void pipeinit(void);	/* prepare pipe for opening */
FILE *pipeopen(void);	/* open pipe for first time */
void pipeclose(void);	/* finished with pipe, close & delete temp files */
void pipereset(void);	/* pipe is empty, do cleanup */
void piperequest(void);	/* request from gswin for pipe data */
void pipeflush(void);	/* start sending data through pipe */
BOOL is_pipe_done(void);	/* true if pipe has just been reset */

/* in print.c */
BOOL CALLBACK _export DeviceDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK _export CancelDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK _export SpoolDlgProc(HWND, UINT, WPARAM, LPARAM);
void gsview_spool(char *, char *);
void pscopydoc(FILE *fp);
char *get_devices(void);
void print_cleanup(void);
void gsview_print(BOOL);
void gsview_extract(void);

/* clip.c */
LONG dib_bytewidth(LPBITMAPINFOHEADER);
UINT dib_pal_colors(LPBITMAPINFOHEADER);
void clip_to_file(void);
void clip_convert(void);
void clip_add_palette(void);
void clip_add_ddb(void);
void clip_add_dib(void);
void extract_eps(void);
void extract_doseps(WORD);
void make_eps_metafile(void);
void make_eps_tiff(WORD);
void make_eps_interchange(void);
void ps_to_eps(void);

#ifdef WIN32
/* Windows NT has never been tried, but these macros fix up the */
 * known differences from Windows 3.1 */
/* imitation pipes will need to be rewritten to use real pipes */
#define MoveTo(hdc,x,y) MoveToEx((hdc),(x),(y),(LPPOINT)NULL)
#define SetClassCursor(hwnd, hcursor) SetClassLong((hwnd), GCL_HCURSOR, (LONG)(hcursor))
#define GetClassCursor(hwnd) ((HCURSOR)GetClassLong((hwnd), GCL_HCURSOR))
#define GetNotification(wParam,lParam) (HIWORD(wParam))
#define SendDlgNotification(hwnd, id, notice) \
    SendMessage((hwnd), WM_COMMAND, MAKELONG((id),(notice)), GetDlgItem((hwnd),(id)))
#else
#define SetClassCursor(hwnd, hcursor) SetClassWord(hwnd, GCW_HCURSOR, (WORD)(hcursor))
#define GetClassCursor(hwnd) ((HCURSOR)GetClassWord(hwnd, GCW_HCURSOR))
#define GetNotification(wParam,lParam) (HIWORD(lParam))
#define SendDlgNotification(hwnd, id, notice) \
    SendMessage((hwnd), WM_COMMAND, id, MAKELPARAM(GetDlgItem((hwnd),(id)),(notice)))
#endif
#endif /* !RC_INVOKED */
