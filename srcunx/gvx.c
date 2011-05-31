/* Copyright (C) 2000-2005, Ghostgum Software Pty Ltd.  All rights reserved.
  
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
#include "gdk/gdkkeysyms.h"

GtkWidget *window;		/* main window */
GtkWidget *main_vbox;
GtkWidget *menubar;
GtkWidget *buttonbar;
GtkWidget *scroll_window;	/* this scrolls the image window */
GtkWidget *img;			/* drawing area, image window */
GtkWidget *statusbar;
GtkWidget *statusfile;
GtkWidget *statuscoord;
GtkWidget *statuspage;
char szLocale[64];
BOOL have_selection;

#ifdef MULTITHREAD
pthread_mutex_t hmutex_ps; 	/* for protecting psfile and pending */
#endif

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

const char *szSpoolPrefix = "%pipe%";	/* GS 6.0 and later should use %pipe% */
char coord_text[64];		/* last text displayed as coordinate */
BOOL multithread;
int geometry_width;
int geometry_height;
int geometry_xoffset;
int geometry_yoffset;

int on_link;			/* TRUE if we were or are over link */
int on_link_page;		/* page number of link target */
const char *on_link_action;	/* action of link target */
long gsbytes_size;		/* number of bytes for this page */
long gsbytes_done;		/* number of byte written */
BOOL quitnow = FALSE;		/* Used to cause exit from nested message loops */

int percent_done;		/* percentage of document processed */
int percent_pending;		/* TRUE if WM_GSPERCENT is pending */
BOOL fit_page_enabled = FALSE;	/* next WM_SIZE is allowed to resize window */

#define MAXARGS 64
int nargc;
char *nargv[MAXARGS];
GSVIEW_ARGS args;		/* Parsed arguments */
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
int disable_gsview_wcmd;	/* to avoid recursive messages */
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
USERMEDIA usermedia[IDM_USERSIZE13 - IDM_USERSIZE1 + 1];
FILE *pstotextOutfile;

typedef struct MESSAGE_s {
    int message;
    int param;
} MESSAGE;

#define MESSAGE_MAX 32
const int message_max = MESSAGE_MAX;
int message_count = 0;
MESSAGE messages[MESSAGE_MAX];
int xdisplay_local;

char workdir[MAXSTR];
BOOL in_img_window = FALSE;

void map_pt_to_pixel(float *x, float *y);
BOOL get_cursorpos(float *x, float *y);
void statuscoord_update(void);
BOOL draghand;
int draghand_x;
int draghand_y;
void highlight_links(void);
void highlight_words(int first, int last, BOOL marked);
BOOL text_marking = FALSE;
int text_mark_first = -1;
int text_mark_last = -1;
void info_link(void);
void selection_add(void);
void selection_release(void);
void *gs_thread(void *arg);
void do_img_message(int message, int param);
int find_img_message(int message);
int process_img_message(void);
int read_img_message(void);
void read_message_pipe_fn(gpointer data, gint fd, GdkInputCondition condition);
void set_menu_sensitive(void);
void selection_handle(GtkWidget *widget, GtkSelectionData *selection_data,
	guint info, guint time_stamp, gpointer data);
gint do_args(gpointer data);
void key_scroll_horz(int key);
void key_scroll_vert(int key);

/**********************************************************/

/* To communicate from the GS thread to the main GUI thread
 * we use a pipe.  This passes 8 byte messages containing
 * a message ID and an integer parameter.
 * GS thread sends a message with post_img_message().
 */

int message_pipe[2];	/* file descriptors */
gint message_pipe_tag;	/* for gtk_input_add */

/* stop listening for messages form GS thread */
void
close_img_message(void)
{
    if (debug & DEBUG_GENERAL)
	gs_addmess("close_img_message:\n");
    if (message_pipe_tag >= 0)
        gdk_input_remove(message_pipe_tag);
    message_pipe_tag = -1;
    if (message_pipe[0] >= 0)
	close(message_pipe[0]);
    message_pipe[0] = -1;
    if (message_pipe[1] >= 0)
	close(message_pipe[1]);
    message_pipe[1] = -1;
}

