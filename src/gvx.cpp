/* Copyright (C) 2000, Ghostgum Software Pty Ltd.  All rights reserved.
  
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

/* gvx.cpp */
/* Main routines for X11 GSview */
#include "gvx.h"

Display *dpy;
GtkWidget *window;
GtkWidget *main_vbox;
GtkWidget *menubar;
GtkWidget *buttonbar;
GtkWidget *scroll_window;
GtkWidget *img;
GtkWidget *statusbar;
GtkWidget *statusfile;
GtkWidget *statuscoord;
GtkWidget *statuspage;
GdkPixmap *pixmap = NULL;
char *pszLocale;
int pixmap_width;
int pixmap_height;
BOOL have_selection;

Atom ghostview_atom;
Atom page_atom;
Atom done_atom;
Atom next_atom;
Window gs_window;
int gs_pipe_stdin[2] = {-1, -1};
int gs_pipe_stdout[2] = {-1, -1};
int gs_pipe_stderr[2] = {-1, -1};
gint gs_pipe_stdin_tag = -1;
gint gs_pipe_stdout_tag = -1;
gint gs_pipe_stderr_tag = -1;
void stop_stdin(void);
void start_stdin(void);
void stop_stdout(void);
void start_stdout(void);
void stop_stderr(void);
void start_stderr(void);

char szAppName[MAXSTR] = GSVIEW_PRODUCT;  /* application name - for title bar */
int nHelpTopic;
char szWait[MAXSTR];
char szDocPath[MAXSTR] = GSVIEW_DOCPATH;
char szEtcPath[MAXSTR] = GSVIEW_ETCPATH;
char szHelpName[MAXSTR];
char szFindText[MAXSTR];
char szIniFile[MAXSTR];
char previous_filename[MAXSTR];
char selectname[MAXSTR];
const char szScratch[] = "gsvx";	/* temporary filename prefix */

#ifdef NOTUSED
const char *szSpoolPrefix = "%pipe%";	/* GS 6.0 and later should use %pipe% */
#else
const char *szSpoolPrefix = "|";	/* earlier versions don't understand %pipe % */
#endif
char coord_text[64];		/* last text displayed as coordinate */
BOOL multithread;

int on_link;			/* TRUE if we were or are over link */
int on_link_page;		/* page number of link target */
long gsbytes_size;		/* number of bytes for this page */
long gsbytes_done;		/* number of byte written */
BOOL quitnow = FALSE;		/* Used to cause exit from nested message loops */

int percent_done;		/* percentage of document processed */
int percent_pending;		/* TRUE if WM_GSPERCENT is pending */

PSFILE psfile;		/* Postscript file structure */
OPTIONS option;		/* GSview options (saved in INI file) */
DISPLAY display;	/* Display parameters */
PRINTER printer;	/* Printer GS parameters */
char last_files[4][MAXSTR];	/* last 4 files used */
GtkWidget *last_file_widget[4];
int last_files_count;		/* number of files known */
HISTORY history;		/* history of pages displayed */
BOOL fullscreen = FALSE;
int command_on_done;		/* execute this command when GS finishes */

int page_skip = 5;		/* number of pages to skip in IDM_NEXTSKIP or IDM_PREVSKIP */
BOOL zoom = FALSE;		/* true if display zoomed */
BOOL print_silent = FALSE;	/* /P or /F command line option used */
BOOL print_exit = FALSE;	/* exit on completion of printing */
int print_count = 0;		/* number of current print jobs */
				/* It is safe to exit GSview when this is 0 */
int disable_gsview_wcmd;
BOOL getting_bbox;		/* PS to EPS get Bounding Box dialog is shown */
int debug = 0;			/* /D command line option used */
struct sound_s sound[NUMSOUND] = {
	{"SoundOutputPage", IDS_SNDPAGE, ""},
	{"SoundNoPage", IDS_SNDNOPAGE, BEEP},
	{"SoundNoNumbering", IDS_SNDNONUMBER, ""},
	{"SoundNotOpen", IDS_SNDNOTOPEN, ""},
	{"SoundError", IDS_SNDERROR, BEEP},
	{"SoundStart", IDS_SNDSTART, ""},
	{"SoundExit", IDS_SNDEXIT, ""},
	{"SoundBusy", IDS_SNDBUSY, BEEP},
};
FILE *pstotextOutfile;

int gargc;	/* for delayed parsing of command line */
char **gargv;
char workdir[MAXSTR];

int flush_stdout(int fd);
int stop_gs(void);
int start_gs(void);
void check_zombie(void);
void map_pt_to_pixel(float *x, float *y);
BOOL get_cursorpos(float *x, float *y);
void statuscoord_update(void);
void highlight_links();
void highlight_words(int first, int last, BOOL marked);
BOOL text_marking = FALSE;
int text_mark_first = -1;
int text_mark_last = -1;
void info_link(void);
void parse_args(int argc, char *argv[]);
void selection_add(void);
void selection_release(void);

void
post_img_message(int message, int param)
{
    if (message == WM_QUIT)
	quit_gsview(window, NULL);
    else if (message == WM_CLOSE)
	quit_gsview(window, NULL);
    else if (message == WM_COMMAND)
	gsview_wcmd(NULL, (gpointer)param);
    else if (message == WM_GSSYNC)
	gtk_widget_draw(img, NULL);
    else if (message == WM_GSWAIT) {
	gs_addmessf("WM_GSWAIT: %s\n", szWait);
	info_wait(param);
    }
    else if (message == WM_GSMESSBOX) {
	delayed_message_box(param, 0);
    }
    else if (message == WM_GSSHOWMESS) {
	gs_showmess();
    }
    else if (message == WM_GSREDISPLAY) {
	gsview_command(IDM_REDISPLAY);
    }
    else if (message == WM_GSTITLE) {
	/* update title */
	if (psfile.name[0] != '\0') {
	    char buf[256];
	    char *p;
	    p = strrchr(psfile.name, '/');
	    if (p == NULL)
		p = psfile.name;
	    else
		p++;
	    sprintf(buf, "%s - %s", p, szAppName);
	    gtk_window_set_title(GTK_WINDOW(window), buf);
	}
	else
	    gtk_window_set_title(GTK_WINDOW(window), szAppName);
    }
    else if (message == WM_GSPERCENT) {
	char buf[MAXSTR];
	percent_pending = FALSE;
	load_string(IDS_WAITDRAW_PC, szWait, sizeof(szWait));
	sprintf(buf, szWait, percent_done);
        gtk_label_set_text(GTK_LABEL(statuspage), buf);
	XFlush(dpy);
    }
    else if (message == WM_GSTEXTINDEX) {
	gs_addmess("WM_GSTEXTINDEX not implemented\n");
	make_text_index();
	text_marking = FALSE;
	text_mark_first = text_mark_last = -1;
	selection_release();
    }
    else
	gs_addmessf("Unknown post_img_message %d\n", message);
}


/* On "File | Exit" or window destroy */ 
void quit_gsview( GtkWidget *w,
                         gpointer   data )
{
    stop_gs();
    gtk_main_quit();
}

void clear_img_window(void)
{
    /* unreference pixmap and redraw window */
    if (pixmap) {
	if (debug & DEBUG_GENERAL)
	    gs_addmess("clear_img_window\n");
	gdk_pixmap_unref(pixmap);
	pixmap = NULL;
	gtk_widget_draw(img, NULL);
    }
}

/* Ghostscript sends us client events.
 * PAGE means showpage has been called.
 * DONE means the x11 device is being closed.
 */
gboolean gs_client_event(GtkWidget *widget,
	GdkEventClient *event, gpointer data)
{
    if (event->type != GDK_CLIENT_EVENT)
	return FALSE;

    if (event->message_type == page_atom) {
	gsdll.state = PAGE;
	gs_addmess("PAGE event\n");
        gs_window = event->data.l[0];
	flush_stdout(gs_pipe_stdout[0]);
	flush_stdout(gs_pipe_stderr[0]);
	if (display.show_find)
	    scroll_to_find();
	gtk_widget_draw(img, NULL);
	if (gsdll.input_index < gsdll.input_count)
	    gsdll.input[gsdll.input_index].seek = TRUE; 
	info_wait(IDS_NOWAIT);
	selection_release();
    }
    else if (event->message_type == done_atom) {
	gs_addmess("DONE event\n");
        gs_window = event->data.l[0];
	stop_gs();
    }
    else if (event->message_type == next_atom) {
	/* shouldn't happen - we send these events to Ghostscript */
	gs_addmess("NEXT event\n");
    }
    else 
	gs_addmessf("Unknown atom %d\n", (int)event->message_type);
    return FALSE;
}

/* Asynchronous write to GS stdin.
 * This is called from event loop when a write is possible on gs_stdin_pipe.
 */
