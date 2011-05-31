/* Copyright (C) 2001, Ghostgum Software Pty Ltd.  All rights reserved.
  
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

/* gvxinit.cpp */

#include "gvx.h"
#include "gvxlang.h"
#include <dlfcn.h>

GtkWidget *find_menu_widget(int id);
void edit_menu_show(GtkWidget *w, gpointer   data);
void remove_main_menu(GtkWidget *window);
void add_main_menu(GtkWidget *window);
void button_enter(GtkButton *button, gpointer user_data);
void button_leave(GtkButton *button, gpointer user_data);
void button_settips(void);
GtkWidget *bitmap_button(GtkWidget *bar, const char **xpm, int id);
void make_buttonbar(void);
gint button_bar_configure_event(GtkWidget *widget, GdkEventExpose *event);
int parse_args(GSVIEW_ARGS *args);
void language_select(GtkWidget *w, gpointer data);
int get_language(void);
int check_locale(const char *lang);
void set_usermedia(void);


GtkItemFactory *item_factory;
GtkAccelGroup *accel_group;
GtkWidget *edit_menu;
guint edit_menu_tag;

/* We load each of these DLLs and if they contain a version
 * string that matches the GSview EXE, we consider them to
 * be a valid language DLL.  We store the details about each
 * in the following structure.  
 * The id is allocated automatically.
 * We refer to each language in the INI file using the twocc code, 
 * which matches the two letter Internet country code.
 * Each language DLL provides a string with the name of the language
 * in it's own language and also in English.  The latter is used 
 * on systems without that language installed.
 * The required ANSI codepage is in codepage.  If any code page is
 * acceptable (e.g. English) then this may be 0.
 */
typedef struct lang_s {
    int id;
    char twocc[3];
/*    char dllname[MAXSTR]; */
    char name[MAXSTR];	/* language name in it's language and codepage */
    char ename[MAXSTR];	/* language name in English */
/*    int codepage; */
} lang_t;

/* The list of language DLLs available */
const int nlang = 8;
lang_t lang[8] = {
{IDM_LANGEN, "en", "English", "English"},
{IDM_LANGDE, "de", "Deutsch", "German"},
{IDM_LANGFR, "fr", "Français", "French"},
{IDM_LANGGR, "gr", "Ellenika", "Greek"},
{IDM_LANGIT, "it", "Italian", "Italian"},
{IDM_LANGES, "es", "Español", "Spanish"},
{IDM_LANGNL, "nl", "Nederlands", "Dutch"},
{IDM_LANGSE, "se", "Svenska", "Swedish"}
};

int language_id(const char *str)
{
    int i;
    for (i=0; i<nlang; i++) {
	if (strcmp(str, lang[i].twocc) == 0)
	    return lang[i].id;
    }
    return 0;
}

const char * language_twocc(int id)
{
    int i;
    for (i=0; i<nlang; i++) {
	if (id == lang[i].id)
	    return lang[i].twocc;
    }
    return "en";
}


/* get text of menu item */
/* returns count of characters copied to str */
int
get_menu_string(int menuid, int itemid, char *str, int len)
{
    int i, j;
    char *s;
    MENU_ENTRY *menu_item;
    int menu_len;
    if (len==0)
	return 0;
    if (menubar == NULL)
	return 0;
    str[0] = '\0';

    /* Usermedia menus need special code because they change */
    for (j=0; j<sizeof(usermedia)/sizeof(USERMEDIA); j++) {
	if (itemid == usermedia[j].id) {
	    strncpy(str, usermedia[j].name, 
		min(len, sizeof(usermedia[j].name)));
	    return strlen(str);
	}
    }

    switch (option.language) {
	case IDM_LANGDE:
	    menu_item = menu_de;
	    menu_len = menu_de_len;
	    break;
	case IDM_LANGFR:
	    menu_item = menu_fr;
	    menu_len = menu_fr_len;
	    break;
	case IDM_LANGGR:
	    menu_item = menu_gr;
	    menu_len = menu_gr_len;
	    break;
	case IDM_LANGES:
	    menu_item = menu_es;
	    menu_len = menu_es_len;
	    break;
	case IDM_LANGIT:
	    menu_item = menu_it;
	    menu_len = menu_it_len;
	    break;
	case IDM_LANGNL:
	    menu_item = menu_nl;
	    menu_len = menu_nl_len;
	    break;
	case IDM_LANGSE:
	    menu_item = menu_se;
	    menu_len = menu_se_len;
	case IDM_LANGEN:
	default:
	    menu_item = menu_en;
	    menu_len = menu_en_len;
    }
    /* search through menu_item until we find the id */
    for (i=0; i<menu_len; i++) {
	if (menu_item[i].callback_action == (unsigned int)itemid)
	    break;
    }
    if (i == menu_len)
	return 0;

    /* find last slash */
    s = strrchr(menu_item[i].path, '/');
    if (s == NULL)
	return 0;

    /* copy, omitting underscores */
    s++;
    for (j=0; (j<len-1) && *s; s++) {
	if (*s != '_')
	    str[j++] = *s;
    }
    str[j] = '\0';

    return strlen(str);
}


