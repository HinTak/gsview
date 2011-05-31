/* Copyright (C) 1998-2005, Ghostgum Software Pty Ltd.  All rights reserved.
  
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

/* gvxedit.c */
/* Convert PostScript to editable form using pstoedit by Wolfgang Glunz */
#include "gvc.h"
#include "gvcedit.h"
#include <dlfcn.h>

void *pstoeditModule;

PSTOEDIT p2e;

setPstoeditOutputFunction_func * setPstoeditOutputFunction;
getPstoeditDriverInfo_plainC_func * getPstoeditDriverInfo_plainC;
pstoedit_plainC_func * pstoedit_plainC;
pstoedit_checkversion_func * PstoeditCheckVersionFunction;
clearPstoeditDriverInfo_plainC_func * clearPstoeditDriverInfo;
struct DriverDescription_S * pstoedit_driver_info;

BOOL load_pstoedit(void);
BOOL pstoedit_dialog(void); 

int
unload_pstoedit(void)
{
    if (pstoeditModule) {
	gs_addmess("Unloading pstoedit\n");
	/* If we are using the old pstoedit dll 300, we can't
	 * We can't free the driver info because we didn't allocate it.
	 * A small amount of memory will be leaked.
	 * If using pstoedit dll 301, we use clearPstoeditDriverInfo.
	 */
	if ( clearPstoeditDriverInfo != 
	    (clearPstoeditDriverInfo_plainC_func *)NULL ) {
	    clearPstoeditDriverInfo(pstoedit_driver_info);
	}
	pstoedit_driver_info = NULL;
	clearPstoeditDriverInfo = NULL;

	setPstoeditOutputFunction = NULL;
	getPstoeditDriverInfo_plainC = NULL;
	pstoedit_plainC = NULL;
	if (pstoeditModule)
	    dlclose(pstoeditModule);
	pstoeditModule = NULL;
	memset(&p2e, 0, sizeof(p2e));
    }
    return 0;
}


BOOL
load_pstoedit(void)
{
char dllname[MAXSTR];

    if (pstoeditModule)
	return TRUE;

    gs_addmess("Loading pstoedit\n");
    strcpy(dllname, "libpstoedit.so");

    /* load pstoedit DLL */
    pstoeditModule = dlopen(dllname, RTLD_NOW);
    if (pstoeditModule == NULL) {
	gs_addmess("Can't load ");
        gs_addmess(dllname);
        gs_addmess("\n");
	gs_addmess("pstoedit is not available\n");
	gs_addmess("See help topic 'PStoEdit'\n");
	return FALSE;
    }

    /* check version match */
    if ( (PstoeditCheckVersionFunction = (pstoedit_checkversion_func *) 
	dlsym(pstoeditModule, "pstoedit_checkversion"))
	== NULL ) {
	gs_addmess("Can't find pstoedit_checkversion()\n");
	unload_pstoedit();
	return FALSE;
    }
    if (PstoeditCheckVersionFunction(pstoeditdllversion)) {
	/* load the clearPstoeditDriverInfo function, which is only
	 * available in version 301 and later.
	 */
	if ( (clearPstoeditDriverInfo= (clearPstoeditDriverInfo_plainC_func *) 
	    dlsym(pstoeditModule, "clearPstoeditDriverInfo_plainC"))
	    == NULL ) {
	    gs_addmess("Couldn't find clearPstoeditDriverInfo\n");
	    unload_pstoedit();
	    return FALSE;
	}
    }
    else {
	/* We didn't have the latest version.
	 * Check if the older version 300 is available, 
	 * which has memory leak problem but is otherwise OK.
	 */
	char buf[MAXSTR];
	clearPstoeditDriverInfo = (clearPstoeditDriverInfo_plainC_func *)NULL;
	sprintf(buf, "Warning: pstoedit dll version is NOT %d\n", 
		pstoeditdllversion);
	gs_addmess(buf);
        if (!PstoeditCheckVersionFunction(300)) {
	    gs_addmess("wrong version of pstoedit.dll\n");
	    unload_pstoedit();
	    return FALSE;
	}
	else {
	    gs_addmess("Loaded pstoedit dll version 300\n");
	}
    }

#ifdef NOTUSED
 /* not needed because pstoedit starts GS as a separate process */
    /* install callback function */
    if ( (setPstoeditOutputFunction = (setPstoeditOutputFunction_func *) 
	dlsym(pstoeditModule, "setPstoeditOutputFunction"))
	 == NULL ) {
	gs_addmess("Can't find setPstoeditOutputFunction()\n");
	unload_pstoedit();
	return FALSE;
    }
    setPstoeditOutputFunction(0, /* no Call Back Data ptr */
				pstoedit_callback);
#endif

    /* find out which formats (backends) are supported */
    if ( (getPstoeditDriverInfo_plainC = (getPstoeditDriverInfo_plainC_func *)
        dlsym(pstoeditModule, "getPstoeditDriverInfo_plainC"))
	== NULL ) {
	gs_addmess("Can't find getPstoeditDriverInfo_plainC");
	unload_pstoedit();
	return FALSE;
    }
    pstoedit_driver_info = getPstoeditDriverInfo_plainC();

    if ( (pstoedit_plainC = 
	 (pstoedit_plainC_func *) 
        dlsym(pstoeditModule, "pstoedit_plainC"))
	== NULL ) {
	gs_addmess("Can't find pstoedit_plainC" );
    }

    memset(&p2e, 0, sizeof(p2e));
    p2e.driver_info = pstoedit_driver_info;

    return TRUE;
}