/* Need to change this to read sections of a page */
/* read from gsdll.buffer if available, otherwise read from file into buffer */
void write_fn(gpointer data, gint fd, GdkInputCondition condition)
{
    if (fd != gs_pipe_stdin[1]) {
	if (debug & DEBUG_GENERAL)
	    gs_addmess("write_fn: called with wrong source\n");
	return;
    }
    if (psfile.file == NULL) {
	if (!psfile.ispdf) {
	    if (debug & DEBUG_GENERAL)
		gs_addmess("write_fn: psfile.file is closed\n");
	    stop_stdin();
	    return;
	}
    }
    if (gs_pipe_stdin[1] == -1) {
	if (debug & DEBUG_GENERAL)
	    gs_addmess("write_fn: gs_pipe_stdin[1] is closed\n");
	return;
    }
    if (condition & GDK_INPUT_EXCEPTION) {
	/* complain */
	if (debug & DEBUG_GENERAL)
	    gs_addmess("write_fn: exception\n");
	close(gs_pipe_stdin[1]);
	gs_pipe_stdin[1] = -1;
	return;
    }
    else if (condition & GDK_INPUT_WRITE) {
        int pcdone;
	int bytes_written = 0;
/*
	if (debug & DEBUG_GENERAL)
	    gs_addmess("write_fn: write\n");
*/
	do {
	    if (gsdll.buffer_count == 0) {
		/* read some more */
		gsdll.buffer_index = 0;
		gsdll.buffer_count = get_gs_input((char *)gsdll.buffer, 
			gsdll.buffer_length);
		if ((gsdll.buffer_count == 0) && gsdll.send_eps_showpage) {
	    	    gsdll.send_eps_showpage = FALSE;
		    gs_printf("\nsystemdict /showpage get exec\n");
		}
	    }
	    if (gsdll.buffer_count == 0) {
		stop_stdin();
		dfclose();
		if (psfile.dsc == (CDSC *)NULL) {
		    /* non-DSC, close stdin to close Ghostscript */
		    if (debug & DEBUG_GENERAL)
		        gs_addmess("write_fn: EOF for non-DSC, closing stdin\n");
		    close_gs_stdin();
		    gsdll.state = IDLE;
		    psfile.pagenum = 1;
		}
		break;
	    }
	    
	    bytes_written = write(gs_pipe_stdin[1], 
		gsdll.buffer+gsdll.buffer_index, gsdll.buffer_count);
	    if (psfile.dsc == (CDSC *)NULL)
		info_wait(IDS_WAITDRAW);
	    /* for debugging, log all output sent to Ghostscript */
	    if (debug & DEBUG_GSINPUT)
		gs_addmess_count((char *)(gsdll.buffer+gsdll.buffer_index), 
			bytes_written);
	    if (bytes_written == -1) {
		if (errno == EAGAIN) {
		    break;	/* come back later */
		}
		else {
		    if (debug & DEBUG_GENERAL)
			gs_addmessf("write_fn: write to GS failed, errno=%d\n", errno);
		}
	    }
	    else if (bytes_written == 0) {
		/* pipe probably closed */
		check_zombie();
	    }
	    else {
		gsdll.buffer_count -= bytes_written;
		if (gsdll.buffer_count == 0)
		   gsdll.buffer_index = 0;
		else
		   gsdll.buffer_index += bytes_written;

		if (psfile.file) {
		    gsbytes_done += bytes_written;
		    pcdone = (int)(gsbytes_done * 100 / gsbytes_size);
		    if ((pcdone != percent_done) && !percent_pending) {
			percent_done = pcdone;
			percent_pending = TRUE;
			post_img_message(WM_GSPERCENT, 0);
		    }
		}
	    }

	} while (bytes_written > 0);
    }
    else {
	if (debug & DEBUG_GENERAL)
	    gs_addmessf("write_fn: unknown condition %d\n", condition);
    }
}

/* read from stdout or stderr if possible */
/* returns 0 if pipe closed, -1 if nothing available */
int flush_stdout(int fd)
{
    char buf[256];
    int bytes_read = 0;
    if (fd < 0)
	return 0;
    do {
	bytes_read = read(fd, buf, sizeof(buf));
	if (bytes_read == -1) {
	    if (errno == EAGAIN) {
		break;	/* come back later */
	    }
	    else {
		if (debug & DEBUG_GENERAL)
		    gs_addmessf("read_stdout_fn: read from GS failed, errno=%d\n", errno);
	    }
	}
	else if (bytes_read == 0) {
	    if (debug & DEBUG_GENERAL)
		gs_addmessf("read_stdout_fn: read 0 bytes from GS %s\n",
		    (fd == gs_pipe_stdout[0]) ? "stdout" : "stderr");
	}
	else {
	    gs_addmess_count(buf, bytes_read);
	    pdf_checktag(buf, bytes_read);
	}

    } while (bytes_read > 0);

    return bytes_read;
}

/* Asynchronous read of GS stdout/stderr.
 * This is called from event loop when a read is possible on gs_stdout_pipe.
 */
void read_stdout_fn(gpointer data, gint fd, GdkInputCondition condition)
{
    int is_stdout = (fd == gs_pipe_stdout[0]);
    int is_stderr = (fd == gs_pipe_stderr[0]);

    if (!is_stdout && !is_stderr) {
	if (debug & DEBUG_GENERAL)
	    gs_addmess("read_stdout_fn: called with wrong source\n");
	return;
    }

    if (condition & GDK_INPUT_EXCEPTION) {
	/* complain */
	if (debug & DEBUG_GENERAL)
	    gs_addmess("read_stdout_fn: exception\n");
	if (is_stdout)
	    stop_stdout();
	else
	    stop_stderr();
	return;
    }
    else if (condition & GDK_INPUT_READ) {
	if (flush_stdout(fd) == 0)
	    check_zombie();
    }
    else {
	if (debug & DEBUG_GENERAL)
	    gs_addmessf("read_stdout_fn: unknown condition %d\n", condition);
    }
}

/* Close GS stdin, to encourage it to exit */
void close_gs_stdin(void)
{
    if (gs_pipe_stdin[1] != -1) {
	close(gs_pipe_stdin[1]);
	gs_pipe_stdin[1] = -1;
    }
}

void stop_stdin(void)
{
    if (gs_pipe_stdin_tag >=0) {
        gdk_input_remove(gs_pipe_stdin_tag);
	gs_pipe_stdin_tag = -1;
	if (debug & DEBUG_GENERAL)
	    gs_addmess("stop_stdin:\n");
    }
}

void start_stdin(void)
{
    if (gs_pipe_stdin_tag >=0)
	return;
    gs_pipe_stdin_tag = gdk_input_add(gs_pipe_stdin[1], 
	(enum GdkInputCondition)(GDK_INPUT_WRITE | GDK_INPUT_EXCEPTION),
	write_fn, 0);
    if (debug & DEBUG_GENERAL)
	gs_addmess("start_stdin:\n");
}

void stop_stdout(void)
{
    if (gs_pipe_stdout_tag >=0) {
	flush_stdout(gs_pipe_stdout[0]);
        gdk_input_remove(gs_pipe_stdout_tag);
	gs_pipe_stdout_tag = -1;
	if (debug & DEBUG_GENERAL)
	    gs_addmess("stop_stdout:\n");
    }
}

void start_stdout(void)
{
    if (gs_pipe_stdout_tag >=0)
	return;
    gs_pipe_stdout_tag = gdk_input_add(gs_pipe_stdout[0], 
	(enum GdkInputCondition)(GDK_INPUT_READ | GDK_INPUT_EXCEPTION),
	read_stdout_fn, 0);
    if (debug & DEBUG_GENERAL)
	gs_addmess("start_stdout:\n");
}

void stop_stderr(void)
{
    if (gs_pipe_stderr_tag >=0) {
	flush_stdout(gs_pipe_stdout[1]);
        gdk_input_remove(gs_pipe_stderr_tag);
	gs_pipe_stderr_tag = -1;
	if (debug & DEBUG_GENERAL)
	    gs_addmess("stop_stderr:\n");
    }
}

void start_stderr(void)
{
    if (gs_pipe_stderr_tag >=0)
	return;
    gs_pipe_stderr_tag = gdk_input_add(gs_pipe_stderr[0], 
	(enum GdkInputCondition)(GDK_INPUT_READ | GDK_INPUT_EXCEPTION),
	read_stdout_fn, 0);
    if (debug & DEBUG_GENERAL)
	gs_addmess("start_stderr:\n");
}

/* display messages if GS terminated early */
void stop_gs_messages(int rc, int status)
{
    if (WIFEXITED(status)) {
	/* normal exit */
	if (debug & DEBUG_GENERAL)
	    gs_addmessf("stop_gs_messages: Ghostscript exit code %d\n", 
		WEXITSTATUS(status));
	if (WEXITSTATUS(status) != 0)
	    gs_showmess();	/* show error message */
    }
    else {
	/* someone killed it */
	if (debug & DEBUG_GENERAL)
	    gs_addmessf("stop_gs_messages: child ended, rc=%d, status=%d\n", 
		rc, status); 
    }
}