GtkWidget *find_menu_widget(int id)
{
    int i;
    char buf[MAXSTR];
    const char *s;
    char *d;
    MENU_ENTRY *menu_item;
    int menu_len;
    if (menubar == NULL)
	return NULL;
    /* search through menu_items until we find the id */
    switch (option.language) {
	case IDM_LANGDE:
	    menu_item = menu_de;
	    menu_len = menu_de_len;
	    break;
	case IDM_LANGFR:
	    menu_item = menu_fr;
	    menu_len = menu_fr_len;
	    break;
	case IDM_LANGGR:
	    menu_item = menu_gr;
	    menu_len = menu_gr_len;
	    break;
	case IDM_LANGES:
	    menu_item = menu_es;
	    menu_len = menu_es_len;
	    break;
	case IDM_LANGIT:
	    menu_item = menu_it;
	    menu_len = menu_it_len;
	    break;
	case IDM_LANGNL:
	    menu_item = menu_nl;
	    menu_len = menu_nl_len;
	    break;
	case IDM_LANGSE:
	    menu_item = menu_se;
	    menu_len = menu_se_len;
	    break;
	case IDM_LANGEN:
	default:
	    menu_item = menu_en;
	    menu_len = menu_en_len;
    }
    for (i=0; i<menu_len; i++) {
	if (menu_item[i].callback_action == (unsigned int)id)
	    break;
    }
    if (i == menu_len)
	return NULL;

    /* copy the menu path, removing underscores */
    d = buf;
    for (s=menu_item[i].path; *s; s++)
	if (*s != '_')
	    *d++ = *s;
    *d = '\0';
    
    /* find it */
    return gtk_item_factory_get_widget(item_factory, buf); 
}

void
enable_menu_item(int menuid, int itemid, BOOL enabled)
{
    GtkWidget *w;
    if (menubar == NULL)
	return;
    if ((w = find_menu_widget(itemid)) != NULL)
	gtk_widget_set_sensitive(w, enabled);
}

extern int disable_gsview_wcmd;

/* change menu item checkmark */
void
check_menu_item(int menuid, int itemid, BOOL checked)
{
    GtkWidget *w;
    if (menubar == NULL)
	return;
    if ((w = find_menu_widget(itemid)) != NULL) {
	/* changing the menu check state causes check_menu_item
	 * to be called recursively.  Disable signal handler
	 * using disable_gsview_wcmd to avoid this.
	 * We can't use gtk_signal_handler_block_by_data()
	 * because the signal handler is being called with
	 * object = NULL.
	 */
        disable_gsview_wcmd = 1;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), checked);
        disable_gsview_wcmd = 0;
    }
    else
	gs_addmessf("check_menu_item: failed to find menu widget %d\n", itemid);
}

/* enable/disable menu items just before they are displayed */ 
void edit_menu_show(GtkWidget *w, gpointer   data)
{
    BOOL addeps;
    BOOL extracteps;
    BOOL idle = (gsdll.state != GS_BUSY);
/*
    enable_menu_item(IDM_EDITMENU, IDM_COPYCLIP, image.open);
    enable_menu_item(IDM_EDITMENU, IDM_CONVERT, FALSE);
*/
    enable_menu_item(IDM_EDITMENU, IDM_PASTETO, image.open);
    addeps =  (psfile.dsc != (CDSC *)NULL) && psfile.dsc->epsf && idle;
    enable_menu_item(IDM_EDITMENU, IDM_ADDEPSMENU, addeps);
    enable_menu_item(IDM_ADDEPSMENU, IDM_MAKEEPSU, addeps);
    addeps =  addeps && image.open;
    enable_menu_item(IDM_ADDEPSMENU, IDM_MAKEEPSI, addeps);
    enable_menu_item(IDM_ADDEPSMENU, IDM_MAKEEPST4, addeps);
    enable_menu_item(IDM_ADDEPSMENU, IDM_MAKEEPST6U, addeps);
    enable_menu_item(IDM_ADDEPSMENU, IDM_MAKEEPST6P, addeps);
    enable_menu_item(IDM_ADDEPSMENU, IDM_MAKEEPSW, addeps);

    extracteps = ((psfile.preview == IDS_EPST) || (psfile.preview == IDS_EPSW))
	 && idle;
    enable_menu_item(IDM_EDITMENU, IDM_EXTRACTPS, extracteps);
    enable_menu_item(IDM_EDITMENU, IDM_EXTRACTPRE, extracteps);

    enable_menu_item(IDM_EDITMENU, IDM_TEXTEXTRACT, idle);
    enable_menu_item(IDM_EDITMENU, IDM_TEXTFIND, idle);
    enable_menu_item(IDM_EDITMENU, IDM_TEXTFINDNEXT, idle);
}

void remove_main_menu(GtkWidget *window)
{
    if (menubar == NULL)
	return;
    if (edit_menu)
        gtk_signal_disconnect(GTK_OBJECT(edit_menu), edit_menu_tag);
    edit_menu_tag = 0;
    edit_menu = NULL;
    last_file_widget[0] = NULL;
    last_file_widget[1] = NULL;
    last_file_widget[2] = NULL;
    last_file_widget[3] = NULL;
/*    gtk_container_remove(GTK_CONTAINER(main_vbox), menubar); */
    gtk_widget_destroy(menubar);
    menubar = NULL;
    if (item_factory != NULL)
	gtk_object_destroy(GTK_OBJECT(item_factory));
    item_factory = NULL;
    if (accel_group != NULL)
	gtk_accel_group_unref(accel_group);
    accel_group = NULL;
}