BOOL pstoedit_dialog(void)
{
    GtkWidget *window;		/* main dialog window */
    GtkWidget *vbox;
    GtkWidget *scrolled_window;
    GtkWidget *format;		/* list box */
    GtkWidget *polygons;	/* Draw text as polygon */
    GtkWidget *latin1;		/* Map to ISO-Latin1 */
    GtkWidget *table;
    GtkWidget *con;
    GtkWidget *label;
    GtkWidget *flatness;
    GtkWidget *deffont;
    GtkWidget *drvopt;
    GtkWidget *hbox_button;
    GtkWidget *button_ok;
    GtkWidget *button_cancel;
    GtkWidget *button_help;
    gchar *p;
    gchar buf[MAXSTR];
    float u;
    char defformat[MAXSTR];
    struct DriverDescription_S * dd;
    int rc = 0;
    int index;
    int i;

    window=gtk_window_new(GTK_WINDOW_DIALOG);
    gtk_widget_set_usize(GTK_WIDGET(window), 480, 400);

    gtk_window_set_title(GTK_WINDOW(window), "PS to Edit");

    gtk_signal_connect(GTK_OBJECT(window), "delete-event",
                       GTK_SIGNAL_FUNC(modal_delete), &rc);
    
    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show(vbox);
   
    /* Create a label */
    label = gtk_label_new("Format");
    con = gtk_alignment_new(0, 0, 0, 0);	/* left align */
    gtk_container_add(GTK_CONTAINER(con), label);
    gtk_box_pack_start(GTK_BOX(vbox), con, FALSE, FALSE, 0);
    gtk_widget_show(con);
    gtk_widget_show(label);

    /* Create a scrolled window to pack the CList widget into */
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);
    gtk_widget_show (scrolled_window);

    /* Create the CList for the formats */
    format = gtk_clist_new(1);

    gtk_clist_set_shadow_type (GTK_CLIST(format), GTK_SHADOW_OUT);

    /* We must set the default columnn width, in pixels */
    gtk_clist_set_column_width (GTK_CLIST(format), 0, 500);

    /* Add the CList widget and show it. */
    gtk_container_add(GTK_CONTAINER(scrolled_window), format);
    gtk_widget_show(format);

    polygons = gtk_check_button_new_with_label("Draw text as polygonts");
    gtk_box_pack_start(GTK_BOX(vbox), polygons, FALSE, FALSE, 0);
    gtk_widget_show(polygons);

    latin1 = gtk_check_button_new_with_label("Map text to ISO-Latin1");
    gtk_box_pack_start(GTK_BOX(vbox), latin1, FALSE, FALSE, 0);
    gtk_widget_show(latin1);

    table = gtk_table_new(3, 2, FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 5);

    label = gtk_label_new("Flatness:");
    con = gtk_alignment_new(0, 0, 0, 0);	/* left align */
    gtk_container_add(GTK_CONTAINER(con), label);
    flatness = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(flatness), 255);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 0, 1, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table), flatness, 1, 2, 0, 1);
    gtk_widget_show(label);
    gtk_widget_show(con);
    gtk_widget_show(flatness);

    label = gtk_label_new("Default font:");
    con = gtk_alignment_new(0, 0, 0, 0);	/* left align */
    gtk_container_add(GTK_CONTAINER(con), label);
    deffont = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(deffont), 255);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 0, 1, 1, 2);
    gtk_table_attach_defaults(GTK_TABLE(table), deffont, 1, 2, 1, 2);
    gtk_widget_show(label);
    gtk_widget_show(con);
    gtk_widget_show(deffont);

    label = gtk_label_new("Driver options:");
    con = gtk_alignment_new(0, 0, 0, 0);	/* left align */
    gtk_container_add(GTK_CONTAINER(con), label);
    drvopt = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(drvopt), 255);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 0, 1, 2, 3);
    gtk_table_attach_defaults(GTK_TABLE(table), drvopt, 1, 2, 2, 3);
    gtk_widget_show(label);
    gtk_widget_show(con);
    gtk_widget_show(drvopt);

    gtk_widget_show(table);

    /* Create and place the buttons across the bottom */
    hbox_button = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_button, FALSE, FALSE, 5);
    gtk_widget_show(hbox_button);

    button_ok = gtk_button_new_with_label(get_string(IDS_AAOK));
    gtk_box_pack_start(GTK_BOX(hbox_button), button_ok, TRUE, FALSE, 5);
    gtk_signal_connect(GTK_OBJECT(button_ok), "clicked",
			      GTK_SIGNAL_FUNC(modal_ok), &rc);
    gtk_widget_show(button_ok);

    button_cancel = gtk_button_new_with_label(get_string(IDS_AACANCEL));
    gtk_box_pack_start(GTK_BOX(hbox_button), button_cancel, TRUE, FALSE, 5);
    gtk_signal_connect(GTK_OBJECT(button_cancel), "clicked",
			  GTK_SIGNAL_FUNC(modal_cancel), &rc);
    gtk_widget_show(button_cancel);

    button_help = gtk_button_new_with_label(get_string(IDS_AAHELP));
    gtk_box_pack_start(GTK_BOX(hbox_button), button_help, TRUE, FALSE, 5);
    gtk_signal_connect(GTK_OBJECT(button_help), "clicked",
			  GTK_SIGNAL_FUNC(modal_help), &rc);
    gtk_widget_show(button_help);

    gtk_window_set_focus(GTK_WINDOW(window), format);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);

    /* set default value of controls */
    defformat[0] = '\0';
    dd = p2e.driver_info;
    i = index = 0;
    p = buf;
    while(dd && (dd->symbolicname) ) {
	sprintf(buf, "%s:  %s %s", dd->symbolicname, dd->explanation,
		    dd->additionalInfo);
	if (strcmp(dd->symbolicname, p2e.format->symbolicname) == 0) {
	    index = i;
	}
	gtk_clist_append(GTK_CLIST(format), &p);
	dd++;
	i++;
    }
    gtk_clist_select_row(GTK_CLIST(format), index, 0);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(polygons), 
	p2e.draw_text_as_polygon);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(latin1), 
	p2e.map_to_latin1);
    if (p2e.flatness > 0) {
	sprintf(buf, "%g", p2e.flatness);
        gtk_entry_set_text(GTK_ENTRY(flatness), buf);
    }
    gtk_entry_set_text(GTK_ENTRY(deffont), p2e.default_font);
    gtk_entry_set_text(GTK_ENTRY(drvopt), p2e.driver_option);

    gtk_widget_show(window);

    gtk_main();
 
    if (rc == IDOK) {
	GList *l = GTK_CLIST(format)->selection;
	i = 0;
	index = (int)((size_t)(l->data));
	dd = p2e.driver_info;
	p2e.format = dd;
	while(dd && (dd->symbolicname) ) {
	    if (i == index) {
		p2e.format = dd;
		break;
	    }
	    i++;
	    dd++;
	}
	p2e.draw_text_as_polygon = 
	    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(polygons));
	p2e.map_to_latin1 = 
	    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(latin1));
	strncpy(buf, gtk_entry_get_text(GTK_ENTRY(flatness)), sizeof(buf)-1);
	if ( (sscanf(buf, "%f", &u) == 1) && (u > 0.0))
	    p2e.flatness = u;
	else
	    p2e.flatness = 0.0;
	strncpy(p2e.default_font, gtk_entry_get_text(GTK_ENTRY(deffont)), 
	    sizeof(p2e.default_font)-1);
	strncpy(p2e.driver_option, gtk_entry_get_text(GTK_ENTRY(drvopt)), 
	    sizeof(p2e.driver_option)-1);
    }

    gtk_widget_destroy(window);

    return (rc == IDOK);
}