void close_gs_stdio()
{
    stop_stdin();
    close_gs_stdin();
    stop_stdout();
    if (gs_pipe_stdout[0] != -1) {
	close(gs_pipe_stdout[0]);
	gs_pipe_stdout[0] = -1;
    }
    stop_stderr();
    if (gs_pipe_stderr[0] != -1) {
	close(gs_pipe_stderr[0]);
	gs_pipe_stderr[0] = -1;
    }
}

void check_zombie(void)
{
    if (gsdll.hmodule > 0) {
	/* check if Ghostscript has exited prematurely */
	int rc = 0;
	int status = 0;
	if ( (rc = waitpid(gsdll.hmodule, &status, WNOHANG)) > 0 ) {
	    close_gs_stdio();
	    stop_gs_messages(rc, status);
	    /* Ghostscript has exited, release resources */
	    gsdll.hmodule = 0;
	    if (debug & DEBUG_GENERAL)
		gs_addmess("check_zombie: calling stop_gs()\n");
	    stop_gs();
	    gsdll.buffer_count = gsdll.buffer_index = 0;
	}
    }
}


/* We have a problem with receiving a DONE after stopping Ghostscript.
 * If we restart Ghostscript, we receive this DONE after the new
 * Ghostscript has started, which triggers us to shut the new Ghostscript.
 * Need to get the DONE earlier, or be able to identify which
 * instance of Ghostscript it applied to.
 */

int stop_gs(void)
{
    if (debug & DEBUG_GENERAL)
	gs_addmess("stop_gs\n");
    close_gs_stdio();

    if (gsdll.hmodule > 0) {
	int rc;
	int status = 0;
	/* get termination code */
	usleep(100000);	/* allow a little time for child to finish */
	errno = 0;
	rc = waitpid(gsdll.hmodule, &status, WNOHANG);
	if (debug & DEBUG_GENERAL)
	    gs_addmessf("stop_gs: waitpid rc=%d status=%d errno=%d\n", 
		rc, status, errno);
	if (rc > 0) {
	    if (!pending.unload && !pending.abort)
	        stop_gs_messages(rc, status);
	}
	else if (rc == 0) {
	    /* child has not yet ended */
	    if (debug & DEBUG_GENERAL)
		gs_addmess("stop_gs: killing Ghostscript\n");
	    kill(gsdll.hmodule, SIGTERM);
	    waitpid(gsdll.hmodule, &status, 0);
	}
	else {
	    /* error - no such process */
	    if (debug & DEBUG_GENERAL)
		gs_addmessf("stop_gs: waitpid rc=%d errno=%d\n", rc, errno);
	}
	gsdll.hmodule = 0;

    }

    if (psfile.file != (FILE *)NULL)
	dfclose();

    clear_img_window();
    selection_release();

    gsdll.state = UNLOADED;
    display.init = FALSE;
    display.need_header = TRUE;
    display.need_trailer = FALSE;

    info_wait(IDS_NOWAIT);

    /* Process gtk messages until queue empty, to make sure 
     * DONE event from Ghostscript is flushed. */
    while (gtk_events_pending())
	gtk_main_iteration_do(FALSE);	/* don't block */

    if (command_on_done) {
	if (debug & DEBUG_GENERAL)
	    gs_addmess("Executing deferred command\n");
	gsview_wcmd(NULL, (gpointer)command_on_done);
    }

    return 0;
}