void add_main_menu(GtkWidget *window)
{
    MENU_ENTRY *menu_item;
    int menu_len;

    switch (option.language) {
	case IDM_LANGDE:
	    menu_item = menu_de;
	    menu_len = menu_de_len;
	    break;
	case IDM_LANGFR:
	    menu_item = menu_fr;
	    menu_len = menu_fr_len;
	    break;
	case IDM_LANGGR:
	    menu_item = menu_gr;
	    menu_len = menu_gr_len;
	    break;
	case IDM_LANGES:
	    menu_item = menu_es;
	    menu_len = menu_es_len;
	    break;
	case IDM_LANGIT:
	    menu_item = menu_it;
	    menu_len = menu_it_len;
	    break;
	case IDM_LANGNL:
	    menu_item = menu_nl;
	    menu_len = menu_nl_len;
	    break;
	case IDM_LANGSE:
	    menu_item = menu_se;
	    menu_len = menu_se_len;
	    break;
	case IDM_LANGEN:
	default:
	    menu_item = menu_en;
	    menu_len = menu_en_len;
    }

    accel_group = gtk_accel_group_new();

    /* This function initializes the item factory.
       Param 1: The type of menu - can be GTK_TYPE_MENU_BAR, GTK_TYPE_MENU,
              or GTK_TYPE_OPTION_MENU.
       Param 2: The path of the menu.
       Param 3: A pointer to a gtk_accel_group.  The item factory sets up
                the accelerator table while generating menus.
    */

    item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", 
				       accel_group);

    /* This function generates the menu items. Pass the item factory,
       the number of items in the array, the array itself, and any
       callback data for the the menu items. */
    gtk_item_factory_create_items(item_factory, menu_len, 
	(GtkItemFactoryEntry*)menu_item, NULL);

    /* Attach the new accelerator group to the window. */
    gtk_window_add_accel_group(GTK_WINDOW (window), accel_group);

    /* Finally, return the actual menu bar created by the item factory. */ 
    menubar = gtk_item_factory_get_widget(item_factory, "<main>");

    last_file_widget[0] = find_menu_widget(IDM_LASTFILE1);
    last_file_widget[1] = find_menu_widget(IDM_LASTFILE2);
    last_file_widget[2] = find_menu_widget(IDM_LASTFILE3);
    last_file_widget[3] = find_menu_widget(IDM_LASTFILE4);

    /* find edit menu */
    edit_menu = find_menu_widget(IDM_EDITMENU);
    if (edit_menu) {
	/* ask to be notified whenever menu changes */
	edit_menu_tag = gtk_signal_connect(GTK_OBJECT(edit_menu), "show", 
			    GTK_SIGNAL_FUNC(edit_menu_show), NULL);
    }
    else {
	gs_addmess("Can't find edit menu\n");
    }

    gtk_box_pack_start(GTK_BOX(main_vbox), menubar, FALSE, TRUE, 0);
    gtk_box_reorder_child(GTK_BOX(main_vbox), menubar, 0);
    gtk_widget_show (menubar);

    /* add some more accelerators */
    /* Don't know how to do this if there is no corresponding menu item */
/*
	GDK_less, IDM_MAGMINUS
	GDK_greater, IDM_MAGPLUS
	GDK_comma, IDM_MAGMINUS
	GDK_period, IDM_MAGPLUS
	F5, IDM_REDISPLAY
*/
}

#include "binary/gvxback.xpm"
#include "binary/gvxfind.xpm"
#include "binary/gvxfindn.xpm"
#include "binary/gvxfwd.xpm"
#include "binary/gvxgoto.xpm"
#include "binary/gvxhelp.xpm"
#include "binary/gvxinfo.xpm"
#include "binary/gvxmagm.xpm"
#include "binary/gvxmagp.xpm"
#include "binary/gvxnext.xpm"
#include "binary/gvxnexts.xpm"
#include "binary/gvxopen.xpm"
#include "binary/gvxprev.xpm"
#include "binary/gvxprevs.xpm"
#include "binary/gvxprint.xpm"

void button_enter(GtkButton *button, gpointer user_data)
{
    gtk_label_set_text(GTK_LABEL(statusfile), get_string((int)user_data));
   
/* The following doesn't work for magplus/magminus which don't
 * have menu items
 */
/*
    char buf[MAXSTR];
    if (get_menu_string(0, (int)user_data, buf, sizeof(buf)-1))
        gtk_label_set_text(GTK_LABEL(statusfile), buf);
*/
}

void button_leave(GtkButton *button, gpointer user_data)
{
    statusbar_update();
}

/* Add tips to button bar */
static GtkTooltips *tooltips;
typedef struct bar_button_s {
    GtkWidget *button;
    int id;
} bar_button;
static bar_button bar_buttons[20];
static int num_buttons;