/* process a message from GS thread*/
void
do_img_message(int message, int param)
{
    if (message == WM_QUIT) {
	quit_gsview(window, NULL);
        gtk_main_quit();
    }
    else if (message == WM_CLOSE) {
	if (window->window) {
	    /* save window location */
	    /* This doesn't work very well, because different X servers
	     * and combinations of local/remote give inconsistent
	     * values for window location.
	     */
	    gint x, y;
	    gdk_window_get_root_origin(window->window, &x, &y);
	    option.img_origin.x = x;
	    option.img_origin.y = y;
	}
	quit_gsview(window, NULL);
    }
    else if (quitnow) {
	/* don't do draw on any windows */
    }
    else if (message == WM_COMMAND) {
	gsview_wcmd(NULL, (gpointer)((size_t)param));
    }
    else if (message == WM_GSDEVICE) {
	/* hide window if closed */
	if (!image.open) {
	    if ((GTK_WIDGET_FLAGS(img) & GTK_VISIBLE))
		gtk_widget_hide_all(img);
	}
    }
    else if (message == WM_GSSYNC) {
	int td;
	struct timeval tv1;
	struct timeval tv2;
	if (!(GTK_WIDGET_FLAGS(img) & GTK_VISIBLE))
	    gtk_widget_show_all(img);
	gettimeofday(&tv1, NULL);
	gtk_widget_draw(img, NULL);
	gettimeofday(&tv2, NULL);
	td = (tv2.tv_sec - tv1.tv_sec) * 1000 
	    + (tv2.tv_usec - tv1.tv_usec) / 1000;
	image_lock(&image);
	if (image.open && (td * 2 > image.tile_interval))
	    image.tile_interval += td + 100;
	image_unlock(&image);
    }
    else if (message == WM_GSPAGE) {
	int td;
	struct timeval tv1;
	struct timeval tv2;
	gsdll.state = GS_PAGE;
	if (display.show_find)
	    scroll_to_find();
	gettimeofday(&tv1, NULL);
	gtk_widget_draw(img, NULL);
	gettimeofday(&tv2, NULL);
	td = (tv2.tv_sec - tv1.tv_sec) * 1000 
	    + (tv2.tv_usec - tv1.tv_usec) / 1000;
	image_lock(&image);
	if (image.open && (td * 2 > image.tile_interval))
		image.tile_interval += td + 100;
	image_unlock(&image);
	info_wait(IDS_NOWAIT);
	selection_release();
    }
    else if (message == WM_GSSIZE) {
	if (image.open) {
	    gtk_drawing_area_size(GTK_DRAWING_AREA (img), 
		image.width, image.height);
	    if (!(GTK_WIDGET_FLAGS(img) & GTK_VISIBLE))
		gtk_widget_show(img);
	}
    }
    else if (message == WM_GSWAIT) {
	info_wait(param);
    }
    else if (message == WM_GSMESSBOX) {
	/* delayed message box, usually from other thread */
	char buf[MAXSTR];
	load_string(param, buf, sizeof(buf));
	message_box(buf, 0);
    }
    else if (message == WM_GSSHOWMESS) {
	gs_showmess();
    }
    else if (message == WM_GSREDISPLAY) {
	  gsview_wcmd(NULL, (gpointer)IDM_REDISPLAY);
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
	while (g_main_iteration(FALSE)); /* flush display */
    }
    else if (message == WM_GSTEXTINDEX) {
	gs_addmess("WM_GSTEXTINDEX not implemented\n");
	make_text_index();
	text_marking = FALSE;
	text_mark_first = text_mark_last = -1;
	selection_release();
    }
    else if (message == WM_GSTILE) {
	GdkRectangle area;
	image_lock(&image);
	if (image.open) {
	    int td;
	    struct timeval tv1;
	    struct timeval tv2;
	    int tx1 = (param >> 24) & 0xff;
	    int ty1 = (param >> 16) & 0xff;
	    int tx2 = (param >> 8) & 0xff;
	    int ty2 = (param) & 0xff;
	    area.x = (tx1 * image.tile_width) + display.offset.x;
	    area.y = (ty1 * image.tile_height) + display.offset.y;
	    area.width = (tx2 - tx1 + 1) * image.tile_width;
	    area.height = (ty2 - ty1 + 1) * image.tile_height;

	    if ((option.update == 2) || 
		((option.update == 1) && (xdisplay_local))) {
		image_unlock(&image);
		gettimeofday(&tv1, NULL);
		gtk_widget_draw(img, &area);
		gettimeofday(&tv2, NULL);
		image_lock(&image);
		td = (tv2.tv_sec - tv1.tv_sec) * 1000 
		    + (tv2.tv_usec - tv1.tv_usec) / 1000;
	    }
	    else {
		/* make sure we don't called too often */
		td = 2000;
	    }

	    if (image.open && (td * 5 > image.tile_interval))
		image.tile_interval += td + 100;
	}
	image_unlock(&image);
    }
    else
	gs_addmessf("Unknown post_img_message %d\n", message);
}

/* return index of the first message in the queue which contains the 
 * message id, or 0 if none in queue.
 */
int 
find_img_message(int message)
{
    int i;
    int idx = 0;
    for (i=1; i<message_count-1; i++) {
	if (messages[i].message == message) {
	    idx = i;
	    break;
	}
    }
    return idx;
}