/* Start Ghostscript */
int start_gs(void)
{
    char buf[256];
    char *nargv[10];
    XWindowAttributes attrib;
    int rc;
    int bpixmap = 0;
#ifdef NOTUSED
    int top_margin = 0;		/* see comments below about this */
    int right_margin = 0;	/* see comments below about this */
#endif
    int orientation;
    char safer_string[] = "-dSAFER";
    char nodisplay_string[] = "-dNODISPLAY";
    char nopause_string[] = "-dNOPAUSE";
    char dash_string[] = "-";

    if (pixmap) {
	gdk_pixmap_unref(pixmap);
	pixmap = NULL;
    }
    pixmap_width = display.width;  /* pixels */
    pixmap_height = display.height; /* pixels */
    if ((display.orientation == 1) || (display.orientation == 3)) {
	int temp = pixmap_width;
	pixmap_width = pixmap_height;
	pixmap_height = temp;

#ifdef NOTUSED
	/* Dodge some unit bugs in Ghostscript/Ghostview interface */
	if (pixmap_width > pixmap_height)
	    top_margin = (int)((pixmap_width - pixmap_height) 
				* 72 / display.xdpi);
	else 
	    right_margin = (int)((pixmap_height - pixmap_width) 
			* 72 / display.xdpi);
#endif
    }

    /* change window size */
    gtk_widget_set_usize(img, pixmap_width, pixmap_height);

    if (debug & DEBUG_GENERAL)
	gs_addmessf("start_gs: width=%d height=%d orientation=%d\n", 
		pixmap_width, pixmap_height, display.orientation);
    if (option.drawmethod == IDM_DRAWPIXMAP) {
	/* Ghostscript is to draw on a pixmap */
	/* We are responsible for copying this to the window */
	pixmap = gdk_pixmap_new(img->window, pixmap_width, pixmap_height, -1);
	gdk_draw_rectangle(pixmap, img->style->white_gc,
		TRUE, 0, 0, pixmap_width, pixmap_height);
    }
    else if (option.drawmethod == IDM_DRAWBACKING) {
	/* Create a backing pixmap for Ghostscript */
	pixmap = gdk_pixmap_new(img->window, pixmap_width, pixmap_height, -1);
	gdk_draw_rectangle(pixmap, img->style->white_gc,
		TRUE, 0, 0, pixmap_width, pixmap_height);
	bpixmap = (int)GDK_WINDOW_XWINDOW(pixmap);
    }
    else if (option.drawmethod == IDM_DRAWSTORAGE) {
	/* Ask the X server to provide backing storage.
	 * If the X server ignores this, exposed areas will not be redrawn.
	 * This is a problem with XFree4.
	 */
	XSetWindowAttributes xswa;
	memset(&xswa, 0, sizeof(xswa));
	xswa.backing_store = Always;
	XChangeWindowAttributes(dpy, GDK_WINDOW_XWINDOW(img->window),
		CWBackingStore, &xswa);
	XFlush(dpy);
    }
    else {
	message_box("start_gs: Invalid draw method", 0);
	return 1;
    }

    /* tell Ghostscript which X Window to draw on */
    if (option.drawmethod == IDM_DRAWPIXMAP) {
	sprintf(buf, "%d %d", (int)GDK_WINDOW_XWINDOW(img->window),
		(int)GDK_WINDOW_XWINDOW(pixmap));
    }
    else {
	sprintf(buf, "%d", (int)GDK_WINDOW_XWINDOW(img->window));
    }

    setenv("GHOSTVIEW", buf, TRUE);
    setenv("DISPLAY", XDisplayString(GDK_WINDOW_XDISPLAY(img->window)), TRUE);
    if (debug & DEBUG_GENERAL)
	gs_addmessf("GHOSTVIEW=%s\n", buf);
    if (debug & DEBUG_GENERAL)
	gs_addmessf("DISPLAY=%s\n", XDisplayString(GDK_WINDOW_XDISPLAY(img->window)));

    if (XGetWindowAttributes(dpy, (int)GDK_WINDOW_XWINDOW(img->window),
	    &attrib)) {
	    if (debug & DEBUG_GENERAL)
		gs_addmessf("xwindow width=%d height=%d\n", attrib.width, attrib.height);
    }
    else {
	    gs_addmessf("Can't open get attributes for window %d\n", 
		    (int)GDK_WINDOW_XWINDOW(img->window));
    }


#ifdef NOTUSED
    /* The values for urx, ury and the margins are very strange.
     * They work for portrait, but when the page is rotated using
     * /Orientation, the value of urx is the output device width
     * in pts, not the PostScript page width.  Just to confuse
     * things further, the ImagingBBox is set using urx and ury
     * assuming they are in PostScript page coordinates, 
     * not output device coordinates in pts.
     * To dodge this inconsistency, we need to set the top margin
     * or right margin when rotating the page.
     */

    sprintf(buf, "%d %d %d %d %d %d %g %g %d %d %d %d",
	bpixmap /*bpixmap*/, 
	0 /*orientation*/,
	0 /*llx*/, 0 /*lly*/, 
	(int)(pixmap_width * 72 / display.xdpi),  /* urx */
	(int)(pixmap_height * 72 / display.ydpi), /* ury */
	display.xdpi, display.ydpi,
	0, 0, right_margin, top_margin);
#else
    /* rely on the Ghostview interface for setting page size and
     * offsets, since the Ghostview code predates the setpagedevice
     * implementation Ghostscript and screws up when Orientation
     * is not portrait.
     */
    orientation = display.orientation * 90;
    switch(display.orientation) {
	/* convert from /Orientation as used by setpagedevice
	 * to rotation angle used by Ghostview interface
	 */
	case 0: /* portrait */
	    orientation = 0;
	    break;
	case 1: /* seascape */
	    orientation = 270;
	    break;
	case 2: /* upside-down */
	    orientation = 180;
	    break;
	case 3: /* landscape */
	    orientation = 90;
	    break;
    }
    sprintf(buf, "%d %d %d %d %d %d %g %g %d %d %d %d",
	bpixmap /*bpixmap*/, 
	orientation /*orientation*/,
	(int)(display.xoffset * 72 / display.xdpi) /*llx*/, 
	(int)(display.yoffset * 72 / display.ydpi) /*lly*/, 
	(int)((display.width+display.xoffset) * 72 / display.xdpi),  /* urx */
	(int)((display.height+display.yoffset) * 72 / display.ydpi), /* ury */
	display.xdpi, display.ydpi,
	0, 0, 0, 0);
#endif

    if (debug & DEBUG_GENERAL)
	gs_addmessf("prop=%s\n", buf);
    rc = XChangeProperty(dpy, (int)GDK_WINDOW_XWINDOW(img->window), 
	ghostview_atom, XA_STRING, 8, PropModeReplace,
	(unsigned char *)buf, strlen(buf));
    if (debug & DEBUG_GENERAL)
	gs_addmessf("XChangeProperty returns %d\n", rc);
    XFlush(dpy);

    /* redirect stdio of Ghostscript */
    if (pipe(gs_pipe_stdin)) {
	gs_addmessf("Could not open pipe for stdin, errno=%d\n", errno);
	return 1;
    }
    if (pipe(gs_pipe_stdout)) {
	gs_addmessf("Could not open pipe for stdout, errno=%d\n", errno);
	return 1;
    }
    if (pipe(gs_pipe_stderr)) {
	gs_addmessf("Could not open pipe for stderr, errno=%d\n", errno);
	return 1;
    }

    gsdll.hmodule = fork();
    if (gsdll.hmodule == 0) {
	int j;
	/* child */
	close(gs_pipe_stdin[1]);	/* close write handle */
	dup2(gs_pipe_stdin[0], 0);	/* duplicate and make it stdin */
	close(gs_pipe_stdin[0]);	/* close original read handle */

	close(gs_pipe_stdout[0]);
	dup2(gs_pipe_stdout[1], 1);	/* duplicate and make it stdout */
	close(gs_pipe_stdout[1]);

	close(gs_pipe_stderr[0]);
	dup2(gs_pipe_stderr[1], 2);	/* duplicate and make it stderr */
	close(gs_pipe_stderr[1]);

	/* replace with Ghostscript */
	/* start Ghostscript */
	j = 0;
	nargv[j++] = option.gsdll;
	if (option.safer)
	    nargv[j++] = safer_string;
	nargv[j++] = nodisplay_string;
	nargv[j++] = nopause_string;
	char talpha[MAXSTR];
	char galpha[MAXSTR];
	char include[MAXSTR];
	if (strlen(option.gsinclude) && (strlen(option.gsinclude) < 200)) {
	    sprintf(include, "-I%s", option.gsinclude);
	    nargv[j++] = include;
	}
	if (strlen(option.gsother))
	    nargv[j++] = option.gsother;
	if (option.gsversion >= 600) {
	    if (real_depth(option.depth) >= 8) {
		sprintf(talpha, "-dTextAlphaBits=%d\n", option.alpha_text);
		nargv[j++] = talpha;
		sprintf(galpha, "-dGraphicsAlphaBits=%d\n", option.alpha_graphics);
		nargv[j++] = galpha;
	    }
	}
	nargv[j++] = dash_string;
	nargv[j++] = NULL;
	if (debug & DEBUG_GENERAL) {
	    fprintf(stdout, "child: starting gs\n");
	    for (j=0; nargv[j]; j++)
		fprintf(stdout, "%s ", nargv[j]);
	    fputc('\n', stdout);
	    fflush(stdout);
	}
	if (execvp(nargv[0], nargv) == -1) {
	    int err = errno;
	    /* write to stdout, which will be captured by GSview */
	    fprintf(stdout, "Failed to start Ghostscript process\n");
	    for (j=0; nargv[j]; j++)
		fprintf(stdout, " %s", nargv[j]);
	    fputc('\n', stdout);
	    fprintf(stdout, " errno=%d\n", err);
	    fflush(stdout);
	    /* If we used exit(), it would call the atexit function
	     * registered by gtk, and kill all the parent's windows.
	     * Instead we use _exit() which does not call atexit functions.
	     */
	    _exit(1);
	}
    }
    else {
	/* parent */
	int flags;

	close(gs_pipe_stdin[0]);	/* close read handle */
	gs_pipe_stdin[0] = -1;
	/* make pipe non blocking */
	flags = fcntl(gs_pipe_stdin[1], F_GETFL, 0);
	if (fcntl(gs_pipe_stdin[1], F_SETFL, flags | O_NONBLOCK)) {
	    gs_addmessf("Could not set stdin pipe to non-blocking, errno=%d\n", errno);
/* should close other handles */
	    return 1;
        }

	close(gs_pipe_stdout[1]);	/* close write handle */
	gs_pipe_stdout[1] = -1;
	/* make pipe non blocking */
	flags = fcntl(gs_pipe_stdout[0], F_GETFL, 0);
	if (fcntl(gs_pipe_stdout[0], F_SETFL, flags | O_NONBLOCK)) {
	    gs_addmessf("Could not set stdout pipe to non-blocking, errno=%d\n", errno);
/* should close other handles */
	    return 1;
        }
	start_stdout();

	close(gs_pipe_stderr[1]);	/* close write handle */
	gs_pipe_stdout[1] = -1;
	/* make pipe non blocking */
	flags = fcntl(gs_pipe_stderr[0], F_GETFL, 0);
	if (fcntl(gs_pipe_stderr[0], F_SETFL, flags | O_NONBLOCK)) {
	    gs_addmessf("Could not set stderr pipe to non-blocking, errno=%d\n", errno);
/* should close other handles */
	    return 1;
        }
	start_stderr();

	/* Remember to trigger async write with start_stdin()*/

	if (debug & DEBUG_GENERAL)
	    gs_addmess("parent\n");
	
	gsdll.state = IDLE;
	gsdll.valid = TRUE;
	info_wait(IDS_WAIT);
	
    } 
    return 0;
}

void
send_next_event(void)
{
    if (gsdll.state == BUSY) {
	if (debug & DEBUG_GENERAL)
	    gs_addmess("send_next_event: gsdll.state == BUSY, ignoring\n");
    }
    else if (gsdll.state == UNLOADED) {
	if (debug & DEBUG_GENERAL)
	    gs_addmess("send_next_event: gsdll.state == UNLOADED, ignoring\n");
    }
    else if (gsdll.state == IDLE) {
	if (debug & DEBUG_GENERAL)
	    gs_addmess("send_next_event: gsdll.state == IDLE, shouldn't happen\n");
    }
    else if (gsdll.state == PAGE) {
	if (debug & DEBUG_GENERAL)
	    gs_addmess("send_next_event: sending NEXT event\n");
	XEvent ev; 
	ev.xclient.type = ClientMessage;
	ev.xclient.serial = 0;
	ev.xclient.send_event = TRUE;
	ev.xclient.display = dpy;
	ev.xclient.window = gs_window;
	ev.xclient.message_type = next_atom;
	ev.xclient.format = 32;
	XSendEvent(dpy, gs_window, FALSE, 0, &ev);
	XFlush(dpy);
	gsdll.state = BUSY;
	post_img_message(WM_GSWAIT, IDS_WAITDRAW);
    }
    else {
	gs_addmess("send_next_event: gsdll.state is unknown\n");
    }
}


void set_last_used(void)
{
    GtkWidget *w;
    GtkItem *item;
    GtkBin *bin;
    GtkWidget *label;
    char buf[MAXSTR];
    int i;
    for (i=0; i<4; i++) {
	w = last_file_widget[i];
	if (i < last_files_count) {
	    sprintf(buf, "%s", last_files[i]);
	    if (strlen(buf)>36) {
		int j;
		for (j=strlen(buf); j>0; j--)
		    if ((buf[j] == '/') || (buf[j] == '\\'))
			break;
		if (strlen(buf) - j > 28)
		    memmove(buf+3, buf+strlen(buf)+1-30, 30);
		else {
		    buf[5] = buf[6] = buf[7] = '.';
		    memmove(buf+8, buf+j, strlen(buf)+1-j);
		}
	    }
	    item = &GTK_MENU_ITEM(w)->item;
	    bin = &item->bin;
	    label = bin->child;
	    gtk_label_set_text(GTK_LABEL(label), buf);
	    gtk_widget_show(w);
	}
	else
	    gtk_widget_hide(w);
    }
}

/* enable/disable menu items based on document state */
void set_menu_sensitive(void)
{
/* not implemented yet */
    enable_menu_item(IDM_OPTIONMENU, IDM_SOUNDS, FALSE);
    enable_menu_item(IDM_EDITMENU, IDM_PSTOEDIT, FALSE);
    enable_menu_item(IDM_OPTIONMENU, IDM_CFG, FALSE);
    enable_menu_item(IDM_VIEWMENU, IDM_FULLSCREEN, FALSE);
    enable_menu_item(IDM_HELPMENU, IDM_HELPSEARCH, FALSE);
}