void
button_settips(void)
{
    int i;
    if (tooltips == NULL)
        tooltips = gtk_tooltips_new();
    for (i=0; i<num_buttons; i++) {
        gtk_tooltips_set_tip(tooltips, bar_buttons[i].button, 
	    get_string(bar_buttons[i].id), NULL);
    }
}

GtkWidget *bitmap_button(GtkWidget *bar, const char **xpm, int id)
{
    GtkWidget *button;
    GtkWidget *pixmapwid;
    GdkPixmap *pixmap;
    GdkBitmap *mask;
    GtkStyle *style;
    style = gtk_widget_get_style(window);
    pixmap = gdk_pixmap_create_from_xpm_d(window->window, &mask,
	&style->bg[GTK_STATE_NORMAL], (gchar **)xpm);
    pixmapwid = gtk_pixmap_new(pixmap, mask);
    gtk_widget_show(pixmapwid);
    button = gtk_button_new();
    if (num_buttons < sizeof(bar_buttons)/sizeof(bar_button)) {
	bar_buttons[num_buttons].button = button;
	bar_buttons[num_buttons].id = id;
	num_buttons++;
    }
    gtk_container_add(GTK_CONTAINER(button), pixmapwid);
    gtk_box_pack_start(GTK_BOX(bar), button, FALSE, FALSE, 0);
    gtk_signal_connect(GTK_OBJECT(button), "clicked",
		  GTK_SIGNAL_FUNC(gsview_wcmd), (gpointer)id);
    gtk_signal_connect(GTK_OBJECT(button), "enter",
		  GTK_SIGNAL_FUNC(button_enter), (gpointer)id);
    gtk_signal_connect(GTK_OBJECT(button), "leave",
		  GTK_SIGNAL_FUNC(button_leave), (gpointer)id);
    GTK_WIDGET_UNSET_FLAGS(button, GTK_CAN_FOCUS);
    gtk_widget_show(button);
    button_settips();
    return button;
}


void make_buttonbar(void)
{
/*
    GtkWidget *button_textfind;
    GtkWidget *button_textfindnext;
*/
    GtkWidget *label;
    bitmap_button(buttonbar, gvxopen, IDM_OPEN);
    bitmap_button(buttonbar, gvxprint, IDM_PRINT);
    bitmap_button(buttonbar, gvxinfo, IDM_INFO);
    bitmap_button(buttonbar, gvxhelp, IDM_HELPCONTENT);
    label = gtk_label_new(" ");
    gtk_box_pack_start(GTK_BOX(buttonbar), label, FALSE, FALSE, 0);
    gtk_widget_show(label);
    bitmap_button(buttonbar, gvxgoto, IDM_GOTO);
    bitmap_button(buttonbar, gvxprevs, IDM_PREVSKIP);
    bitmap_button(buttonbar, gvxprev, IDM_PREV);
    bitmap_button(buttonbar, gvxnext, IDM_NEXT);
    bitmap_button(buttonbar, gvxnexts, IDM_NEXTSKIP);
    label = gtk_label_new(" ");
    gtk_box_pack_start(GTK_BOX(buttonbar), label, FALSE, FALSE, 0);
    gtk_widget_show(label);
    bitmap_button(buttonbar, gvxback, IDM_GOBACK);
    bitmap_button(buttonbar, gvxfwd, IDM_GOFWD);
    label = gtk_label_new(" ");
    gtk_box_pack_start(GTK_BOX(buttonbar), label, FALSE, FALSE, 0);
    gtk_widget_show(label);
    bitmap_button(buttonbar, gvxmagm, IDM_MAGMINUS);
    bitmap_button(buttonbar, gvxmagp, IDM_MAGPLUS);
    label = gtk_label_new(" ");
    gtk_box_pack_start(GTK_BOX(buttonbar), label, FALSE, FALSE, 0);
    gtk_widget_show(label);
    bitmap_button(buttonbar, gvxfind, IDM_TEXTFIND);
    bitmap_button(buttonbar, gvxfindn, IDM_TEXTFINDNEXT);
}

BOOL button_bar_created = FALSE;

gint
button_bar_configure_event(GtkWidget *widget, GdkEventExpose *event)
{
    if (button_bar_created == FALSE) {
	/* this needs to be deferred until the X Window is created */
	make_buttonbar();
        button_bar_created = TRUE;
    }
    return TRUE; 
}


/* Platform specific preprocessing of arguments */
int
parse_args(GSVIEW_ARGS *args)
{
    char *filename = args->filename;
    debug = args->debug;
#ifdef MULTITHREAD
    multithread = args->multithread;
#else
    multithread = FALSE;
#endif
    if (args->print || args->convert) {
	print_silent = TRUE;
	print_exit = TRUE;
    }
    if (filename[0]) {
	/* make sure filename contains full path */
	char fullname[MAXSTR+MAXSTR];
	if (filename[0]=='/') {
	    /* contains full path */
	    /* make this the work dir */
	    char *t;
	    char filedir[MAXSTR];
	    strcpy(filedir, filename);
	    if ( (t = strrchr(filedir, '/')) != (char *)NULL ) {
		*(++t) = '\0';
		gs_chdir(filedir);
	    }
	}
	else {
	    /* Doesn't include path, so add work dir */
	    int j;
	    strcpy(fullname, workdir);
	    j = strlen(workdir) - 1;
	    if ( (j >= 0) && (fullname[j] != '/'))
		strcat(fullname, "/");
	    strcat(fullname, filename);
	    strncpy(args->filename, fullname, sizeof(args->filename)-1);
	}
    }

    return TRUE;
}