int
gsview_pstoedit(void)
{
BOOL flag;
int pagenum;
    flag = load_pstoedit();
    load_pstoedit_options();

    if (flag) {
        nHelpTopic = IDS_TOPICPSTOEDIT;
	flag = pstoedit_dialog();
    }
    if (flag)
        save_pstoedit_options();

    if (flag) {
	pagenum = psfile.pagenum;
	if ( (psfile.dsc != (CDSC *)NULL) && (psfile.dsc->page_count != 0) )
	  flag = get_page(&pagenum, 
	    p2e.format->backendSupportsMultiplePages, FALSE);
    }

    if (flag) {
	/* get filename */
	/* Specify extension would be nice, but it is hard to do
	 * with the current get_filename interface.
	 */
	flag = get_filename(p2e.output_filename, TRUE, FILTER_ALL, 
		0, IDS_TOPICPSTOEDIT);
    }

    if (debug && flag) {
      char buf[MAXSTR];
      sprintf(buf, "format=%s dt=%d latin1=%d flat=%g\nfont=%s option=%s\n",
        p2e.format->symbolicname, p2e.draw_text_as_polygon,
	p2e.map_to_latin1, p2e.flatness,
	p2e.default_font, p2e.driver_option);
      gs_addmess(buf);
      sprintf(buf, "filename=%s\n", p2e.output_filename);
      gs_addmess(buf);
    }

    if (flag)
        save_pstoedit_options();

    /* extract pages to temporary file */
    if (flag)
	flag = extract_temp(p2e.temp_filename);

    if (!flag) {
	unload_pstoedit();
	post_img_message(WM_GSSHOWMESS, 0);
	return FALSE;
    }

    
    /* will need to do the remaining items on the GS thread */

    /* pstoedit will probably run Ghostscript as a separate process,
     * but it might be changed to use libgs.so at some stage, so
     * play safe and unload Ghostscript first.
     */
    /* shut down Ghostscript */
    pending.unload = TRUE;	
    pending.now = TRUE;	

    pending.pstoedit = TRUE;

    return flag;
}