/* callback from menu to gsview_command */
void gsview_wcmd(GtkWidget *w, gpointer data)
{
    int command = (int)data;
    command_on_done = 0;
    if (disable_gsview_wcmd)
       return;

    if (getting_bbox) {
	/* Using the menu is not allowed */
	/* Cancel get_bbox() */
	getting_bbox = FALSE;
	gtk_main_quit();
	return;
    }

    check_zombie();
    if (debug & DEBUG_GENERAL)
	gs_addmessf("gsview_wcmd: gsdll.state=%d\n", gsdll.state);

    gsview_command(command);
    if (debug & DEBUG_GENERAL)
	gs_addmessf("gsview_wcmd: now=%d next=%d unload=%d\n", 
	    pending.now, pending.next, pending.unload);

    while (pending.now || pending.next || pending.unload) {
	if (display.bitcount == 0) {
	    XWindowAttributes attrib;
	    if (XGetWindowAttributes(dpy, 
		(int)GDK_WINDOW_XWINDOW(img->window), &attrib)) {
		if (debug & DEBUG_GENERAL)
		    gs_addmessf("depth=%d\n", attrib.depth);
		display.planes = 1;
		display.bitcount = attrib.depth;
	    }
	    else {
		gs_addmessf("Can't open get attributes for window %d\n", 
		(int)GDK_WINDOW_XWINDOW(img->window));
	    }
	}
    
	gs_process();

	if (gsdll.state == PAGE) {
	    if (psfile.dsc == (CDSC *)NULL)
		psfile.pagenum++;
	    send_next_event();
	}

	if ((gsdll.buffer_count != 0) || 
	     (gsdll.input_index != gsdll.input_count)) {
	    gsdll.state = BUSY;
	}
    }

    set_menu_sensitive();
    switch (command) {
	case IDM_OPEN:
	case IDM_SELECT:
	case IDM_LASTFILE1:
	case IDM_LASTFILE2:
	case IDM_LASTFILE3:
	case IDM_LASTFILE4:
	    set_last_used();	/* update last files */
    }
}

void selection_handle(GtkWidget *widget, GtkSelectionData *selection_data,
	guint info, guint time_stamp, gpointer data)
{
    guchar nulchar = '\0';
    if ( text_index && (text_mark_first != -1) && (text_mark_last != -1)) {
	/* copy text, not image */
	int first, last, line;
	int length;
	int i;
	char * data;
	char *p;
	first = text_mark_first;
	last = text_mark_last;
	if (first > last) {
	    first = text_mark_last;
	    last = text_mark_first;
	}
	line = text_index[first].line;
	length = 1;
	for (i=first; i<=last; i++) {
	    if (text_index[i].line != line) {
	        line = text_index[i].line;
		length += 2;
	    }
	    length += strlen( text_words + text_index[i].word ) + 1;
	}
	data = (char *)malloc(length);
	if (data == (char *)NULL) {
	    message_box("out of memory", 0);
	    gtk_selection_data_set(selection_data, GDK_SELECTION_TYPE_STRING,
		8, &nulchar, 0);
	    return;
	}
	line = text_index[first].line;
	p = data;
	for (i=first; i<=last; i++) {
	    if (text_index[i].line != line) {
	        line = text_index[i].line;
		strcpy(p, "\r\n");
		p += strlen(p);
	    }
	    strcpy(p, text_words + text_index[i].word);
	    strcat(p, " ");
	    p += strlen(p);
	}
        gtk_selection_data_set(selection_data, GDK_SELECTION_TYPE_STRING,
	    8, (guchar *)data, strlen(data));
    }
    else
        gtk_selection_data_set(selection_data, GDK_SELECTION_TYPE_STRING,
	    8, &nulchar, 0);
}

void 
selection_add(void)
{
    gtk_selection_add_target(img, GDK_SELECTION_PRIMARY,
	GDK_SELECTION_TYPE_STRING, 1);
    gtk_signal_connect(GTK_OBJECT(img), "selection_get",
	GTK_SIGNAL_FUNC(selection_handle), NULL);
}

void
selection_release(void)
{
    if (have_selection) {
	if (gdk_selection_owner_get(GDK_SELECTION_PRIMARY) == img->window)
	    gtk_selection_owner_set(NULL, GDK_SELECTION_PRIMARY,
		GDK_CURRENT_TIME);
	have_selection = FALSE;
    }
}

gint
button_release_event(GtkWidget *widget, GdkEventButton *event)
{
    if (event->button==1) {
	text_marking = FALSE;
	if ((text_mark_first != -1) && (text_mark_last != -1)) {
	    /* we have selected something, so claim the X selection */
	    have_selection = gtk_selection_owner_set(widget, 
		GDK_SELECTION_PRIMARY, GDK_CURRENT_TIME);
	}
	else
	    selection_release();
    }
    return TRUE;
}

gint
button_press_event(GtkWidget *widget, GdkEventButton *event)
{
    if (event->button==1) {
	if (getting_bbox)
	    bbox_click();
	else {
	    float x, y;
	    PDFLINK link;
	    int iword ;
	    if (get_cursorpos(&x, &y)) {
		if ( (iword = word_find((int)x, (int)y)) >= 0 ) {
		    /* remove any current selection */
		    highlight_words(text_mark_first, text_mark_last, FALSE);
		    /* mark new selection */
		    text_mark_first = text_mark_last = iword;
		    text_marking = TRUE;
		    highlight_words(text_mark_first, text_mark_last, TRUE);
		}
		else {
		    /* remove selection */
		    highlight_words(text_mark_first, text_mark_last, FALSE);
		    text_mark_first = text_mark_last = -1;
		}
		if (is_link(x, y, &link)) {
		    /* found link */
		    if (link.page == 0)
			gserror(IDS_NOLINKTARGET, NULL, 0, SOUND_ERROR);
		    else {
			gsview_unzoom();
			pending.pagenum = link.page;
			history_add(pending.pagenum);
			pending.now = TRUE;
			gsview_wcmd(NULL, 0);
		    }
		}
		measure_setpoint(x, y);
	    }
	}
    }
#ifdef NOTUSED
    if (event->button==2) {
	/* button 2 not used */
    }
#endif
    if (event->button==3) {
	float x, y;
	int zwidth, zheight;
	int scrollx, scrolly;
	if (get_cursorpos(&x, &y)) {
	    zoom = !zoom;
	    display.zoom_xoffset = (int)x;
	    display.zoom_yoffset = (int)y;
	    scrollx = (int)(gtk_scrolled_window_get_hadjustment(
		GTK_SCROLLED_WINDOW(scroll_window))->value);
	    scrolly = (int)(gtk_scrolled_window_get_vadjustment(
		GTK_SCROLLED_WINDOW(scroll_window))->value);
	    zwidth = scroll_window->allocation.width;
	    zheight = scroll_window->allocation.height;
	    x = (scrollx + zwidth/2)*72.0/option.xdpi;
	    y = (display.height-(scrolly + zheight/2))*72.0/option.ydpi;
	    transform_point(&x, &y);
	    x *= option.xdpi/72.0;
	    y *= option.ydpi/72.0;
	    display.zoom_xoffset -= (int)(x*72.0/option.zoom_xdpi);
	    display.zoom_yoffset -= (int)(y*72.0/option.zoom_ydpi);
	}
	else {
	    zoom = FALSE;
	}
	gsview_wcmd(NULL, (gpointer)IDM_ZOOM);
    }

    return TRUE;
}

BOOL in_img_window = FALSE;

gint
motion_notify_event(GtkWidget *widget, GdkEventMotion *event)
{
    if (event->type == GDK_LEAVE_NOTIFY) {
	in_img_window = FALSE;
    }
    else if (event->type == GDK_MOTION_NOTIFY) {
#ifdef NOTUSED
	int x, y;
	GdkModifierType state;
	/* x,y is pixel offset from top left corner of window */
	if (event->is_hint)
	    gdk_window_get_pointer(event->window, &x, &y, &state);
	else {
	    x = (int)(event->x);
	    y = (int)(event->y);
	    state = (GdkModifierType)event->state;
	}
	gs_addmessf("motion_notify_event: motion %d %d\n", x, y);
#endif
	in_img_window = TRUE;
    }

    float x, y;
    if (get_cursorpos(&x, &y)) {
	PDFLINK link;
	int iword;
	info_link();
	if (is_link(x, y, &link)) {
	    /* should change cursor to a hand */
/* not implemented */
	}
	if (text_marking) {
	    if ( (iword = word_find((int)x, (int)y)) >= 0 ) {
		if (iword != text_mark_last) {
		    int first, last;
		    if ((text_mark_last-text_mark_first >= 0) != (iword-text_mark_first >= 0)) {
			/* changing direction */
			/* clear everything */
			highlight_words(text_mark_first, text_mark_last, FALSE);
			/* reinstate first word */
			text_mark_last = text_mark_first;
			highlight_words(text_mark_first, text_mark_last, TRUE);
		    }
		    if (iword != text_mark_last) {
		      if (iword >= text_mark_first) {
			if (iword > text_mark_last)
			    first=text_mark_last+1, last=iword;
			else
			    first=iword+1, last=text_mark_last;
		      }
		      else {
			if (iword > text_mark_last)
			    first=text_mark_last, last=iword-1;
			else
			    first=iword, last=text_mark_last-1;
		      }
		      highlight_words(first, last, TRUE);
		      text_mark_last = iword;
		    }
		}
	    }
	}
	measure_paint(x, y);
    }

    statuscoord_update();

    return TRUE; 
}