int
gsview_init(int argc, char *argv[])
{
    GtkWidget *con1, *con2, *con3;
    int badarg;
    char *p;

    args.multithread = TRUE;
    badarg = parse_argv(&args, argc, argv);
    parse_args(&args);
    if (badarg) {
        fprintf(stdout, "Bad argument %s\n", argv[badarg]);
	return -1;	/* exit with error */
    }

    if (args.version) {
	fprintf(stdout, "GSview %s %s\n", GSVIEW_DOT_VERSION,
	    GSVIEW_DATE);
	fprintf(stdout, "Documentation files in %s\n", GSVIEW_DOCPATH);
	fprintf(stdout, "Configuration files in %s\n", GSVIEW_ETCPATH);
	return 1;	/* exit now without error */
    }
    else if (args.help) {
	fprintf(stdout, "Usage: %s [options] [filename]\n", argv[0]);
	fprintf(stdout, "Options are:\n");
	fprintf(stdout, " -help\n");
	fprintf(stdout, " -version\n");
	fprintf(stdout, " -geometry WIDTHxHEIGHT\n");
	return 1;	/* exit now without error */
    }

    init_options();
    strcpy(option.printer_queue, "lpr");

    p = getenv("HOME");
    if ((p != NULL) && (strlen(p) + strlen(INIFILE) + 2 < sizeof(szIniFile))) {
	strcpy(szIniFile, p);
	strcat(szIniFile, "/.");
	strcat(szIniFile, INIFILE);
    }
    else {
	gs_addmess("gsview_init: home path too long\n");
        strcpy(szIniFile, ".");
        strcat(szIniFile, INIFILE);
    }
    read_profile(szIniFile);
    use_args(&args);

    view_init(&view);

    if (init_img_message())
	return 1;

#ifdef MULTITHREAD
    if (multithread) {
	pthread_mutex_init(&image.hmutex, NULL);
	pthread_mutex_init(&hmutex_ps, NULL);
        sem_init(&display.event, 0, 0);
    }
#endif

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_signal_connect(GTK_OBJECT (window), "destroy", 
			GTK_SIGNAL_FUNC (quit_gsview), NULL);

    gtk_window_set_title(GTK_WINDOW(window), szAppName);

    /* do not allow window to be entirely off-screen */
    if ((option.img_origin.x != CW_USEDEFAULT) &&
        (option.img_origin.y != CW_USEDEFAULT)) {
	if (option.img_size.x + option.img_origin.x < 16)
	    option.img_origin.x = 0;
	if (option.img_size.y + option.img_origin.y < 16)
	    option.img_origin.y = 0;
    }
    else {
        if ((option.img_origin.x < 0) || (option.img_origin.x > 4096) ||
	    (option.img_origin.y == CW_USEDEFAULT))
	    option.img_origin.x = CW_USEDEFAULT;
        if ((option.img_origin.y < 0) || (option.img_origin.y > 4096) ||
	    (option.img_origin.x == CW_USEDEFAULT))
	    option.img_origin.y = CW_USEDEFAULT;
    }

    /* Set window origin */
    if ((option.img_origin.x != CW_USEDEFAULT) &&
        (option.img_origin.y != CW_USEDEFAULT))
        gtk_widget_set_uposition(window, 
	    option.img_origin.x, option.img_origin.y);

    if (option.img_size.x == CW_USEDEFAULT)
        option.img_size.x = 480;
    if (option.img_size.y == CW_USEDEFAULT)
        option.img_size.y = 480;
    if (option.img_size.x < 200)
	option.img_size.x = 200;
    if (option.img_size.y < 200)
	option.img_size.y = 200;
    gtk_widget_set_usize(window, 
	option.img_size.x, option.img_size.y);
    /* Allow user to resize window */ 
    gtk_window_set_policy(GTK_WINDOW(window), TRUE, TRUE, FALSE);

    /* Pick up key presses */
    gtk_signal_connect (GTK_OBJECT (window), "key_press_event", 
			GTK_SIGNAL_FUNC (key_press_event), 
			NULL);
    gtk_widget_set_events(window, GDK_KEY_PRESS_MASK);
   

    main_vbox = gtk_vbox_new (FALSE, 1);
    gtk_container_border_width (GTK_CONTAINER (main_vbox), 1);
    gtk_container_add (GTK_CONTAINER (window), main_vbox);
    gtk_widget_show (main_vbox);

/*    add_main_menu(window); */
    
    buttonbar = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX (main_vbox), buttonbar, FALSE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (window), "configure_event", 
			GTK_SIGNAL_FUNC (button_bar_configure_event), NULL);
    gtk_widget_show(buttonbar);

    img = gtk_drawing_area_new();
    gtk_drawing_area_size(GTK_DRAWING_AREA(img), 595, 842);
    gtk_signal_connect (GTK_OBJECT (img), "motion_notify_event", 
			GTK_SIGNAL_FUNC (motion_notify_event), 
			NULL);
    gtk_signal_connect (GTK_OBJECT (img), "button_press_event", 
			GTK_SIGNAL_FUNC (button_press_event), 
			NULL);
    gtk_signal_connect (GTK_OBJECT (img), "button_release_event", 
			GTK_SIGNAL_FUNC (button_release_event), 
			NULL);
    gtk_signal_connect (GTK_OBJECT (img), "leave_notify_event", 
			GTK_SIGNAL_FUNC (motion_notify_event), 
			NULL);
    gtk_widget_set_events(img, 
	GDK_LEAVE_NOTIFY_MASK |
	GDK_POINTER_MOTION_MASK |
	GDK_POINTER_MOTION_HINT_MASK |
	GDK_BUTTON_PRESS_MASK |
	GDK_BUTTON_RELEASE_MASK);


    scroll_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_window),
	GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroll_window),
	img);
    gtk_widget_show (scroll_window);

    gtk_box_pack_start (GTK_BOX (main_vbox), scroll_window, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (img), "expose_event", 
			GTK_SIGNAL_FUNC (expose_event), NULL);
    gtk_signal_connect (GTK_OBJECT (img), "configure_event", 
			GTK_SIGNAL_FUNC (configure_event), NULL);
    gtk_signal_connect (GTK_OBJECT (img), "size-allocate", 
			GTK_SIGNAL_FUNC (size_event), NULL);

    statusbar = gtk_hbox_new(TRUE, 10);
    gtk_box_pack_start (GTK_BOX (main_vbox), statusbar, FALSE, FALSE, 0);
    statusfile = gtk_label_new("");
    statuscoord = gtk_label_new("");
    statuspage = gtk_label_new("");
    con1 = gtk_alignment_new(0, 0, 0, 0);
    con2 = gtk_alignment_new(1, 0, 0, 0);
    con3 = gtk_alignment_new(0, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(con1), statusfile);
    gtk_container_add(GTK_CONTAINER(con2), statuscoord);
    gtk_container_add(GTK_CONTAINER(con3), statuspage);
    gtk_box_pack_start (GTK_BOX (statusbar), con1, TRUE, TRUE, 10);
    gtk_box_pack_start (GTK_BOX (statusbar), con2, TRUE, TRUE, 10);
    gtk_box_pack_start (GTK_BOX (statusbar), con3, TRUE, TRUE, 10);
    gtk_widget_show(statusbar);
    gtk_widget_show(con1);
    gtk_widget_show(con2);
    gtk_widget_show(con3);
    gtk_widget_show(statusfile);
    gtk_widget_show(statuscoord);
    gtk_widget_show(statuspage);
    
    gtk_widget_show (img);

    change_language();	/* adds menu bar */
    gtk_widget_show (window);

    gsview_initc(argc, argv);

    info_wait(IDS_NOWAIT);

    return 0;
}