/* process messages in the queue */
int
process_img_message(void)
{
    int message, param;
    int idx;
    BOOL ignore;
    if (message_count) {
	message = messages[0].message;
	param = messages[0].param;
	ignore = FALSE;
	/* Avoid doing potentially expensive drawing operations
	 * if there is another one in the queue.
	 */
	if (message == WM_GSSYNC) {
	    if (find_img_message(WM_GSSYNC) || find_img_message(WM_GSPAGE))
		ignore = TRUE;
	}
	else if (message == WM_GSPAGE) {
	    if (find_img_message(WM_GSPAGE))
		ignore = TRUE;
	}
	else if (message == WM_GSTILE) {
	    if (find_img_message(WM_GSSYNC) || find_img_message(WM_GSPAGE))
		ignore = TRUE;
	    else if ((idx = find_img_message(WM_GSTILE)) != 0) {
		/* merge tile updates */
		int nparam = messages[idx].param;
		int tx1 = (param >> 24) & 0xff;
		int ty1 = (param >> 16) & 0xff;
		int tx2 = (param >> 8) & 0xff;
		int ty2 = (param) & 0xff;
		int nx1 = (nparam >> 24) & 0xff;
		int ny1 = (nparam >> 16) & 0xff;
		int nx2 = (nparam >> 8) & 0xff;
		int ny2 = (nparam) & 0xff;
		nx1 = min(tx1, nx1);
		ny1 = min(ty1, ny1);
		nx2 = max(tx2, nx2);
		ny2 = max(ty2, ny2);
		nparam = (nx1<<24) + (ny1<<16) + (nx2<<8) + ny2;
		messages[idx].param = nparam;
		ignore = TRUE;
	    }
	}
	if (!ignore)
            do_img_message(message, param);
	/* BEWARE: the previous call can change message count if it lets
	 * the message loop run
	 */ 
	if (message_count > 0)
	    --message_count;
	else
	    message_count = 0;
	if (message_count > 0) {
	    memmove(messages, messages+1, message_count * sizeof(messages[0]));
	}
    }
    return 0;
}

/* read a message from the GS thread to us the GUI thread */
int
read_img_message(void)
{
    int bytes_read;
    int message, param;
    bytes_read = read(message_pipe[0], &message, sizeof(message));
    if (bytes_read > 0)
        bytes_read = read(message_pipe[0], &param, sizeof(param));
    if (bytes_read == -1) {
	if (errno == EAGAIN) {
	    return 1;	/* come back later */
	}
	else {
	    if (debug & DEBUG_GENERAL)
		gs_addmessf("read_img_message: read failed, errno=%d\n", errno);
	    return -1;
	}
    }
    if (message_count < message_max) {
	messages[message_count].message = message;
	messages[message_count].param = param;
	message_count++;
    }
    return 0;
}


/* Asynchronous read of message_pipe.
 * This is called from event loop when a read is possible on message_pipe.
 */
void read_message_pipe_fn(gpointer data, gint fd, GdkInputCondition condition)
{
    if (message_pipe[0] != fd) {
	if (debug & DEBUG_GENERAL)
	    gs_addmess("read_message_pipe_fn: called with wrong source\n");
	return;
    }

    if (condition & GDK_INPUT_EXCEPTION) {
	/* complain */
	if (debug & DEBUG_GENERAL)
	    gs_addmess("read_message_pipe_fn: exception\n");
	close_img_message();
    }
    else if (condition & GDK_INPUT_READ) {
	while (read_img_message()==0) {
	    if (message_count >= message_max)
		process_img_message();
	}
	while (message_count)
	    process_img_message();
    }
    else {
	if (debug & DEBUG_GENERAL)
	    gs_addmessf("read_message_pipe_fn: unknown condition %d\n", condition);
    }
}

/* start listening to messages from the GS thread */
int
init_img_message(void)
{
    int flags;
    if (pipe(message_pipe)) {
	gs_addmessf("Could not open pipe for messages, errno=%d\n", errno);
	return 1;
    }
    flags = fcntl(message_pipe[0], F_GETFL, 0);
    if (fcntl(message_pipe[0], F_SETFL, flags | O_NONBLOCK)) {
	gs_addmessf("Could not set message pipe to non-blocking, errno=%d\n", errno);
	close(message_pipe[0]);
	close(message_pipe[1]);
	return 1;
    }
    message_pipe_tag = gdk_input_add(message_pipe[0], 
	(GdkInputCondition)(GDK_INPUT_READ | GDK_INPUT_EXCEPTION),
	read_message_pipe_fn, 0);
    return 0;
}

/* Send a message from GS thread to the GUI thread.
 * Use a pipe to implement this.
 * We need this because the GS thread is not allowed
 * to use gtk, gdk or Xlib.
 */