gint
configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data)
{
    return TRUE; 
}

gint
size_event(GtkWidget *widget, GtkAllocation *allocation, gpointer user_data)
{
    /* remember image size */
    option.img_origin.x = window->allocation.x;
    option.img_origin.y = window->allocation.y;
    option.img_size.x = window->allocation.width;
    option.img_size.y = window->allocation.height;
    return TRUE; 
}


gint
expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
    if ((option.drawmethod == IDM_DRAWPIXMAP) ||
        (option.drawmethod == IDM_DRAWBACKING)) {
	if (pixmap) {
	   gdk_draw_pixmap(widget->window,
		widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		pixmap,
		event->area.x, event->area.y,
		event->area.x, event->area.y,
		event->area.width, event->area.height);
	}
	else {
	    /* draw background instead */
	    gdk_draw_rectangle(widget->window, img->style->white_gc, TRUE, 
		event->area.x, event->area.y,
		event->area.width, event->area.height);
	}
    }
    else {
	/* IDM_DRAWSTORAGE */
	/* We are not responsible for drawing */
    }

    if (option.show_bbox && (psfile.dsc != (CDSC *)NULL) &&
	(psfile.dsc->bbox != (CDSCBBOX *)NULL)) {
	int left, right, top, bottom;
	float x, y;
	/* map bounding box to device coordinates */
	x = psfile.dsc->bbox->llx;
	y = psfile.dsc->bbox->lly;
	map_pt_to_pixel(&x, &y);
	left   = (int)x;
	bottom = (int)y;
	x = psfile.dsc->bbox->urx;
	y = psfile.dsc->bbox->ury;
	map_pt_to_pixel(&x, &y);
	right  = (int)x;
	top    = (int)y;
	if (left > right) {
	    int temp = left;
	    left = right;
	    right = temp;
	}
	if (bottom < top) {
	    int temp = top;
	    top = bottom;
	    bottom = temp;
	}
	GdkGC *gcdash = gdk_gc_new(img->window);
	GdkColor black = {0, 0, 0, 0};
#ifdef NOTUSED
/* DOUBLE_DASH didn't seem to work.  Always ended up with solid black */
	GdkColor white = {1, 65535, 65535, 65535};
	gdk_gc_set_fill(gcdash, GDK_SOLID);
	gdk_gc_set_background(gcdash, &white);
	gdk_gc_set_line_attributes(gcdash, 1, GDK_LINE_DOUBLE_DASH,
		GDK_CAP_BUTT, GDK_JOIN_MITER);
#endif
	gdk_gc_set_foreground(gcdash, &black);
	gdk_gc_set_line_attributes(gcdash, 1, GDK_LINE_ON_OFF_DASH,
		GDK_CAP_BUTT, GDK_JOIN_MITER);
        gdk_draw_rectangle(img->window, gcdash, FALSE,
	    left, top, right-left, bottom-top);
	gdk_gc_unref(gcdash);
    }


    /* highlight found search word */
    if (pixmap && display.show_find) {
	float x, y;
	int left, top, bottom, right;
	/* map bounding box to device coordinates */
	x = psfile.text_bbox.llx;
	y = psfile.text_bbox.lly;
	map_pt_to_pixel(&x, &y);
	left   = (int)x;
	bottom = (int)y;
	x = psfile.text_bbox.urx;
	y = psfile.text_bbox.ury;
	map_pt_to_pixel(&x, &y);
	right  = (int)x;
	top    = (int)y;
	if (top > bottom) {
	    int temp = top;
	    top = bottom;
	    bottom = temp;
	}
	if (left > right) {
	    int temp = right;
	    right = left;
	    left = temp;
	}

	if (pixmap) {
	    /* redraw rectangle we about to invert */
	    gdk_draw_pixmap(img->window,
		    img->style->fg_gc[GTK_WIDGET_STATE(img)],
		    pixmap,
		    left, top,
		    left, top,
		    right-left, bottom-top);
	}
	/* invert text */
	GdkGC *gcinvert = gdk_gc_new(img->window);
	gdk_gc_set_function(gcinvert, GDK_INVERT);
	gdk_draw_rectangle(img->window, gcinvert,
		TRUE, left, top, right-left, bottom-top);
	gdk_gc_unref(gcinvert);
    }

    /* highlight marked words */
    highlight_words(text_mark_first, text_mark_last, TRUE);

    highlight_links();

    return FALSE; 
}


/* map from a coordinate in points, to a coordinate in pixels */
/* This is the opposite of the transform part of get_cursorpos */
/* Used when showing bbox */
void
map_pt_to_pixel(float *x, float *y)
{
    if (zoom) {
	/* WARNING - this doesn't cope with EPS Clip */
	*x = (*x - display.zoom_xoffset) * option.zoom_xdpi / 72.0;
	*y = (*y - display.zoom_yoffset) * option.zoom_ydpi / 72.0;
	*x = (*x * 72.0 / option.xdpi);
	*y = (*y * 72.0 / option.ydpi);
	itransform_point(x, y);
	*x = (*x * option.xdpi / 72.0) + display.offset.x;
	*y = -(*y * option.ydpi / 72.0) + (pixmap_height - 1) 
		+ display.offset.y;
    }
    else {
	int xoffset = 0;
	int yoffset = 0;
	if (display.epsf_clipped && (psfile.dsc->bbox!=NULL)) {
	    xoffset = psfile.dsc->bbox->llx;
	    yoffset = psfile.dsc->bbox->lly;
	}
	*x = *x - xoffset;
	*y = *y - yoffset;
	itransform_point(x, y);
	*x = *x * option.xdpi/72.0 + display.offset.x;
	*y = -(*y * option.ydpi/72.0)
	      + (pixmap_height - 1) + display.offset.y;
    }
}

BOOL
get_cursorpos(float *x, float *y)
{
    int ix=0, iy=0;
    GdkModifierType state;
    if (!in_img_window)
	return FALSE;
    gdk_window_get_pointer(img->window, &ix, &iy, &state);
    *x = ix;
    *y = pixmap_height - 1 - iy;
    transform_cursorpos(x, y);
    return TRUE;
}


void
statuscoord_update(void)
{
float x, y;
char buf[64];
char fmt[32];
int digits = option.unitfine ? 2 : 0;
    if ((psfile.name[0] != '\0') && gsdll.hmodule) {
	/* show coordinate */
	if (get_cursorpos(&x, &y)) {
	    switch(option.unit) {
	       case IDM_UNITPT:   
		  sprintf(fmt, "%%.%df, %%.%dfpt", digits, digits);
		  sprintf(buf, fmt, x, y);
		  break;
	       case IDM_UNITMM:   
		  sprintf(fmt, "%%.%df, %%.%dfmm", digits, digits);
		  sprintf(buf, fmt, x/72*25.4, y/72*25.4);
		  break;
	       case IDM_UNITINCH:   
		  digits += 1;
		  sprintf(fmt, "%%.%df, %%.%dfin", digits, digits);
		  sprintf(buf, fmt, x/72, y/72);
		  break;
	    }
	    
	    /* measure_paint(x, y); not implemented */
	}
	else {
	    buf[0] = '\0';
	}
    }
    else {
	buf[0] = '\0';
    }

    if (strcmp(coord_text, buf) != 0) {
	gtk_label_set_text(GTK_LABEL(statuscoord), buf);
	strncpy(coord_text, buf, sizeof(coord_text)-1);
	if (dpy != NULL)
	    XFlush(dpy);
    }
}