int config_wizard(BOOL bVerbose)
{
    gs_addmess("config_wizard: not implemented\n");
    gsview_printer_profiles();    /* copy printer.ini */
    option.configured = TRUE;
    write_profile();
    return TRUE;
}

int load_language(int language)
{
    switch(language) {
	case IDM_LANGDE:
	case IDM_LANGFR:
	case IDM_LANGGR:
	case IDM_LANGES:
	case IDM_LANGIT:
	case IDM_LANGNL:
	case IDM_LANGSE:
	case IDM_LANGEN:
	return TRUE;
    }
    return FALSE;
}

/* should be in gvx.h */
void check_string_order(STRING_ENTRY *st, int stlen);

void set_usermedia(void)
{
    GtkWidget *w;
    GtkItem *item;
    GtkBin *bin;
    GtkWidget *label;
    int i;
    for (i=0; i<sizeof(usermedia)/sizeof(USERMEDIA); i++) {
	w = find_menu_widget(i+IDM_USERSIZE1);
	if (w) {
	    if (usermedia[i].name[0]) {
		item = &GTK_MENU_ITEM(w)->item;
		bin = &item->bin;
		label = bin->child;
		gtk_label_set_text(GTK_LABEL(label), usermedia[i].name);
		gtk_widget_show(w);
	    }
	    else {
		gtk_widget_hide(w);
	    }
	}
    }
}

void change_language(void)
{
    STRING_ENTRY *st;
    int stlen;

    strcpy(szHelpName, szDocPath);
    switch (option.language) {
	case IDM_LANGDE:
    	    strcat(szHelpName, "gvxde.htm");
	    st = string_de;
	    stlen = string_de_len;
	    break;
	case IDM_LANGFR:
    	    strcat(szHelpName, "gvxfr.htm");
	    st = string_fr;
	    stlen = string_fr_len;
	    break;
	case IDM_LANGGR:
    	    strcat(szHelpName, "gvxgr.htm");
	    st = string_gr;
	    stlen = string_gr_len;
	    break;
	case IDM_LANGES:
    	    strcat(szHelpName, "gvxes.htm");
	    st = string_es;
	    stlen = string_es_len;
	    break;
	case IDM_LANGIT:
    	    strcat(szHelpName, "gvxit.htm");
	    st = string_it;
	    stlen = string_it_len;
	    break;
	case IDM_LANGNL:
    	    strcat(szHelpName, "gvxnl.htm");
	    st = string_nl;
	    stlen = string_nl_len;
	    break;
	case IDM_LANGSE:
    	    strcat(szHelpName, "gvxse.htm");
	    st = string_se;
	    stlen = string_se_len;
	    break;
	case IDM_LANGEN:
	default:
    	    strcat(szHelpName, "gvxen.htm");
	    st = string_en;
	    stlen = string_en_len;
    }
    if (debug & DEBUG_GENERAL)
        check_string_order(st, stlen);

    remove_main_menu(window);
    add_main_menu(window);
    set_usermedia();

    nHelpTopic = IDS_TOPICROOT;
    /* Update the window before we change the check marks */
    gtk_main_iteration_do(FALSE);
    init_check_menu();
    set_last_used();
    measure_close();
    statusbar_update();
    button_settips();
}