void
post_img_message(int message, int param)
{
    int mess[2];
    mess[0] = message;
    mess[1] = param;
    write(message_pipe[1], &mess, sizeof(mess));
    /* no flush needed, because low level I/O doesn't buffer */
}


/* On "File | Exit" or window destroy */ 
void quit_gsview( GtkWidget *w,
                         gpointer   data )
{
    /* Tell GS thread to exit. */
    /* When it finishes, it will post the quit message */
    quitnow = TRUE;
    pending.unload = TRUE;
#ifdef MULTITHREAD
    pending.abort = TRUE;
    if (multithread)
	sem_post(&display.event);	/* unblock display thread */
#endif
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
	    if (strlen(buf)>64) {
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
    enable_menu_item(IDM_OPTIONMENU, IDM_CFG, FALSE);
    enable_menu_item(IDM_VIEWMENU, IDM_FULLSCREEN, FALSE);
    enable_menu_item(IDM_HELPMENU, IDM_HELPSEARCH, FALSE);
}

/* callback from menu to gsview_command */
void gsview_wcmd(GtkWidget *w, gpointer data)
{
    int command = (int)((size_t)data);
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

    if (debug & DEBUG_GENERAL)
	gs_addmessf("gsview_wcmd: gsdll.state=%d\n", gsdll.state);

    gsview_command(command);
    if (debug & DEBUG_GENERAL)
	gs_addmessf("gsview_wcmd: now=%d next=%d unload=%d\n", 
	    pending.now, pending.next, pending.unload);

    if (pending.now || pending.next || pending.unload || quitnow) {
	if (display.bitcount == 0) {
	    gint x, y, width, height, depth;
	    gdk_window_get_geometry(window->window, &x, &y, 
		&width, &height, &depth);
	    display.planes = 1;
	    display.bitcount = depth;
	}
    
#ifdef MULTITHREAD
	if (multithread) {
	    /* release other thread if needed */
	    sem_post(&display.event);
	}
#endif
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
	if (draghand) {
	    draghand = FALSE;
	}
    }
    return TRUE;
}

void
key_scroll_horz(int key)
{
    int delta = 0;
    GtkAdjustment *hadjust = gtk_scrolled_window_get_hadjustment(
	    GTK_SCROLLED_WINDOW(scroll_window));
    switch(key) {
	case GDK_Home:
	case GDK_KP_Home:
	    delta = -hadjust->value;
	    break;
	case GDK_End:
	case GDK_KP_End:
	    delta = (hadjust->upper - hadjust->page_size) - hadjust->value;
	    break;
	case GDK_Left:
	case GDK_KP_Left:
	    delta = -hadjust->step_increment;
	    break;
	case GDK_Right:
	case GDK_KP_Right:
	    delta = hadjust->step_increment;
	    break;
	case GDK_Page_Up:
	case GDK_KP_Page_Up:
	    delta = min(-1,-hadjust->page_increment);
	    break;
	case GDK_Page_Down:
	case GDK_KP_Page_Down:
	    delta = max(1,+hadjust->page_increment);
	    break;
	default:
	    delta = 0;
    }

    if (hadjust->value + delta < 0)
	delta = -hadjust->value;
    if (hadjust->value + delta >= hadjust->upper - hadjust->page_size)
	delta = hadjust->upper - hadjust->page_size - hadjust->value;

    set_scroll(hadjust->value + delta, -1);
}

void
key_scroll_vert(int key)
{
    int delta = 0;
    GtkAdjustment *vadjust = gtk_scrolled_window_get_vadjustment(
	    GTK_SCROLLED_WINDOW(scroll_window));
    switch(key) {
	case GDK_Home:
	case GDK_KP_Home:
	    delta = -vadjust->value;
	    break;
	case GDK_End:
	case GDK_KP_End:
	    delta = (vadjust->upper - vadjust->page_size) - vadjust->value;
	    break;
	case GDK_Up:
	case GDK_KP_Up:
	    delta = -vadjust->step_increment;
	    break;
	case GDK_Down:
	case GDK_KP_Down:
	    delta = vadjust->step_increment;
	    break;
	case GDK_Page_Up:
	case GDK_KP_Page_Up:
	    delta = min(-1,-vadjust->page_increment);
	    break;
	case GDK_Page_Down:
	case GDK_KP_Page_Down:
	    delta = max(1,+vadjust->page_increment);
	    break;
	default:
	    delta = 0;
    }

    if (vadjust->value + delta < 0)
	delta = -vadjust->value;
    if (vadjust->value + delta >= vadjust->upper - vadjust->page_size)
	delta = vadjust->upper - vadjust->page_size - vadjust->value;

    if ((gsdll.state != GS_IDLE) && 
	((key == GDK_Page_Down) || (key == GDK_KP_Page_Down) ||
	 (key == GDK_Page_Up) || (key == GDK_KP_Page_Up)) &&
	(delta == 0)) {
	/* Advance to next or previous page */
	int numpages = 0;
	request_mutex();
	if (psfile.dsc != (CDSC *)NULL)
	    numpages = psfile.dsc->page_count;
	release_mutex();
	switch(key) {
	    case GDK_Page_Up:
	    case GDK_KP_Page_Up:
	      if ((psfile.dsc != (CDSC *)NULL)
		    && (psfile.pagenum != 1)) {
		set_scroll(-1, vadjust->upper - vadjust->page_size);
		gsview_wcmd(NULL, (gpointer)IDM_PREV);
	      }
	      break;
	    case GDK_Page_Down:
	    case GDK_KP_Page_Down:
	      if ((psfile.dsc == (CDSC *)NULL)
		    || (psfile.pagenum < numpages)) {
		set_scroll(-1, 0);
		gsview_wcmd(NULL, (gpointer)IDM_NEXT);
	      }
	      break;
	}
    }
    else 
        set_scroll(-1, vadjust->value + delta);
}


gint 
key_press_event(GtkWidget *widget, GdkEventKey *event)
{

    if (event->state & GDK_MOD1_MASK) { 	/* Alt key down */
        switch (event->keyval) {
	    case GDK_comma:
	        gsview_wcmd(NULL, (gpointer)IDM_PREV);
		break;
	    case GDK_period:
	        gsview_wcmd(NULL, (gpointer)IDM_NEXT);
		break;
	}
	return TRUE;
    }

    switch (event->keyval) {
	case GDK_Alt_L:
	case GDK_Alt_R:
	    break;
	case GDK_KP_Up:
	case GDK_Up:
	case GDK_KP_Down:
	case GDK_Down:
	    key_scroll_vert(event->keyval);
	    break;
	case GDK_Page_Up:
	case GDK_KP_Page_Up:
	case GDK_Page_Down:
	case GDK_KP_Page_Down:
	case GDK_Home: case GDK_KP_Home:
	case GDK_End:
	case GDK_KP_End:
	    if (event->state & GDK_CONTROL_MASK)
		key_scroll_horz(event->keyval);
	    else
		key_scroll_vert(event->keyval);
	    break;
	case GDK_KP_Left:
	case GDK_KP_Right:
	case GDK_Left:
	case GDK_Right:
	    key_scroll_horz(event->keyval);
	    break;
	case GDK_BackSpace:
	    gsview_wcmd(NULL, (gpointer)IDM_PREVHOME);
	    break;
	case GDK_plus:
	case GDK_KP_Add:
	    gsview_wcmd(NULL, (gpointer)IDM_NEXT);
	    break;
	case GDK_minus:
	case GDK_KP_Subtract:
	    gsview_wcmd(NULL, (gpointer)IDM_PREV);
	    break;
	case GDK_less:
	case GDK_comma:
	    gsview_wcmd(NULL, (gpointer)IDM_MAGMINUS);
	    break;
	case GDK_greater:
	case GDK_period:
	    gsview_wcmd(NULL, (gpointer)IDM_MAGPLUS);
	    break;
	case GDK_F1:
	    gsview_wcmd(NULL, (gpointer)IDM_HELPCONTENT);
	    break;
	  /* ignore it 
	default:
	      g_print("Key press 0x%x 0x%x\n", event->keyval, event->state);
	   */
    }
    return TRUE;
}

gint
button_press_event(GtkWidget *widget, GdkEventButton *event)
{
    /* On some platforms, pressing the button causes us to be notified
     * that we have left the window, which prevents get_cursorpos()
     * from working.  Assume that if we got the button click,
     * we must have been in the window anyway.
     */
    in_img_window = TRUE;
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
		    if (link.page == 0) {
			if (strcmp(link.action, "GoBack")==0)
			    history_back();
			else
			    gserror(IDS_NOLINKTARGET, NULL, 0, 
				SOUND_ERROR);
		    }
		    else {
			gsview_unzoom();
			pending.pagenum = link.page;
			history_add(pending.pagenum);
			pending.now = TRUE;
			gsview_wcmd(NULL, 0);
		    }
		}
	        else {
		    draghand = TRUE;
		    draghand_x = (int)event->x;
		    draghand_y = (int)event->y;
		}
		measure_setpoint(x, y);
	    }
	    else
	    {
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
	    /* This is the coordinate in pts that will become  */
	    /* the centre of the window. */
	    display.zoom_xoffset = (int)x;
	    display.zoom_yoffset = (int)y;
	    /* Get scroll bar position. */
	    /* x=0 is left, y=0 is top */
	    scrollx = (int)(gtk_scrolled_window_get_hadjustment(
		GTK_SCROLLED_WINDOW(scroll_window))->value);
	    scrolly = (int)(gtk_scrolled_window_get_vadjustment(
		GTK_SCROLLED_WINDOW(scroll_window))->value);
	    /* Get viewable area */
	    /* This is wrong because it ignores the size of the scrollbars */
	    zwidth = scroll_window->allocation.width;
	    zheight = scroll_window->allocation.height;
	    if (zwidth > image.width)
		zwidth = image.width;
	    if (zheight > image.height)
		zheight = image.height;

	    /* get coordinates of centre of viewable area */
	    /* relative to bottom left of page */
	    x = (scrollx + zwidth/2)*72.0/option.xdpi;
	    y = (image.height-(scrolly + zheight/2))*72.0/option.ydpi;
	    /* cope with rotated pages */
	    transform_point(&x, &y);
	    /* adjust zoom offset to put cursor position at centre */
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

gint
motion_notify_event(GtkWidget *widget, GdkEventMotion *event)
{
    float x, y;
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
	if (draghand) {
	    GtkAdjustment *hadjust = gtk_scrolled_window_get_hadjustment(
		GTK_SCROLLED_WINDOW(scroll_window));
	    GtkAdjustment *vadjust = gtk_scrolled_window_get_vadjustment(
		GTK_SCROLLED_WINDOW(scroll_window));
	    int dx = (int)event->x - draghand_x;
	    int dy = (int)event->y - draghand_y;
	    set_scroll(hadjust->value - dx, vadjust->value - dy);
	}
    }

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
    gint x, y;
    /* remember image size and location */
    if (window->window) {
	gdk_window_get_position(window->window, &x, &y);
	option.img_origin.x = x;
	option.img_origin.y = y;
    }
    option.img_size.x = window->allocation.width;
    option.img_size.y = window->allocation.height;
    return TRUE; 
}