/* update the status bar */
void
statusbar_update(void) 
{
    CDSC *dsc = psfile.dsc;
    int i;
    char buf[256];
    char fmt[256];
    char coord[64];
    coord[0] = '\0';
    if (psfile.name[0] != '\0') {
	char *p;
	p = strrchr(psfile.name, '/');
	if (p == NULL)
	    p = psfile.name;
	else
	    p++;
	i = load_string(IDS_FILE, buf, sizeof(buf));
        strncpy(buf+i, p, sizeof(buf)-i-1);
	gtk_label_set_text(GTK_LABEL(statusfile), buf);

	if (szWait[0] != '\0') {
	    sprintf(buf, szWait, percent_done);
	    gtk_label_set_text(GTK_LABEL(statuspage), buf);
	}
	else {
	  if (psfile.dsc!=(CDSC *)NULL) {
	    int n = map_page(psfile.pagenum - 1);
	    load_string(IDS_PAGEINFO, fmt, sizeof(fmt));
	    if (on_link) {
		load_string(IDS_LINKPAGE, fmt, sizeof(fmt));
		sprintf(buf, fmt, on_link_page);
	    }
	    else {
		if (psfile.dsc->page_count)
		    sprintf(buf, fmt, dsc->page[n].label ? 
			dsc->page[n].label : " ", psfile.pagenum,  
			dsc->page_count);
		else
		    sprintf(buf, fmt, " " ,psfile.pagenum,  dsc->page_count);
	    }
	    if (zoom)
		load_string(IDS_ZOOMED, buf+strlen(buf), sizeof(buf)-strlen(buf));
	    gtk_label_set_text(GTK_LABEL(statuspage), buf);
	  }
	  else {
	    if ((gsdll.state == IDLE) || (gsdll.state == UNLOADED))
		load_string(IDS_NOMORE, buf, sizeof(buf));
	    else {
		load_string(IDS_PAGE, buf, sizeof(buf));
		sprintf(buf+i, "%d", psfile.pagenum);
	    }
	    gtk_label_set_text(GTK_LABEL(statuspage), buf);
	  }
	}
    }
    else {
	load_string(IDS_NOFILE, buf, sizeof(buf));
	gtk_label_set_text(GTK_LABEL(statusfile), buf);
	if (szWait[0] != '\0') {
	    sprintf(buf, szWait, percent_done);
	    gtk_label_set_text(GTK_LABEL(statuspage), buf);
	}
	else {
	    gtk_label_set_text(GTK_LABEL(statuspage), "");
	}
    }
    /* show coordinate */
    statuscoord_update();
    if (dpy != NULL)
        XFlush(dpy);
}

void gsview_fullscreen_end(void)
{
/*
    gs_addmess("gsview_fullscreen_end: not implemented\n");
*/
}

void gsview_fullscreen(void)
{
    gs_addmess("gsview_fullscreen: not implemented\n");
}


/* Set the current resolution to fill the window.
 * If neither width nor height match, fit whole page
 * into window.  If either width or height match
 * the window size, fit the height or width respectively.
 */
void 
gsview_fitwin(void) 
{
int window_width, window_height;
int width, height;
float dpi, xdpi, ydpi, xdpi2, ydpi2;
    if (psfile.ispdf) {
/* TESTING TESTING TESTING */
/* This also need to be copied to gvwin.cpp and gvpm.cpp */
	CDSCBBOX *mediabox = NULL;
	CDSCBBOX *cropbox = NULL;
	if (psfile.pagenum < (int)psfile.dsc->page_count) {
	    if (psfile.dsc->page[psfile.pagenum].media)
		mediabox = psfile.dsc->page[psfile.pagenum].media->mediabox;
	    cropbox = psfile.dsc->page[psfile.pagenum].bbox;
	}
	if (option.epsf_clip && (cropbox != (CDSCBBOX *)NULL)) {
	    width = cropbox->urx - cropbox->llx;
	    height = cropbox->ury - cropbox->lly;
	}
	else {
	    if (mediabox) {
		width = mediabox->urx - mediabox->llx;
		height = mediabox->ury - mediabox->lly;
	    }
	    else {
		width = get_paper_width();
		height = get_paper_height();
	    }
	}
    }
    else {
	width = get_paper_width();
	height = get_paper_height();
    }

    if (display.orientation & 1) {
	/* page is rotated 90 degrees */
	int temp = width;
	width = height;
	height = temp;
    }


    if (fullscreen) {
	gs_addmess("gsview_fitwin: fullscreen not implemented\n");
	window_width = scroll_window->allocation.width;
	window_height = scroll_window->allocation.height;
    }
    else {
	/* get size including scroll bars area */
	window_width = scroll_window->allocation.width;
	window_height = scroll_window->allocation.height;
    }
    xdpi = (window_width) * 72.0 / width;
    ydpi = (window_height) * 72.0 / height;
    if (fullscreen) {
	xdpi2 = xdpi;
	ydpi2 = ydpi;
    }
    else {
	/* These are the resolutions allowing for a scroll bar */
/* These do not allow for the extra border provided by the scrolled_window */
/* We don't know how to obtain this extra width */
	xdpi2 = (window_width - 
	    GTK_SCROLLED_WINDOW(scroll_window)->vscrollbar->allocation.width)
	    * 72.0 / width;
	ydpi2 = (window_height - 
	    GTK_SCROLLED_WINDOW(scroll_window)->hscrollbar->allocation.height)
	    * 72.0 / height;
    }

    if (display.orientation & 1) {
	/* page is rotated 90 degrees */
	float ftemp;
	ftemp = xdpi;
	xdpi = ydpi;
	ydpi = ftemp;
	ftemp = xdpi2;
	xdpi2 = ydpi2;
	ydpi2 = ftemp;
    }

    if ( ((xdpi + 0.5) > option.xdpi) && (xdpi - 0.5) < option.xdpi) {
	/* Width matches. Set size based on height. */
	if (fullscreen || (ydpi <= xdpi))
	    dpi = ydpi;
	else
	    dpi = ydpi2;
    }
    else if ( ((ydpi + 0.5) > option.ydpi) && (ydpi - 0.5) < option.ydpi) {
	/* Height matches. Set size based on width. */
	if (fullscreen || (xdpi <= ydpi))
	    dpi = xdpi;
	else
	    dpi = xdpi2;
    }
    else  {
	/* Neither width nor height match.  Fit the whole page. */
	if (xdpi > ydpi)
		dpi = ydpi;
	else
		dpi = xdpi;
    }
#ifdef DEBUG
    {
    char buf[MAXSTR];
    sprintf(buf, "\nwindow size=%d %d\n", 
    window_width, window_height);
    gs_addmess(buf);
    sprintf(buf, "size=%d %d\n", width, height);
    gs_addmess(buf);
    sprintf(buf, "old dpi=%f %f\n", option.xdpi, option.ydpi);
    gs_addmess(buf);
    sprintf(buf, "dpi=%f %f %f %f\n", xdpi, ydpi, xdpi2, ydpi2);
    gs_addmess(buf);
    sprintf(buf, "final dpi=%f\n", dpi);
    gs_addmess(buf);
    }
#endif
    option.xdpi = option.ydpi = dpi;
    gs_resize();
}

/* highlight words from first to last inclusive */
/* first may be > last */
/* word = -1 means nothing to mark */
void
highlight_words(int first, int last, BOOL marked)
{
    int left, right, top, bottom;
    float x, y;
    TEXTINDEX *text;
    int i;
    if ((first == -1) || (last == -1))
	return;

    if (pixmap == NULL)
	return;

    if ((first > (int)text_index_count) || (last > (int)text_index_count)) {
	gs_addmess("\nhighlight_words called with invalid arguments\n");
	return;
    }
    if (first > last) {
        int temp = first;
	first = last;
	last = temp;
    }

    GdkGC *gcinvert = gdk_gc_new(img->window);
    gdk_gc_set_function(gcinvert, GDK_INVERT);
    for (i = first; i<=last; i++) {
	text = &text_index[i];
	/* highlight found word */
	/* map bounding box to device coordinates */
	x = text->bbox.llx;
	y = text->bbox.lly;
	map_pt_to_pixel(&x, &y);
	left   = (int)x;
	bottom = (int)y;
	x = text->bbox.urx;
	y = text->bbox.ury;
	map_pt_to_pixel(&x, &y);
	right  = (int)x;
	top    = (int)y;
	if (top > bottom) {
	    int temp = top;
	    top = bottom;
	    bottom = temp;
	}
	if (left > right) {
	    int temp = right;
	    right = left;
	    left = temp;
	}

	if (pixmap) {
	    /* redraw rectangle we about to invert */
	    gdk_draw_pixmap(img->window,
		    img->style->fg_gc[GTK_WIDGET_STATE(img)],
		    pixmap,
		    left, top,
		    left, top,
		    right-left, bottom-top);
	}
	if (marked) {
	    /* invert text */
	    gdk_draw_rectangle(img->window, gcinvert,
		    TRUE, left, top, right-left, bottom-top);
	}
    }
    gdk_gc_unref(gcinvert);
}