int language_value;

void language_select(GtkWidget *w, gpointer data)
{
    language_value = (int)data;
    gtk_main_quit();
}

int get_language(void)
{
    GtkWidget *window;		/* main dialog window */
    GtkWidget *vbox;
    GtkWidget *button_de;
    GtkWidget *button_en;
    GtkWidget *button_es;
    GtkWidget *button_fr;
    GtkWidget *button_gr;
    GtkWidget *button_it;
    GtkWidget *button_nl;
    GtkWidget *button_se;
    int rc = 0;

    language_value = IDM_LANGEN;

    window=gtk_window_new(GTK_WINDOW_DIALOG);
    gtk_signal_connect(GTK_OBJECT(window), "delete-event",
	GTK_SIGNAL_FUNC(modal_delete), &rc);
    gtk_window_set_title(GTK_WINDOW(window), get_string(IDS_AASELECTLANGUAGE));
    gtk_window_set_default_size(GTK_WINDOW(window), 
	180, 100);	/* only width is important */

    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show(vbox);
   
    button_en = gtk_button_new_with_label(get_string(IDS_AAENGLISH));
    gtk_box_pack_start(GTK_BOX(vbox), button_en, TRUE, TRUE, 5);
    gtk_signal_connect(GTK_OBJECT(button_en), "clicked",
		  GTK_SIGNAL_FUNC(language_select), (gpointer)IDM_LANGEN);
    gtk_widget_show(button_en);

    button_de = gtk_button_new_with_label(get_string(IDS_AADEUTSCH));
    gtk_box_pack_start(GTK_BOX(vbox), button_de, TRUE, TRUE, 5);
    gtk_signal_connect(GTK_OBJECT(button_de), "clicked",
		  GTK_SIGNAL_FUNC(language_select), (gpointer)IDM_LANGDE);
    gtk_widget_show(button_de);

    button_fr = gtk_button_new_with_label(get_string(IDS_AAFRANCAIS));
    gtk_box_pack_start(GTK_BOX(vbox), button_fr, TRUE, TRUE, 5);
    gtk_signal_connect(GTK_OBJECT(button_fr), "clicked",
		  GTK_SIGNAL_FUNC(language_select), (gpointer)IDM_LANGFR);
    gtk_widget_show(button_fr);

    button_gr = gtk_button_new_with_label(get_string(IDS_AAGREEK));
    gtk_box_pack_start(GTK_BOX(vbox), button_gr, TRUE, TRUE, 5);
    gtk_signal_connect(GTK_OBJECT(button_gr), "clicked",
		  GTK_SIGNAL_FUNC(language_select), (gpointer)IDM_LANGGR);
    gtk_widget_show(button_gr);

    button_it = gtk_button_new_with_label(get_string(IDS_AAITALIANO));
    gtk_box_pack_start(GTK_BOX(vbox), button_it, TRUE, TRUE, 5);
    gtk_signal_connect(GTK_OBJECT(button_it), "clicked",
		  GTK_SIGNAL_FUNC(language_select), (gpointer)IDM_LANGIT);
    gtk_widget_show(button_it);

    button_es = gtk_button_new_with_label(get_string(IDS_AAESPANOL));
    gtk_box_pack_start(GTK_BOX(vbox), button_es, TRUE, TRUE, 5);
    gtk_signal_connect(GTK_OBJECT(button_es), "clicked",
		  GTK_SIGNAL_FUNC(language_select), (gpointer)IDM_LANGES);
    gtk_widget_show(button_es);

    button_nl = gtk_button_new_with_label(get_string(IDS_AANEDERLANDS));
    gtk_box_pack_start(GTK_BOX(vbox), button_nl, TRUE, TRUE, 5);
    gtk_signal_connect(GTK_OBJECT(button_nl), "clicked",
		  GTK_SIGNAL_FUNC(language_select), (gpointer)IDM_LANGNL);
    gtk_widget_show(button_nl);

    button_se = gtk_button_new_with_label(get_string(IDS_AASVENSKA));
    gtk_box_pack_start(GTK_BOX(vbox), button_se, TRUE, TRUE, 5);
    gtk_signal_connect(GTK_OBJECT(button_se), "clicked",
		  GTK_SIGNAL_FUNC(language_select), (gpointer)IDM_LANGSE);
    gtk_widget_show(button_se);

    /* show dialog and wait for language or close */
    gtk_window_set_focus(GTK_WINDOW(window), button_en);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_show(window);
    gtk_main();
 
    gtk_widget_destroy(window);
 
    return language_value;
}