static void
window_draw(GtkWidget *widget, int x, int y, int width, int height)
{
    if (image.open && image.image) {
        int color = image.format & DISPLAY_COLORS_MASK;
	int depth = image.format & DISPLAY_DEPTH_MASK;
	switch (color) {
	    case DISPLAY_COLORS_NATIVE:
		if (depth == DISPLAY_DEPTH_8) {
		    gdk_draw_indexed_image(widget->window, 
			widget->style->fg_gc[GTK_STATE_NORMAL],
			x, y, width, height, GDK_RGB_DITHER_MAX, 
			image.image + x + y*image.raster, 
			image.raster, image.cmap);
		}
	  	else if ((depth == DISPLAY_DEPTH_16) && image.rgbbuf) {
		    gdk_draw_rgb_image(widget->window, 
			widget->style->fg_gc[GTK_STATE_NORMAL],
			x, y, width, height, GDK_RGB_DITHER_MAX, 
			image.rgbbuf + x*3 + image.width*3*y, 
			image.width * 3);
		}
		break;
	    case DISPLAY_COLORS_GRAY:
		if (depth == DISPLAY_DEPTH_8)
		    gdk_draw_gray_image(widget->window, 
			widget->style->fg_gc[GTK_STATE_NORMAL],
			x, y, width, height, GDK_RGB_DITHER_MAX, 
			image.image + x + y*image.raster, 
		        image.raster);
		else if (depth == DISPLAY_DEPTH_1) {
		    /* FIX */
		    gs_addmess("1 bit/pixel drawing not implemented\n");
		}
		break;
	    case DISPLAY_COLORS_RGB:
		if (depth == DISPLAY_DEPTH_8) {
		    if (image.rgbbuf) {
			gdk_draw_rgb_image(widget->window, 
			    widget->style->fg_gc[GTK_STATE_NORMAL],
			    x, y, width, height, GDK_RGB_DITHER_MAX, 
			    image.rgbbuf + x*3 + image.width*3*y, 
			    image.width * 3);
		    }
		    else {
			gdk_draw_rgb_image(widget->window, 
			    widget->style->fg_gc[GTK_STATE_NORMAL],
			    x, y, width, height, GDK_RGB_DITHER_MAX, 
			    image.image + x*3 + y*image.raster, 
			    image.raster);
		    }
		}
		break;
	    case DISPLAY_COLORS_CMYK:
		if ((depth == DISPLAY_DEPTH_8) && image.rgbbuf)
		    gdk_draw_rgb_image(widget->window, 
			widget->style->fg_gc[GTK_STATE_NORMAL],
			x, y, width, height, GDK_RGB_DITHER_MAX, 
			image.rgbbuf + x*3 + image.width*3*y, 
			image.width * 3);
		break;
	}
    }
}