void
highlight_links()
{
PDFLINK link;
int i = 0;
float x, y;
int x0, y0, x1, y1;
int w2;
GdkGC *gclink = gdk_gc_new(img->window);
/* colour code doesn't work - we need to allocate a colour map first */
GdkColor red = {0, 65535, 0, 0};
    
    while ( pdf_get_link(i, &link) ) {
	i++;
	if (link.border_width) {
	    /* map bounding box to device coordinates */
	    x = link.bbox.llx;
	    y = link.bbox.lly;
	    map_pt_to_pixel(&x, &y);
	    x0   = (int)x;
	    y1   = (int)y;
	    x = link.bbox.urx;
	    y = link.bbox.ury;
	    map_pt_to_pixel(&x, &y);
	    x1   = (int)x;
	    y0   = (int)y;
	    if (y0 > y1) {
		int temp = y0;
		y0 = y1;
		y1 = temp;
	    }
	    if (x0 > x1)  {
		int temp = x1;
		x1 = x0;
		x0 = temp;
	    }
	    /* draw border */
	    if (link.colour_valid) {
		GdkColor colour = {0, 0, 0, 0};
		colour.red = (int)(link.colour_red*65535+0.5);
		colour.green = (int)(link.colour_green*65535+0.5);
		colour.blue = (int)(link.colour_blue*65535+0.5);
		gdk_gc_set_foreground(gclink, &colour);
	    }
	    else {
		gdk_gc_set_foreground(gclink, &red);
	    }
	    gdk_draw_rectangle(img->window, gclink, FALSE,
		x0, y0, x1-x0, y1-y0);

	    w2 = (int)((link.border_width+0.5)/2);
	    gdk_gc_set_line_attributes(gclink, w2, GDK_LINE_SOLID,
		GDK_CAP_BUTT, GDK_JOIN_MITER);
/*
	    RoundRect(hdc, rect.left-w2, rect.top-w2, 
		    rect.right+w2, rect.bottom+w2, 
		    2 * ((int)(link.border_xr+0.5)),
		    2 * ((int)(link.border_yr+0.5)));
*/
	}
    }
    gdk_gc_unref(gclink);
}


void
info_link(void)
{
float x, y;
PDFLINK link;
    if (get_cursorpos(&x, &y)) {
	if (is_link(x, y, &link)) {
	    on_link = TRUE;
	    on_link_page = link.page;
	    statusbar_update();
	}
	else if (on_link)
	{
	    on_link = FALSE;
	    statusbar_update();
	}
    }
}

void copy_clipboard(void)
{
    gs_addmess("copy_clipboard: not implemented\n");
}

/* Save Pixmap to file */
/* doesn't work because of structure packing */
void
paste_to_file(void)
{
    LPBITMAP2 pbmih;
    BITMAPFILE bmfh;
    DWORD header_size;
    DWORD bitmap_size;
    FILE *f;
    PREBMAP pbmap;
    static char output[MAXSTR];
    pbmih = get_bitmap();
    if (pbmih == (LPBITMAP2)NULL)
	 return;
    scan_dib(&pbmap, (BYTE *)pbmih);
    header_size = pbmap.bits - (BYTE *)pbmih;
    bitmap_size = pbmap.height * pbmap.bytewidth;
    bmfh.bfType = ('M'<<8) | 'B';
    bmfh.bfReserved1 = 0;
    bmfh.bfReserved2 = 0;
    bmfh.bfOffBits = header_size + BITMAPFILE_LENGTH;
    bmfh.bfSize = bitmap_size + bmfh.bfOffBits;

    if ( get_filename(output, TRUE, FILTER_BMP, 0, IDS_TOPICCLIP)
	&& ((f = fopen(output, "w")) != NULL) ) {
	fputc('B', f);
	fputc('M', f);
	write_dword(bmfh.bfSize, f);
	write_word(bmfh.bfReserved1, f);
	write_word(bmfh.bfReserved2, f);
	write_dword(bmfh.bfOffBits, f);
	fwrite(pbmih, 1, header_size + bitmap_size, f);
	fclose(f);
    }
    release_bitmap();
}


void clip_convert(void)
{
    gs_addmess("clip_convert: not implemented\n");
}

void show_buttons(void)
{
    if (option.button_show)
	gtk_widget_show(GTK_WIDGET(buttonbar));
    else
	gtk_widget_hide(GTK_WIDGET(buttonbar));
}

void set_scroll(int hscroll, int vscroll)
{
    if (hscroll >= 0)
	gtk_adjustment_set_value(gtk_scrolled_window_get_hadjustment(
	GTK_SCROLLED_WINDOW(scroll_window)), hscroll);
    if (vscroll >= 0)
	gtk_adjustment_set_value(gtk_scrolled_window_get_vadjustment(
	GTK_SCROLLED_WINDOW(scroll_window)), vscroll);
}

/* if found word is not visible, scroll window to make it visible */
void scroll_to_find(void)
{
    float x, y;
    int left, right, top, bottom;
    int scrollx, scrolly;
    int window_width, window_height;

    BOOL changed = FALSE;

    /* first translate found box to window coordinates */
    x = psfile.text_bbox.llx;
    y = psfile.text_bbox.lly;
    map_pt_to_pixel(&x, &y);
    left   = (int)x;
    bottom = (int)y;
    x = psfile.text_bbox.urx;
    y = psfile.text_bbox.ury;
    map_pt_to_pixel(&x, &y);
    right  = (int)x;
    top    = (int)y;

    /* get current scroll position */
    scrollx = (int)(gtk_scrolled_window_get_hadjustment(
	GTK_SCROLLED_WINDOW(scroll_window))->value);
    scrolly = (int)(gtk_scrolled_window_get_vadjustment(
	GTK_SCROLLED_WINDOW(scroll_window))->value);

    /* get current window size */
    /* this is approximate, since the scroll window size less the
     * scroll bar is larger than the client area
     */
    window_width = scroll_window->allocation.width -
	GTK_SCROLLED_WINDOW(scroll_window)->vscrollbar->allocation.width;
    window_height = scroll_window->allocation.height -
	GTK_SCROLLED_WINDOW(scroll_window)->hscrollbar->allocation.height;

    if ((left < scrollx) || (right > scrollx + window_width))
	changed = TRUE;
    if ((top < scrolly) || (bottom > scrolly + window_height))
	changed = TRUE;

    if (changed)
	set_scroll((left + right - window_width) / 2,
	    (top + bottom - window_height) / 2);
}

int gsview_pstoedit(void)
{
    gs_addmess("gsview_pstoedit: not implemented\n");
    return FALSE;
}

void pre_parse_args(int argc, char *argv[])
{
    int i;
    char *str;
    for (i=1; i < argc; i++) {
	str = argv[i];
	if ( (*str == '-') && (str[1] != '0') ) {
	    /* a command line switch */
	    if ((str[1] == 'D') || (str[1] == 'd')) {
		str+=2;
		if (*str)
		    debug = atoi(str);
		else
		    debug = 1;
	    }
	}
    }
}

gint do_args_tag;

gint
do_args(gpointer data)
{
    if (do_args_tag)
	gtk_idle_remove(do_args_tag);
    do_args_tag = 0;

    if (gsview_changed()) {
	gtk_main_quit();
	return TRUE;
    }

    registration_check();

    if (gargc) {
	/* 
	 * we had some command line options that couldn't be
	 * be processed until the window was created.
	 */
	parse_args(gargc, gargv);
	gargc = 0;
	gargv = NULL;
    }

    selection_add();

    return TRUE;
}

void parse_args(int argc, char *argv[])
{
    int i;
    char *str;
    char filename[MAXSTR];
    char *p = filename;
    *p = '\0';
    for (i=1; i < argc; i++) {
	str = argv[i];
	if ( (*str == '-') && (str[1] != '0') ) {
	    /* a command line switch */
	    if ((str[1] == 'D') || (str[1] == 'd')) {
		/* already processed by pre_parse_args() */
	    }
	    else {
	        gserror(IDS_BADCLI, str, MB_ICONEXCLAMATION, SOUND_ERROR);
	    }
	}
	else {
	    /* a filename - we only allow one */
	    if (filename[0])
		continue;

	    if (*str == '\042')
		str++; 		/* don't copy quotes */
	    /* we need the filename to include the path */
	    if (str[0] && (str[0]=='/')) {
		/* contains full path */
		/* make this the work dir */
		char *t;
		strcpy(filename, str);
	        if ( (t = strrchr(filename, '/')) != (char *)NULL ) {
		    *(++t) = '\0';
		    gs_chdir(filename);
		}
	    }
	    else {
		/* Doesn't include full path so add work dir */
		char *t = workdir;
		while (*t)
		   *p++ = *t++;
		t--;
		if (*t != '/')
		   *p++ = '/';
	    }
	    while (*str) {
		if (*str == '\042')
		    str++; 	/* don't copy quotes */
		else
		    *p++ = *str++;
	    }
	    *p = '\0';
	}
    }
    if (filename[0]) {
	gsview_selectfile(filename);
	gsview_wcmd(NULL, (gpointer)IDM_REDISPLAY);
    }
}


int main( int argc, char *argv[] )
{
    pszLocale = gtk_set_locale();
    setlocale(LC_NUMERIC, "C");
    gtk_init (&argc, &argv);

    gs_getcwd(workdir, sizeof(workdir));
    pre_parse_args(argc, argv);
    if (argc > 1) {
	/* delay the argument processing until we reach idle state */
	gargc = argc;
	gargv = argv;
    }
    do_args_tag = gtk_idle_add(do_args, (gpointer)0);

    gs_addmessf("debug=%d\n", debug);

    if (gsview_init()) {
	fprintf(stdout, "Initialisation failed\n");
	return 1;
    }
    set_menu_sensitive();
    set_last_used();

    gtk_main ();

    measure_close();
    psfile_free(&psfile);
    pdf_free_link();
    if (option.settings && !print_silent)
	write_profile(); 
    else
	write_profile_last_files();	/* always save MRU files */
  
    return 0;
}

/* end of gvx.cpp */