int check_locale(const char *lang)
{
    if ((tolower(pszLocale[0]) == lang[0]) &&
        (tolower(pszLocale[1]) == lang[1]))
	return 0;	/* match */
    return 1;	/* mismatch */
}

void check_language(void)
{
    if ( (debug & DEBUG_GENERAL)
      || ((option.language == IDM_LANGEN) && check_locale("en"))
      || ((option.language == IDM_LANGDE) && check_locale("de"))
      || ((option.language == IDM_LANGES) && check_locale("es"))
      || ((option.language == IDM_LANGFR) && check_locale("fr"))
      || ((option.language == IDM_LANGGR) && check_locale("gr"))
      || ((option.language == IDM_LANGIT) && check_locale("it"))
      || ((option.language == IDM_LANGNL) && check_locale("nl"))
      || ((option.language == IDM_LANGNL) && check_locale("se"))
	)
    {
        /* GSview language doesn't match locale, prompt user */
	int language = get_language();
	switch (language) {
	    case IDM_LANGEN:
	    case IDM_LANGDE:
	    case IDM_LANGES:
	    case IDM_LANGFR:
	    case IDM_LANGGR:
	    case IDM_LANGIT:
	    case IDM_LANGNL:
	    case IDM_LANGSE:
		gsview_language(language);
	}
    }
}

void post_args(void)
{
/* Don't need to do this, because we use delayed parsing of
 * the command line instead.
 */
}


/***************************/

void *zlib_hinstance;
PFN_gzopen gzopen;
PFN_gzread gzread;
PFN_gzclose gzclose;

void
unload_zlib(void)
{
    if (zlib_hinstance == (void *)NULL)
	return;
    dlclose(zlib_hinstance);
    zlib_hinstance = NULL;
    gzopen = NULL;
    gzread = NULL;
    gzclose = NULL;
}

/* load zlib DLL (libz.so) for gunzip */
BOOL
load_zlib(void)
{   
char buf[MAXSTR];
    char zlibname[] = "libz.so";
    if (zlib_hinstance != (void *)NULL)
	return TRUE;	/* already loaded */

    strcpy(buf, zlibname);
    gs_addmess("Attempting to load ");
    gs_addmess(buf);
    gs_addmess("\n");
    zlib_hinstance = dlopen(buf, RTLD_NOW);
    if (zlib_hinstance != NULL) {
        gzopen = (PFN_gzopen) dlsym(zlib_hinstance, "gzopen");
	if (gzopen == NULL) {
	    unload_zlib();
	}
	else {
	    gzread = (PFN_gzread) dlsym(zlib_hinstance, "gzread");
	    if (gzread == NULL) {
		unload_zlib();
	    }
	    else {
		gzclose = (PFN_gzclose) dlsym(zlib_hinstance, "gzclose");
		if (gzclose == NULL) {
		    unload_zlib();
		}
	    }
	}
    }

    if (zlib_hinstance == NULL) {
	load_string(IDS_ZLIB_FAIL, buf, sizeof(buf));
	if (message_box(buf, MB_OKCANCEL) == IDOK) {
	    nHelpTopic = IDS_TOPICZLIB;
	    get_help();
	}
	return FALSE;
    }
    
    return TRUE;
} 

/***************************/

void * bzip2_hinstance;
PFN_bzopen bzopen;
PFN_bzread bzread;
PFN_bzclose bzclose;

void
unload_bzip2(void)
{
    if (bzip2_hinstance == (void *)NULL)
	return;
    dlclose(bzip2_hinstance);
    bzip2_hinstance = NULL;
    bzopen = NULL;
    bzread = NULL;
    bzclose = NULL;
}

/* load bzip2 DLL for gunzip */
BOOL
load_bzip2(void)
{   
char buf[MAXSTR];
    char bzip2name[] = "libbz2.so";
    if (bzip2_hinstance != (void *)NULL)
	return TRUE;	/* already loaded */

    strcpy(buf, bzip2name);
    gs_addmess("Attempting to load ");
    gs_addmess(buf);
    gs_addmess("\n");
    bzip2_hinstance = dlopen(buf, RTLD_NOW);
    if (bzip2_hinstance != (void *)NULL) {
        bzopen = (PFN_bzopen) dlsym(bzip2_hinstance, "bzopen");
	if (bzopen == NULL) {
	    unload_bzip2();
	}
	else {
	    bzread = (PFN_bzread) dlsym(bzip2_hinstance, "bzread");
	    if (bzread == NULL) {
		unload_bzip2();
	    }
	    else {
		bzclose = (PFN_bzclose) dlsym(bzip2_hinstance, "bzclose");
		if (bzclose == NULL) {
		    unload_bzip2();
		}
	    }
	}
    }

    if (bzip2_hinstance == NULL) {
	load_string(IDS_BZIP2_FAIL, buf, sizeof(buf));
	if (message_box(buf, MB_OKCANCEL) == IDOK) {
	    nHelpTopic = IDS_TOPICBZIP2;
	    get_help();
	}
	return FALSE;
    }
    
    return TRUE;
}