gint
expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
    GdkGC *gcdash;
    GdkGC *gcinvert;
    GdkColor black = {0, 0, 0, 0};
    image_lock(view.img);
    if (image.open && image.image) {
	int x = event->area.x;
	int y = event->area.y;
	int width = event->area.width;
	int height = event->area.height;
	if ((x>=0) && (y>=0) && (x <= image.width) && (y <= image.height)) {
	    /* drawing area intersects the bitmap, so draw it */
	    if (x + width > image.width)
		width = image.width - x;
	    if (y + height > image.height)
		height = image.height - y;
	    window_draw(img, x, y, width, height);
	}
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
	gcdash = gdk_gc_new(img->window);
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
    if (image.open && display.show_find) {
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

	if (image.open) {
	    /* redraw rectangle we about to invert */
            window_draw(img, left, top, right-left, bottom-top);
	}
	/* invert text */
	gcinvert = gdk_gc_new(img->window);
	gdk_gc_set_function(gcinvert, GDK_INVERT);
	gdk_draw_rectangle(img->window, gcinvert,
		TRUE, left, top, right-left, bottom-top);
	gdk_gc_unref(gcinvert);
    }

    /* highlight marked words */
    highlight_words(text_mark_first, text_mark_last, TRUE);

    /* GS 6.50 highlights links itself for PDF files */
    if ((option.gsversion < 650) || !psfile.ispdf)
	highlight_links();

    image_unlock(view.img);

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
	*y = -(*y * option.ydpi / 72.0) + (image.height - 1) 
		+ display.offset.y;
    }
    else {
	int xoffset = display.xoffset / display.xdpi * 72.0 + 0.5;
	int yoffset = display.yoffset / display.ydpi * 72.0 + 0.5;
	*x = *x - xoffset;
	*y = *y - yoffset;
	itransform_point(x, y);
	*x = *x * option.xdpi/72.0 + display.offset.x;
	*y = -(*y * option.ydpi/72.0)
	      + (image.height - 1) + display.offset.y;
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
    *y = image.height - 1 - iy;
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
	while (g_main_iteration(FALSE)); /* flush display */
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
		if ((on_link_page == 0) && on_link_action[0]) {
		    strncpy(buf, on_link_action, sizeof(buf)-1);
		}
		else
		{
		    load_string(IDS_LINKPAGE, fmt, sizeof(fmt));
		    sprintf(buf, fmt, on_link_page);
		}
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
	    if ((gsdll.state == GS_IDLE) || (gsdll.state == GS_UNINIT))
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

    while (g_main_iteration(FALSE)); /* flush display */
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
        int page = psfile.pagenum;
	if ((page > 0) && (page <= (int)psfile.dsc->page_count))  {
	    if (psfile.dsc->page[page-1].media)
		mediabox = psfile.dsc->page[page-1].media->mediabox;
	    cropbox = psfile.dsc->page[page-1].bbox;
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
    /* -4 is to allow for border around window */
    xdpi = (window_width - 4) * 72.0 / width;
    ydpi = (window_height - 4) * 72.0 / height;
    if (fullscreen) {
	xdpi2 = xdpi;
	ydpi2 = ydpi;
    }
    else {
	/* These are the resolutions allowing for a scroll bar */
/* These do not allow for the extra border provided by the scrolled_window */
/* We don't know how to obtain this extra width */
/*
	xdpi2 = (window_width - 
	    GTK_SCROLLED_WINDOW(scroll_window)->vscrollbar->allocation.width)
	    * 72.0 / width;
	ydpi2 = (window_height - 
	    GTK_SCROLLED_WINDOW(scroll_window)->hscrollbar->allocation.height)
	    * 72.0 / height;
*/
	ydpi2 = (window_height) * 72.0 / height;

	/* Assume scroll bar is 22 pixels wide */
	xdpi2 = (window_width - 22) * 72.0 / width;
	ydpi2 = (window_height - 22) * 72.0 / height;

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
    GdkGC *gcinvert;
    int i;
    if ((first == -1) || (last == -1))
	return;

    if (!image.open)
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

    gcinvert = gdk_gc_new(img->window);
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

	/* redraw rectangle we about to invert */
	window_draw(img, left, top, right-left, bottom-top);

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
	    on_link_action = link.action;
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

/* Save image to file */
/* doesn't work because of structure packing */
void
paste_to_file(void)
{
/* not implemented fully */
    LPBITMAP2 pbmih;
    BITMAPFILE bmfh;
    DWORD header_size;
    DWORD bitmap_size;
    FILE *f;
    PREBMAP pbmap;
    static char output[MAXSTR];

    image_lock(view.img);
    pbmih = get_bitmap();
    image_unlock(view.img);
    if (pbmih == (LPBITMAP2)NULL)
	 return;

    scan_dib(&pbmap, (BYTE *)pbmih);
    header_size = BITMAP2_LENGTH;
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
    GtkAdjustment *hadjust = gtk_scrolled_window_get_hadjustment(
	GTK_SCROLLED_WINDOW(scroll_window));
    GtkAdjustment *vadjust = gtk_scrolled_window_get_vadjustment(
	GTK_SCROLLED_WINDOW(scroll_window));
    hscroll = min(hscroll, (int)(hadjust->upper - hadjust->page_size));
    vscroll = min(vscroll, (int)(vadjust->upper - vadjust->page_size));
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


gint do_args_tag;

gint
do_args(gpointer data)
{
    GSVIEW_ARGS *pargs = &args;
    if (do_args_tag)
	gtk_idle_remove(do_args_tag);
    else
	return TRUE;	/* Ignore multiple calls */
    do_args_tag = 0;

    if (gsview_changed()) {
	gtk_main_quit();
	return TRUE;
    }

    registration_check();

    /* 
     * we had some command line options that couldn't be
     * be processed until the window was created.
     */

    if (pargs->print || pargs->convert) {
	make_cwd(pargs->filename);
	gsview_selectfile(pargs->filename);
	if (pargs->device[0]) {
	    if (pargs->print)
		strcpy(option.printer_device, pargs->device);
	    else
		strcpy(option.convert_device, pargs->device);
	}
	if ((pargs->queue[0]) && pargs->print)
	    strcpy(option.printer_queue, pargs->queue);
	if (dfreopen() != 0) {
	    release_mutex();
	    return 0;
	}
	if (psfile.name[0] != '\0') {
	    option.print_to_file = FALSE;
	    gsview_print(!pargs->print);
	}
	dfclose();
    }
    else if (pargs->spool) {
	gs_addmess("-s unsupported\n");
    }
    else  if (pargs->filename[0]) {
	make_cwd(pargs->filename);
	gsview_displayfile(pargs->filename);
	gsview_wcmd(NULL, (gpointer)0); 	/* give it a kick */
    }

    selection_add();

    return TRUE;
}


void *
gs_thread(void *arg)
{
    while (!quitnow) {
	if (!pending.now)
	    wait_event();
	if (!quitnow)
	    gs_process();
    }

    /* signal that we have finished */
    post_img_message(WM_QUIT, 0);   /* shut down application */
    return NULL;
}

int main( int argc, char *argv[] )
{
    int rc;
    /* This is gtk+-1.2 which doesn't support UTF-8 cleanly,
     * so remove it if set in LANG environment variable.
     * Our menus and strings are in a local codepage, not UTF-8.
     */
    char lang[MAXSTR];
    char *p;
    const char *pszLocale;
    gchar *xdisplay;
    memset(lang, 0, sizeof(lang));
    strncpy(lang, "LANG=", 5);
    p = getenv("LANG");
    if (p) {
	strncpy(lang+5, p, sizeof(lang)-6);
	p = strchr(lang, '.');
    }
    if (p && ((strcmp(p, ".UTF-8") == 0) || (strcmp(p, ".utf8") == 0))) {
	*p = '\0';	/* remove UTF-8 */
 	putenv(lang);
    }
    pszLocale = gtk_set_locale();
    memset(szLocale, 0, sizeof(szLocale));
    strncpy(szLocale, pszLocale, sizeof(szLocale)-1);
    setlocale(LC_NUMERIC, "C");
    gtk_init (&argc, &argv);
    gdk_rgb_init();
    gtk_widget_set_default_colormap(gdk_rgb_get_cmap());
    gtk_widget_set_default_visual(gdk_rgb_get_visual());
    xdisplay = gdk_get_display();
    xdisplay_local = (xdisplay[0] == ':');

    gs_getcwd(workdir, sizeof(workdir));

    rc = gsview_init(argc, argv);

    gs_addmessf("debug=%d\n", debug);
    if (rc < 0) {
	fprintf(stdout, "Initialisation failed\n");
	return 1;
    }
    else if (rc > 0) {
	return 0;	/* clean exit */
    }
    do_args_tag = gtk_idle_add(do_args, (gpointer)0);
    set_menu_sensitive();
    set_last_used();

#ifdef MULTITHREAD
    if (multithread) {
	/* start thread for displaying */
	if (pthread_create(&display.tid, NULL, gs_thread, NULL)) {
	    fprintf(stdout, "pthread_create failed\n");
	    return 1;
	}
    }
#endif

    /* test code for single threading */
    if (multithread) {
        gtk_main();
    }
    else {
        while (!quitnow) {
	    /* wait for event */
	    gtk_main_iteration();
	    if (pending.now) {
		gs_process();	/* start Ghostscript */
	    }
	}
    }

    close_img_message();
#ifdef MULTITHREAD
    if (multithread) {
	sem_destroy(&display.event);
	pthread_mutex_destroy(&image.hmutex);
	pthread_mutex_destroy(&hmutex_ps);
    }
#endif

    unload_pstoedit();
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
