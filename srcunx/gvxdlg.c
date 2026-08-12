/* Copyright (C) 1993-2005, Ghostgum Software Pty Ltd.  All rights reserved.
  
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

/* gvxdlg.cpp */

#include "gvx.h"
#include "en/gvclang.h"
gint modal_delete(GtkWidget *widget, GdkEvent *event, gpointer data);
void modal_ok(GtkWidget *w, gpointer data);
void modal_cancel(GtkWidget *w, gpointer data);
void modal_yes(GtkWidget *w, gpointer data);
void modal_no(GtkWidget *w, gpointer data);
void modal_ignore(GtkWidget *w, gpointer data);
int modal_dialog(GtkWidget *w, GtkWidget *okw, GtkWidget *cancelw);
void info_add(GtkWidget *table, const char *str1, const char *str2, int *y);
void add_about(GtkWidget *w, const char *str);
void setting_add(GtkWidget *table, const char *str1, GtkWidget *w, const char *str3, int *y);
void set_menu_sensitive(void);
int depth_to_index(int depth);
int alpha_to_index(int alpha);
int draw_to_index(int draw);
gint showmess_realize(GtkWidget *widget, gpointer user_data);
void cfg_defaults(GtkWidget *w, gpointer data);

void width_percent(GtkWidget *w, GtkRequisition *req, gpointer data)
{
    req->width = req->width * ((int)((size_t)data)) / 100;
}

gint modal_delete(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    if (debug & DEBUG_GENERAL)
	gs_addmess("modal_delete:\n");
    *((int *)data) = IDCANCEL;
    gtk_main_quit();
    /* Do not emit "destroy" signal.
     * We will destroy the window ourselves.
     */
    return TRUE;
}

void modal_ok(GtkWidget *w, gpointer data)
{
    if (debug & DEBUG_GENERAL)
	gs_addmess("modal_ok:\n");
    *((int *)data) = IDOK;
    gtk_main_quit();
}

void modal_cancel(GtkWidget *w, gpointer data)
{
    if (debug & DEBUG_GENERAL)
	gs_addmess("modal_cancel:\n");
    *((int *)data) = IDCANCEL;
    gtk_main_quit();
}

void modal_yes(GtkWidget *w, gpointer data)
{
    if (debug & DEBUG_GENERAL)
	gs_addmess("modal_yes:\n");
    *((int *)data) = IDYES;
    gtk_main_quit();
}

void modal_no(GtkWidget *w, gpointer data)
{
    if (debug & DEBUG_GENERAL)
	gs_addmess("modal_no:\n");
    *((int *)data) = IDNO;
    gtk_main_quit();
}

void modal_ignore(GtkWidget *w, gpointer data)
{
    if (debug & DEBUG_GENERAL)
	gs_addmess("modal_ignore:\n");
    *((int *)data) = DSC_IGNORE_ALL;
    gtk_main_quit();
}

int modal_dialog(GtkWidget *w, GtkWidget *okw, GtkWidget *cancelw) 
{
    int rc = 0;
    if (okw == NULL)
	return 0;
    gtk_widget_show(w);
    gtk_window_set_focus(GTK_WINDOW(w), okw);
    gtk_window_set_modal(GTK_WINDOW(w), TRUE);

    gtk_signal_connect(GTK_OBJECT(okw), "clicked",
	GTK_SIGNAL_FUNC(modal_ok), &rc);
    if (cancelw)
	gtk_signal_connect(GTK_OBJECT(cancelw), "clicked",
	    GTK_SIGNAL_FUNC(modal_cancel), &rc);

    gtk_signal_connect(GTK_OBJECT(w), "delete-event",
	GTK_SIGNAL_FUNC(modal_delete), &rc);
    
    gtk_main();

    if (debug & DEBUG_GENERAL)
	gs_addmessf("modal_dialog: returns %d\n", rc);

    return rc;
}

void modal_help(GtkWidget *w, gpointer data)
{
    if (debug & DEBUG_GENERAL)
	gs_addmess("modal_help:\n");
    get_help();
}

BOOL get_filename(char *filename, BOOL save, int filter, int title, int help)
{
    GtkWidget *filew;
    char szTitle[MAXSTR];
    int rc;
    char *newfilename;
    if (title)
	load_string(title, szTitle, sizeof(szTitle));
    else
	strcpy(szTitle, save ? "Save As" : "Open");
    filew = gtk_file_selection_new(szTitle);
    if (filew == NULL)
	return FALSE;
    if (filename != NULL)
	gtk_file_selection_set_filename(GTK_FILE_SELECTION(filew), filename);

    rc = modal_dialog(filew, GTK_FILE_SELECTION(filew)->ok_button,
	GTK_FILE_SELECTION(filew)->cancel_button);

    if (rc == IDOK) {
	newfilename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(filew));
	strncpy(filename, newfilename, MAXSTR-1);
        make_cwd(filename);
    }

    gtk_widget_destroy(filew);
    
    return rc == IDOK;
}

/* display message */
int message_box(const char *str, int icon)
{
    GtkWidget *window;		/* main dialog window */
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *button_ok = NULL , *button_cancel = NULL;
    GtkWidget *button_yes = NULL, *button_no = NULL;
    int rc = 0;
    int type = icon & MB_TYPEMASK;

    window=gtk_window_new(GTK_WINDOW_TOPLEVEL); gtk_window_set_decorated(GTK_WINDOW(window), FALSE);

    gtk_window_set_title(GTK_WINDOW(window), szAppName);

    gtk_signal_connect(GTK_OBJECT(window), "delete-event",
                       GTK_SIGNAL_FUNC(modal_delete), &rc);
    
    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show(vbox);
   
    /* Create a label */
    label = gtk_label_new(str);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
    gtk_widget_show(label);

    /* Create and place the buttons across the bottom */
    hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
    gtk_widget_show(hbox);

    if ((type == MB_OK) || (type == MB_OKCANCEL)) {
	button_ok = gtk_button_new_with_label(get_string(IDS_AAOK));
	gtk_box_pack_start(GTK_BOX(hbox), button_ok, TRUE, FALSE, 5);
	gtk_signal_connect(GTK_OBJECT(button_ok), "clicked",
				  GTK_SIGNAL_FUNC(modal_ok), &rc);
	gtk_widget_show(button_ok);
    }

    if (type == MB_OKCANCEL) {
	button_cancel = gtk_button_new_with_label(get_string(IDS_AACANCEL));
	gtk_box_pack_start(GTK_BOX(hbox), button_cancel, TRUE, FALSE, 5);
	gtk_signal_connect(GTK_OBJECT(button_cancel), "clicked",
			      GTK_SIGNAL_FUNC(modal_cancel), &rc);
	gtk_widget_show(button_cancel);
    }

    if (type == MB_YESNO) {
	button_yes = gtk_button_new_with_label(get_string(IDS_AAYES));
	gtk_box_pack_start(GTK_BOX(hbox), button_yes, TRUE, FALSE, 5);
	gtk_signal_connect(GTK_OBJECT(button_yes), "clicked",
				  GTK_SIGNAL_FUNC(modal_yes), &rc);
	gtk_widget_show(button_yes);
	button_no = gtk_button_new_with_label(get_string(IDS_AANO));
	gtk_box_pack_start(GTK_BOX(hbox), button_no, TRUE, FALSE, 5);
	gtk_signal_connect(GTK_OBJECT(button_no), "clicked",
				  GTK_SIGNAL_FUNC(modal_no), &rc);
	gtk_widget_show(button_no);
    }

    /* show dialog and wait for OK, Cancel, Yes, No, or close */
#ifdef NOTUSED
    if (button_ok)
	gtk_window_set_focus(GTK_WINDOW(window), button_ok);
#endif
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_show(window);
    gtk_main();
 
    gtk_widget_destroy(window);

    return rc;
}

BOOL
query_string(const char *prompt, char *answer)
{
    GtkWidget *window;		/* main dialog window */
    GtkWidget *hbox;		/* hbox for buttons below */
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *button_ok, *button_cancel, *button_help;
    int rc = 0;

    window=gtk_window_new(GTK_WINDOW_TOPLEVEL); gtk_window_set_decorated(GTK_WINDOW(window), FALSE);

    gtk_window_set_title(GTK_WINDOW(window), get_string(IDS_AAINPUT));

    gtk_signal_connect(GTK_OBJECT(window), "delete-event",
                       GTK_SIGNAL_FUNC(modal_delete), &rc);
    
    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show(vbox);
   
    /* Create a label */
    label = gtk_label_new(prompt);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
    gtk_widget_show(label);

    /* Create entry */
    entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry), 255);
    gtk_entry_set_text(GTK_ENTRY(entry), answer);
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 5);
    gtk_widget_show(entry);
    /* need to set a default size for entry */

    /* Create and place the buttons across the bottom */
    hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
    gtk_widget_show(hbox);

    button_ok = gtk_button_new_with_label(get_string(IDS_AAOK));
    button_cancel = gtk_button_new_with_label(get_string(IDS_AACANCEL));
    button_help = gtk_button_new_with_label(get_string(IDS_AAHELP));

    gtk_box_pack_start(GTK_BOX(hbox), button_ok, TRUE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), button_cancel, TRUE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), button_help, TRUE, FALSE, 5);

    /* Connect our callbacks to the three buttons */
    gtk_signal_connect(GTK_OBJECT(button_ok), "clicked",
                              GTK_SIGNAL_FUNC(modal_ok), &rc);
    gtk_signal_connect(GTK_OBJECT(button_cancel), "clicked",
                              GTK_SIGNAL_FUNC(modal_cancel), &rc);
    gtk_signal_connect(GTK_OBJECT(button_help), "clicked",
                              GTK_SIGNAL_FUNC(modal_help), &rc);

    gtk_widget_show(button_ok);
    gtk_widget_show(button_cancel);
    gtk_widget_show(button_help);

    /* Hitting return in entry field is the same as OK */
    gtk_signal_connect(GTK_OBJECT(entry), "activate",
                              GTK_SIGNAL_FUNC(modal_ok), &rc);

    /* show dialog and wait for OK, Cancel or close */
    gtk_window_set_focus(GTK_WINDOW(window), entry);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_show(window);
    gtk_main();
 
    if (rc == IDOK)
	strncpy(answer, gtk_entry_get_text(GTK_ENTRY(entry)), 255);

    gtk_widget_destroy(window);

    return (rc == IDOK);
}

/* User clicked the "All" button. */
void selpage_all(GtkButton *button, gpointer data)
{
    if (debug & DEBUG_GENERAL)
	gs_addmess("button All pressed\n");
    gtk_clist_select_all(GTK_CLIST(data));
    return;
}

/* User clicked the "Odd" button. */
void selpage_odd(GtkButton *button, gpointer data)
{
    GtkCList *clist = GTK_CLIST(data);
    int i;
    if (debug & DEBUG_GENERAL)
	gs_addmess("button Odd pressed\n");
    for (i=0; i<clist->rows; i++) {
	if (i & 1)
	    gtk_clist_unselect_row(clist, i, 0);
	else
	    gtk_clist_select_row(clist, i, 0);
    }
    return;
}

/* User clicked the "Even" button. */
void selpage_even(GtkButton *button, gpointer data)
{
    GtkCList *clist = GTK_CLIST(data);
    int i;
    if (debug & DEBUG_GENERAL)
	gs_addmess("button Even pressed\n");
    for (i=0; i<clist->rows; i++) {
	if (i & 1)
	    gtk_clist_select_row(clist, i, 0);
	else
	    gtk_clist_unselect_row(clist, i, 0);
    }
    return;
}


/* Get page number from dialog box and store in ppage */
/* multiple is TRUE if multiple pages may be selected */
/* allpages is TRUE if all pages should be initially selected */
BOOL
get_page(int *ppage, BOOL multiple, BOOL allpages)
{
    GtkWidget *window;		/* main dialog window */
    GtkWidget *hbox;		/* hbox for list on left, buttons on right */
    GtkWidget *vbox;		/* vbox on right for buttons */
    GtkWidget *scrolled_window, *clist;	/* page list */
    GtkWidget *button_ok, *button_cancel;
    GtkWidget *button_all=NULL, *button_odd=NULL, *button_even=NULL;    
    GtkWidget *button_reverse=NULL;
    int rc = 0;
    int i;
    char buf[MAXSTR];
    char *p;

    if (psfile.dsc == (CDSC *)NULL)
	return FALSE;

    if (psfile.dsc->page_count == 0) {
	gserror(IDS_NOPAGE, NULL, MB_ICONEXCLAMATION, SOUND_NONUMBER);
	return FALSE;
    }

    /* Make psfile.page_list.reverse sticky */
    /* psfile.page_list.reverse = FALSE; */
    psfile.page_list.current = *ppage - 1;
    psfile.page_list.multiple = multiple;
    if (psfile.page_list.select == (BOOL *)NULL)
	return FALSE;

    memset(psfile.page_list.select, 0, psfile.dsc->page_count * sizeof(BOOL) );
    if (multiple) {
	for (i=0; i< (int)(psfile.dsc->page_count); i++)
	    psfile.page_list.select[i] = allpages;
    }
    psfile.page_list.select[psfile.page_list.current] = TRUE;

    window=gtk_window_new(GTK_WINDOW_TOPLEVEL); gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_widget_set_usize(GTK_WIDGET(window), 200, 300);

    gtk_window_set_title(GTK_WINDOW(window), 
	multiple ? get_string(IDS_AASELECTPAGES) : get_string(IDS_AASELECTPAGE));

    gtk_signal_connect(GTK_OBJECT(window),
                       "delete-event",
                       GTK_SIGNAL_FUNC(modal_delete),
                       &rc);
    
    hbox = gtk_hbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_container_add(GTK_CONTAINER(window), hbox);
    gtk_widget_show(hbox);
   
    /* Create a scrolled window to pack the CList widget into */
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

    gtk_box_pack_start(GTK_BOX(hbox), scrolled_window, TRUE, TRUE, 0);
    gtk_widget_show (scrolled_window);

    /* Create the CList for the pages */
    clist = gtk_clist_new(1);

    gtk_clist_set_shadow_type (GTK_CLIST(clist), GTK_SHADOW_OUT);

    /* We must set the default columnn width, in pixels */
    gtk_clist_set_column_width (GTK_CLIST(clist), 0, 64);

    /* Add the CList widget to the horizontal box and show it. */
    gtk_container_add(GTK_CONTAINER(scrolled_window), clist);
    gtk_widget_show(clist);

    /* Create and place the buttons in a vbox at the right */
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);
    gtk_widget_show(vbox);

    button_ok = gtk_button_new_with_label(get_string(IDS_AAOK));
    button_cancel = gtk_button_new_with_label(get_string(IDS_AACANCEL));
    if (multiple) {
	button_all = gtk_button_new_with_label(get_string(IDS_AAALL));
	button_odd = gtk_button_new_with_label(get_string(IDS_AAODD));
	button_even = gtk_button_new_with_label(get_string(IDS_AAEVEN));
	button_reverse = gtk_check_button_new_with_label(get_string(IDS_AAREVERSE));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_reverse), 
		psfile.page_list.reverse);
    }

    gtk_box_pack_start(GTK_BOX(vbox), button_ok, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), button_cancel, FALSE, FALSE, 5);
    if (multiple) {
	gtk_box_pack_start(GTK_BOX(vbox), button_all, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), button_odd, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), button_even, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), button_reverse, FALSE, FALSE, 5);
    }

    /* Connect our callbacks to the three buttons */
    gtk_signal_connect(GTK_OBJECT(button_ok), "clicked",
                              GTK_SIGNAL_FUNC(modal_ok), &rc);
    gtk_signal_connect(GTK_OBJECT(button_cancel), "clicked",
                              GTK_SIGNAL_FUNC(modal_cancel), &rc);

    if (multiple) {
	gtk_signal_connect(GTK_OBJECT(button_all), "clicked",
				  GTK_SIGNAL_FUNC(selpage_all),
				  (gpointer) clist);
	gtk_signal_connect(GTK_OBJECT(button_odd), "clicked",
				  GTK_SIGNAL_FUNC(selpage_odd),
				  (gpointer) clist);
	gtk_signal_connect(GTK_OBJECT(button_even), "clicked",
				  GTK_SIGNAL_FUNC(selpage_even),
				  (gpointer) clist);
    }

    gtk_widget_show(button_ok);
    gtk_widget_show(button_cancel);
    if (multiple) {
	gtk_widget_show(button_all);
	gtk_widget_show(button_odd);
	gtk_widget_show(button_even);
	gtk_widget_show(button_reverse);
    }

    if (multiple)
	gtk_clist_set_selection_mode(GTK_CLIST(clist), GTK_SELECTION_EXTENDED);

    p = buf;
    for (i=0; i< (int)(psfile.dsc->page_count); i++) {
	page_ordlabel(buf, i);
	gtk_clist_append( (GtkCList *) clist, &p);
        if ((multiple && psfile.page_list.select[i])
	    || (i == psfile.page_list.current)) {
	    gtk_clist_select_row(GTK_CLIST(clist), i, 0);
	}
    }

    /* show dialog and wait for OK, Cancel or close */
    gtk_window_set_focus(GTK_WINDOW(window), button_ok);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_show(window);
    gtk_main();

    if (rc == IDOK) {
	GList *l = GTK_CLIST(clist)->selection;
	for (i = 0; i < (int)psfile.dsc->page_count; i++)
	    psfile.page_list.select[i] = FALSE;
	while (l) {
	    psfile.page_list.current = (int)((size_t)(l->data));
	    psfile.page_list.select[(int)((size_t)(l->data))] = TRUE;
	    l = g_list_next(l);
	}
	if (psfile.page_list.current >= 0)
	    *ppage = psfile.page_list.current + 1;
	if (multiple)
	    psfile.page_list.reverse = 
	        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button_reverse));
    }

    gtk_widget_destroy(window);

    return (rc == IDOK);
}

void info_add(GtkWidget *table, const char *str1, const char *str2, int *y)
{
    GtkWidget *label1 = gtk_label_new(str1);
    GtkWidget *label2 = gtk_label_new(str2);
    GtkWidget *con1, *con2;
    con1 = gtk_alignment_new(1, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(con1), label1);
    con2 = gtk_alignment_new(0, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(con2), label2);
    gtk_widget_show(label1);
    gtk_widget_show(label2);
    gtk_widget_show(con1);
    gtk_widget_show(con2);
    gtk_table_attach_defaults(GTK_TABLE(table), con1, 0, 1, *y, (*y)+1);
    gtk_table_attach_defaults(GTK_TABLE(table), con2, 2, 3, *y, (*y)+1);
    (*y)++;
}

void show_info(void)
{
    GtkWidget *window;		/* main dialog window */
    GtkWidget *table;
    GtkWidget *button_ok;
    GtkWidget *spacer;
    CDSC *dsc = psfile.dsc;
    char buf[MAXSTR];
    char *p;
    int y = 0;
    int n;

    window=gtk_window_new(GTK_WINDOW_TOPLEVEL); gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_window_set_title(GTK_WINDOW(window), get_string(IDS_AAINFO));

    table = gtk_table_new(12, 3, FALSE);
    gtk_container_add(GTK_CONTAINER(window), table);

    if (psfile.name[0] != '\0') {
	info_add(table, get_string(IDS_AAFILEC), psfile.name, &y);
	if (dsc) {
	    p = buf;
	    *p = '\0';
	    if (psfile.gzip) {
		strcpy(p, "gzip ");
	        p += strlen(p);
	    }
	    if (psfile.bzip2) {
		strcpy(p, "bzip2 ");
	        p += strlen(p);
	    }
	    if (psfile.ctrld)
		load_string(IDS_CTRLD, p, sizeof(buf));
	    if (psfile.pjl)
		load_string(IDS_PJL, p, sizeof(buf));
	    p += strlen(p);
	    if (psfile.ispdf) {
		load_string(IDS_PDF, p, sizeof(buf)-strlen(buf));
	    }
	    else if (dsc->epsf) {
		switch (psfile.preview) {
		    case IDS_EPSI:
		      load_string(IDS_EPSI, p, sizeof(buf)-strlen(buf));
		      break;
		    case IDS_EPST:
		      load_string(IDS_EPST, p, sizeof(buf)-strlen(buf));
		      break;
		    case IDS_EPSW:
		      load_string(IDS_EPSW, p, sizeof(buf)-strlen(buf));
		      break;
		    default:
		      load_string(IDS_EPSF, p, sizeof(buf)-strlen(buf));
		}
	    }
	    else
		load_string(IDS_DSC, p, sizeof(buf)-strlen(buf));
	    info_add(table, get_string(IDS_AATYPEC), buf, &y);
	    info_add(table, get_string(IDS_AATITLEC), dsc->dsc_title ? dsc->dsc_title : "", &y);
	    info_add(table, get_string(IDS_AADATEC), dsc->dsc_date ? dsc->dsc_date : "", &y);
	    if (dsc->bbox)
		sprintf(buf, "%d %d %d %d", dsc->bbox->llx, dsc->bbox->lly, 
		    dsc->bbox->urx, dsc->bbox->ury);
	    else
		buf[0]='\0';
	    info_add(table, get_string(IDS_AABOUNDINGBOXC), buf, &y);
	    switch(dsc->page_orientation) {
		case CDSC_LANDSCAPE:
			load_string(IDS_LANDSCAPE, buf, sizeof(buf));
			break;
		case CDSC_PORTRAIT:
			load_string(IDS_PORTRAIT, buf, sizeof(buf));
			break;
		default:
			buf[0] = '\0';
	    }
	    info_add(table, get_string(IDS_AAORIENTATIONC), buf, &y);
	    switch(dsc->page_order) {
		case CDSC_ASCEND: 
			load_string(IDS_ASCEND, buf, sizeof(buf));
			break;
		case CDSC_DESCEND: 
			load_string(IDS_DESCEND, buf, sizeof(buf));
			break;
		case CDSC_SPECIAL:
			load_string(IDS_SPECIAL, buf, sizeof(buf));
			break;
		default:
			buf[0] = '\0';
	    }
	    info_add(table, get_string(IDS_AAPAGEORDERC), buf, &y);
	    if (dsc->page_media && dsc->page_media->name) {
		sprintf(buf,"%.200s %g %g",dsc->page_media->name,
		    dsc->page_media->width, dsc->page_media->height);
	    }
	    else {
		buf[0] = '\0';
	    }
	    info_add(table, get_string(IDS_AADEFAULTMEDIAC), buf, &y);
	    sprintf(buf, "%d", dsc->page_count);
	    info_add(table, get_string(IDS_AAPAGESC), buf, &y);
	    n = map_page(psfile.pagenum - 1);
	    if (dsc->page_count)
		    sprintf(buf, "\"%.200s\"   %d", dsc->page[n].label ? 
			dsc->page[n].label : "", psfile.pagenum);
	    else
		    buf[0] = '\0';
	    info_add(table, get_string(IDS_AAPAGEC), buf, &y);
	}
    }
    else
	info_add(table, get_string(IDS_AAFILEC), get_string(IDS_NOFILE), &y);

    /* add a blank line and set minimum spacing */
    spacer = gtk_label_new("   ");
    gtk_widget_show(spacer);
    gtk_table_attach_defaults(GTK_TABLE(table), spacer, 1, 2, y, y+1);
    info_add(table, "        ", "                              ", &y);

    button_ok = gtk_button_new_with_label("OK");
    gtk_widget_show(button_ok);
    gtk_table_attach_defaults(GTK_TABLE(table), button_ok, 0, 1, y, y+1);

    gtk_widget_show(GTK_WIDGET(table));
    modal_dialog(window, button_ok, NULL);
    gtk_widget_destroy(window);
}


void add_about(GtkWidget *w, const char *str)
{
    GtkWidget *label = label = gtk_label_new(str);
    GtkWidget *align = gtk_alignment_new(0, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), label);
    gtk_box_pack_start(GTK_BOX(w), align, FALSE, FALSE, 0);
    gtk_widget_show(label);
    gtk_widget_show(align);
}

void show_about(void)
{
    GtkWidget *window;		/* main dialog window */
    GtkWidget *vbox;
    GtkWidget *button_ok = NULL;
    char buf[MAXSTR];
    int rc = 0;

    gs_addmess("show_about: not implemented fully\n");

    window=gtk_window_new(GTK_WINDOW_TOPLEVEL); gtk_window_set_decorated(GTK_WINDOW(window), FALSE);

    gtk_window_set_title(GTK_WINDOW(window), get_string(IDS_AAABOUTX11));
    gtk_signal_connect(GTK_OBJECT(window), "delete-event",
                       GTK_SIGNAL_FUNC(modal_delete), &rc);
    
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show(vbox);
   
    sprintf(buf, "GSview   %s %s    %s", get_string(IDS_AACOPY1), GSVIEW_DOT_VERSION,
	GSVIEW_DATE);
    add_about(vbox, buf);
    add_about(vbox, get_string(IDS_AACOPY2));
    add_about(vbox,"");
    add_about(vbox, GSVIEW_COPYRIGHT1);
    add_about(vbox, GSVIEW_COPYRIGHT2);
    add_about(vbox,"");
    add_about(vbox, get_string(IDS_AACOPY4));
    add_about(vbox, get_string(IDS_AACOPY5));
    add_about(vbox, get_string(IDS_AACOPY6));
    add_about(vbox, get_string(IDS_AACOPY7));
    add_about(vbox, get_string(IDS_AACOPY8));
    add_about(vbox, get_string(IDS_AACOPY9));
    add_about(vbox, get_string(IDS_AACOPY10));
    add_about(vbox, get_string(IDS_AACOPY11));
    add_about(vbox, get_string(IDS_AACOPY12));
    add_about(vbox, get_string(IDS_AACOPY13));
    add_about(vbox, get_string(IDS_AACOPY14));
    add_about(vbox,"");
    add_about(vbox, get_string(IDS_AACOPY19));
    add_about(vbox,"");
/*
ICON ID_GSVIEW, ABOUT_ICON, 8, 8, 18, 20
*/
    add_about(vbox, "");
    add_about(vbox, "");

    /* Create and place OK button at the bottom */
    button_ok = gtk_button_new_with_label(get_string(IDS_AAOK));
    gtk_box_pack_start(GTK_BOX(vbox), button_ok, FALSE, FALSE, 5);
    gtk_signal_connect(GTK_OBJECT(button_ok), "clicked",
			  GTK_SIGNAL_FUNC(modal_ok), &rc);
    gtk_widget_show(button_ok);

    /* show dialog and wait for OK or close */
    gtk_window_set_focus(GTK_WINDOW(window), button_ok);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_show(window);
    gtk_main();
 
    gtk_widget_destroy(window);
}

typedef struct {
    PSBBOX *bbox;
    GtkWidget *label;
    int bboxindex;
} GETBBOX;

GETBBOX bbox_data;

void bbox_click(void)
{
    float x, y;
    GETBBOX *gb = &bbox_data;
    if (debug & DEBUG_GENERAL)
	gs_addmessf("bbox_click: index=%d\n", gb->bboxindex);
    if (!get_cursorpos(&x, &y)) {
	gtk_main_quit();
    }
    switch(gb->bboxindex) {
	case 0:
	    gb->bbox->llx = (int)x;
	    break;
	case 1:
	    gb->bbox->lly = (int)y;
	    break;
	case 2:
	    gb->bbox->urx = (int)x;
	    break;
	case 3:
	    gb->bbox->ury = (int)y;
	    gb->bbox->valid = TRUE;
	    gtk_main_quit();
	    break;
    }
    gb->bboxindex++;
    if (gb->bboxindex <= 3) {
        gtk_label_set_text(GTK_LABEL(gb->label), 
		get_string(IDS_BBPROMPT+gb->bboxindex));
    }
}


BOOL get_bbox(void)
{
    int rc;
    GtkWidget *window;
    GETBBOX *gb = &bbox_data;

    gb->bbox = &bbox;
    gb->bbox->valid = FALSE;
    gb->bbox->llx = gb->bbox->lly = gb->bbox->urx = gb->bbox->ury = 0;
    gb->bboxindex = 0;
    if ((gsdll.state != GS_PAGE) && (gsdll.state != GS_IDLE)) {
	gserror(IDS_EPSNOBBOX, NULL, MB_ICONEXCLAMATION, SOUND_ERROR);
	return FALSE;
    }

    /* display modeless dialog */
    window=gtk_window_new(GTK_WINDOW_TOPLEVEL); gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_title(GTK_WINDOW(window), get_string(IDS_AABOUNDINGBOX));
    gtk_signal_connect(GTK_OBJECT(window), "delete-event",
                       GTK_SIGNAL_FUNC(modal_delete), &rc);

    gb->label = gtk_label_new(get_string(IDS_BBPROMPT+gb->bboxindex));
    gtk_container_set_border_width(GTK_CONTAINER(gb->label), 10);
    gtk_container_add(GTK_CONTAINER(window), gb->label);
    gtk_widget_show(gb->label);
    gtk_widget_show(window);
    /* this is not modal, since we want the image scroll bars to work */

    getting_bbox = TRUE;
    gtk_main();
    gtk_widget_destroy(window);
    getting_bbox = FALSE;

    return bbox.valid;
}



BOOL pstoeps_warn(void)
{
    GtkWidget *window;		/* main dialog window */
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *check;
    GtkWidget *button_yes = NULL, *button_no = NULL, *button_help;
    int rc = 0;

    nHelpTopic = IDS_TOPICPSTOEPS;
    window=gtk_window_new(GTK_WINDOW_TOPLEVEL); gtk_window_set_decorated(GTK_WINDOW(window), FALSE);

    gtk_window_set_title(GTK_WINDOW(window), get_string(IDS_AAPSTOEPS));

    gtk_signal_connect(GTK_OBJECT(window), "delete-event",
                       GTK_SIGNAL_FUNC(modal_delete), &rc);
    
    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show(vbox);
   
    /* Create a label */
    label = gtk_label_new(get_string(IDS_AAPSTOEPSREAD));
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
    gtk_widget_show(label);

    /* Create checkbox */
    check = gtk_check_button_new_with_label(get_string(IDS_AAPSTOEPSAUTO));
    gtk_box_pack_start(GTK_BOX(vbox), check, FALSE, FALSE, 5);
    gtk_widget_show(check);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), option.auto_bbox);

    /* Create and place the buttons across the bottom */
    hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
    gtk_widget_show(hbox);

    button_yes = gtk_button_new_with_label(get_string(IDS_AAYES));
    gtk_box_pack_start(GTK_BOX(hbox), button_yes, TRUE, TRUE, 10);
    gtk_signal_connect(GTK_OBJECT(button_yes), "clicked",
			      GTK_SIGNAL_FUNC(modal_yes), &rc);
    gtk_widget_show(button_yes);

    button_no = gtk_button_new_with_label(get_string(IDS_AANO));
    gtk_box_pack_start(GTK_BOX(hbox), button_no, TRUE, TRUE, 10);
    gtk_signal_connect(GTK_OBJECT(button_no), "clicked",
			      GTK_SIGNAL_FUNC(modal_no), &rc);
    gtk_widget_show(button_no);

    button_help = gtk_button_new_with_label(get_string(IDS_AAHELP));
    gtk_box_pack_start(GTK_BOX(hbox), button_help, TRUE, TRUE, 10);
    gtk_signal_connect(GTK_OBJECT(button_help), "clicked",
			      GTK_SIGNAL_FUNC(modal_help), &rc);
    gtk_widget_show(button_help);

    /* show dialog and wait for Yes, No, or close */
    gtk_window_set_focus(GTK_WINDOW(window), button_no);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_show(window);
    gtk_main();

    if (rc == IDYES)
	option.auto_bbox =
	        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check));
 
    gtk_widget_destroy(window);

    return (rc == IDYES);
}

void change_sounds(void)
{
    gs_addmess("change_sounds: not implemented\n");
}

void setting_add(GtkWidget *table, const char *str1, GtkWidget *w, const char *str3, int *y)
{
    GtkWidget *label1 = gtk_label_new(str1);
    GtkWidget *label3 = gtk_label_new(str3);
    GtkWidget *con1, *con2, *con3;
    con1 = gtk_alignment_new(1, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(con1), label1);
    con2 = gtk_alignment_new(0, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(con2), w);
    con3 = gtk_alignment_new(0, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(con3), label3);
    gtk_widget_show(label1);
    gtk_widget_show(w);
    gtk_widget_show(label3);
    gtk_widget_show(con1);
    gtk_widget_show(con2);
    gtk_widget_show(con3);
    gtk_table_attach(GTK_TABLE(table), con1, 0, 1, *y, (*y)+1,
	GTK_FILL, GTK_FILL, 5, 0);
    gtk_table_attach(GTK_TABLE(table), con2, 1, 2, *y, (*y)+1,
	GTK_FILL, GTK_FILL, 5, 0);
    gtk_table_attach(GTK_TABLE(table), con3, 2, 3, *y, (*y)+1,
	GTK_FILL, GTK_FILL, 5, 0);
    (*y)++;
}

const char *depthlist[] =    {"Default", "8", "24"};
int index_to_depth[] = { 0, 8, 24};
int depth_to_index(int depth)
{
int i;
    for (i=0; i<(int)(sizeof(index_to_depth)/sizeof(int)); i++)
	if (index_to_depth[i] == depth)
	    return i;
    return 0;
}

const char *alphalist[] =    {"1", "2", "4"};
int index_to_alpha[] = { 1,   2,   4};
int alpha_to_index(int alpha)
{
int i;
    for (i=0; i<(int)(sizeof(index_to_alpha)/sizeof(int)); i++)
	if (index_to_alpha[i] == alpha)
	    return i;
    return 0;
}

const char *drawlist[] =    {"Pixmap", "Backing pixmap", "Backing storage"};
int index_to_draw[] = { IDM_DRAWPIXMAP, IDM_DRAWBACKING,  IDM_DRAWSTORAGE };
int draw_to_index(int draw)
{
unsigned int i;
    for (i=0; i<sizeof(index_to_draw)/sizeof(int); i++)
	if (index_to_draw[i] == draw)
	    return i;
    return 0;
}


void display_settings(void)
{
    GtkWidget *window;		/* main dialog window */
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *table;
    GtkWidget *button_ok, *button_cancel, *button_help;
    GtkWidget *wres, *wzres, *wdepth, *wtext, *wgraphics;
    GList *list, *l;
    char buf[MAXSTR];
    int i;
    int y = 0;
    int rc;
    const char *s;

    nHelpTopic = IDS_TOPICDSET;
    window=gtk_window_new(GTK_WINDOW_TOPLEVEL); gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_signal_connect(GTK_OBJECT(window), "delete-event",
                       GTK_SIGNAL_FUNC(modal_delete), &rc);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_window_set_title(GTK_WINDOW(window), get_string(IDS_AADISPLAYSETTINGS));
    
    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    table = gtk_table_new(6, 3, FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 5);

    if (option.xdpi == option.ydpi)
	sprintf(buf,"%g", option.xdpi);
    else 
	sprintf(buf,"%g %g", option.xdpi, option.ydpi);
    wres = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(wres), 32);
    gtk_entry_set_text(GTK_ENTRY(wres), buf);
    setting_add(table, get_string(IDS_AARESOLUTION), wres, get_string(IDS_AADPI), &y);
    gtk_signal_connect(GTK_OBJECT(wres), "activate",
                              GTK_SIGNAL_FUNC(modal_ok), &rc);

    if (option.zoom_xdpi == option.zoom_ydpi)
	sprintf(buf,"%g", option.zoom_xdpi);
    else 
	sprintf(buf,"%g %g", option.zoom_xdpi, option.zoom_ydpi);
    wzres = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(wzres), 32);
    gtk_entry_set_text(GTK_ENTRY(wzres), buf);
    setting_add(table, get_string(IDS_AAZOOMRESOLUTION), wzres, get_string(IDS_AADPI), &y);
    gtk_signal_connect(GTK_OBJECT(wres), "activate",
                              GTK_SIGNAL_FUNC(modal_ok), &rc);

    wdepth = gtk_combo_new();
    list = NULL;
    for (i=0; i<(int)(sizeof(depthlist)/sizeof(char *)); i++) {
	char *p;
	strcpy(buf, depthlist[i]);
	if (strcmp(buf, "Default")==0)
	    load_string(IDS_DEFAULT, buf, sizeof(buf)-1);
	p = (char *)malloc(strlen(buf)+1);
	strcpy(p, buf);
	list = g_list_append(list, p);
    }
    gtk_combo_set_popdown_strings(GTK_COMBO(wdepth), list);
    l = list;
    while (l) {
	free(l->data);
	l = g_list_next(l);
    }
    g_list_free(list);
    list = NULL;
    gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(wdepth)->entry), FALSE);
    setting_add(table, get_string(IDS_AADEPTH), wdepth, get_string(IDS_AABPP), &y);
    gtk_signal_connect(GTK_OBJECT(GTK_COMBO(wdepth)->entry), "activate",
                              GTK_SIGNAL_FUNC(modal_ok), &rc);
    
    s = depthlist[depth_to_index(option.depth)];
    if (strcmp(s, "Default")==0)
	s = get_string(IDS_DEFAULT);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(wdepth)->entry), s);

    wtext = gtk_combo_new();
    list = NULL;
    for (i=0; i<(int)(sizeof(alphalist)/sizeof(char *)); i++) {
	char *p;
	strcpy(buf, alphalist[i]);
	if (strcmp(buf, "Default")==0)
	    load_string(IDS_DEFAULT, buf, sizeof(buf)-1);
	p = (char *)malloc(strlen(buf)+1);
	strcpy(p, buf);
	list = g_list_append(list, p);
    }
    gtk_combo_set_popdown_strings(GTK_COMBO(wtext), list);
    l = list;
    while (l) {
	free(l->data);
	l = g_list_next(l);
    }
    g_list_free(list);
    list = NULL;
    gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(wtext)->entry), FALSE);

    s = alphalist[alpha_to_index(option.alpha_text)];
    if (strcmp(s, "Default")==0)
	s = get_string(IDS_DEFAULT);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(wtext)->entry), s);
    setting_add(table, get_string(IDS_AATEXTALPHA), wtext, get_string(IDS_AABITS), &y);
    gtk_signal_connect(GTK_OBJECT(GTK_COMBO(wtext)->entry), "activate",
                              GTK_SIGNAL_FUNC(modal_ok), &rc);

    wgraphics = gtk_combo_new();
    list = NULL;
    for (i=0; i<(int)(sizeof(alphalist)/sizeof(char *)); i++) {
	char *p;
	strcpy(buf, alphalist[i]);
	if (strcmp(buf, "Default")==0)
	    load_string(IDS_DEFAULT, buf, sizeof(buf)-1);
	p = (char *)malloc(strlen(buf)+1);
	strcpy(p, buf);
	list = g_list_append(list, p);
    }
    gtk_combo_set_popdown_strings(GTK_COMBO(wgraphics), list);
    l = list;
    while (l) {
	free(l->data);
	l = g_list_next(l);
    }
    g_list_free(list);
    list = NULL;
    gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(wgraphics)->entry), FALSE);
    s = alphalist[alpha_to_index(option.alpha_graphics)];
    if (strcmp(s, "Default")==0)
	s = get_string(IDS_DEFAULT);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(wgraphics)->entry), s);
    setting_add(table, get_string(IDS_AAGRAPHICSALPHA), wgraphics, get_string(IDS_AABITS), &y);
    gtk_signal_connect(GTK_OBJECT(GTK_COMBO(wgraphics)->entry), "activate",
                              GTK_SIGNAL_FUNC(modal_ok), &rc);


    /* buttons across bottom */
    hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
    button_ok = gtk_button_new_with_label(get_string(IDS_AAOK));
    button_cancel = gtk_button_new_with_label(get_string(IDS_AACANCEL));
    button_help = gtk_button_new_with_label(get_string(IDS_AAHELP));
    gtk_box_pack_start(GTK_BOX(hbox), button_ok, TRUE, TRUE, 20);
    gtk_box_pack_start(GTK_BOX(hbox), button_cancel, TRUE, TRUE, 20);
    gtk_box_pack_start(GTK_BOX(hbox), button_help, TRUE, TRUE, 20);

    /* Connect our callbacks to the three buttons */
    gtk_signal_connect(GTK_OBJECT(button_ok), "clicked",
                              GTK_SIGNAL_FUNC(modal_ok), &rc);
    gtk_signal_connect(GTK_OBJECT(button_cancel), "clicked",
                              GTK_SIGNAL_FUNC(modal_cancel), &rc);
    gtk_signal_connect(GTK_OBJECT(button_help), "clicked",
                              GTK_SIGNAL_FUNC(modal_help), &rc);

    gtk_widget_show(button_ok);
    gtk_widget_show(button_cancel);
    gtk_widget_show(button_help);
    gtk_widget_show(hbox);

    /* show dialog and wait for OK, Cancel or close */
    gtk_widget_show(vbox);
    gtk_widget_show(table);
    gtk_window_set_focus(GTK_WINDOW(window), wres);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_show(window);
    gtk_main();
 
    if (rc == IDOK) {
	BOOL unzoom = FALSE;
	BOOL resize = FALSE;
	BOOL restart = FALSE;
	float x, y;
	
	strncpy(buf, gtk_entry_get_text(GTK_ENTRY(wres)), 255);
	switch (sscanf(buf,"%f %f", &x, &y)) {
	  case EOF:
	  case 0:
	    break;
	  case 1:
	    y = x;
	  case 2:
	    if (x==0.0)
		x= DEFAULT_RESOLUTION;
	    if (y==0.0)
		y= DEFAULT_RESOLUTION;
	    if ( (x != option.xdpi) || (y != option.ydpi) ) {
		option.xdpi = x;
		option.ydpi = y;
		resize = TRUE; 
		unzoom = TRUE;
	    }
	}
	strncpy(buf, gtk_entry_get_text(GTK_ENTRY(wzres)), 255);
	switch (sscanf(buf,"%f %f", &x, &y)) {
	  case EOF:
	  case 0:
	    break;
	  case 1:
	    y = x;
	  case 2:
	    if (x==0.0)
		x= DEFAULT_RESOLUTION;
	    if (y==0.0)
		y= DEFAULT_RESOLUTION;
	    if ( (x != option.zoom_xdpi) || (y != option.zoom_ydpi) ) {
		option.zoom_xdpi = x;
		option.zoom_ydpi = y;
		resize = TRUE; 
		unzoom = TRUE;
	    }
	}
	
	strncpy(buf, gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(wdepth)->entry)),
	    sizeof(buf)-1);
	if (strcmp(buf, get_string(IDS_DEFAULT))==0)
	    strcpy(buf, "Default");
	for (i=0; i<(int)(sizeof(depthlist)/sizeof(char *)); i++) {
	    if (strcmp(buf, depthlist[i]) == 0)
		break;
	}
	if (i >= (int)(sizeof(depthlist)/sizeof(char *)))
	    i = 0;
	i = index_to_depth[i];
	if (i != option.depth) {
	    option.depth = i;
	    restart = TRUE;
	    resize = TRUE; 
	    unzoom = TRUE;
	}

	strncpy(buf, gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(wtext)->entry)),
	    sizeof(buf)-1);
	if (strcmp(buf, get_string(IDS_DEFAULT))==0)
	    strcpy(buf, "Default");
	for (i=0; i<(int)(sizeof(alphalist)/sizeof(char *)); i++) {
	    if (strcmp(buf, alphalist[i]) == 0)
		break;
	}
	i = index_to_alpha[i];
	if (i != option.alpha_text) {
	    option.alpha_text = i;
	    restart = TRUE;
	    resize = TRUE; 
	    unzoom = TRUE;
	}

	strncpy(buf, gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(wgraphics)->entry)),
	    sizeof(buf)-1);
	if (strcmp(buf, get_string(IDS_DEFAULT))==0)
	    strcpy(buf, "Default");
	for (i=0; i<(int)(sizeof(alphalist)/sizeof(char *)); i++) {
	    if (strcmp(buf, alphalist[i]) == 0)
		break;
	}
	i = index_to_alpha[i];
	if (i != option.alpha_graphics) {
	    option.alpha_graphics = i;
	    restart = TRUE;
	    resize = TRUE; 
	    unzoom = TRUE;
	}

	if (resize) {
	    if (unzoom)
		gsview_unzoom();
	    if (gsdll.state != GS_UNINIT) {
/* gs_resize has this check 
		if (option.redisplay && (gsdll.state == GS_PAGE) && (psfile.dsc != (CDSC *)NULL))
*/
		    gs_resize();
		/* for those that can't be changed with a */
		/* postscript command so must close gs */
		if (restart)
		    pending.restart = TRUE;
	    }
	}
    }

    gtk_widget_destroy(window);
}

gint
showmess_realize(GtkWidget *widget, gpointer user_data)
{
   (GTK_TEXT_VIEW(widget)->vadjustment)->value = (GTK_TEXT_VIEW(widget)->vadjustment)->upper
	- (GTK_TEXT_VIEW(widget)->vadjustment)->page_size;
   gtk_signal_emit_by_name(GTK_OBJECT(GTK_TEXT_VIEW(widget)->vadjustment), "changed");
   return FALSE; 
}


#define TWLENGTH 61440
#define TWSCROLL 1024
char twbuf[TWLENGTH];
int twend;

void gs_showmess(void)
{
    GtkWidget *window;		/* main dialog window */
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *text;
    GtkTextBuffer *textbuf;
    GtkTextIter textiter;
    GtkWidget *table;
    GtkWidget *vscrollbar;
    GtkWidget *button_ok, *button_help;
    int rc = 0;

    nHelpTopic = IDS_TOPICMESS;
    window=gtk_window_new(GTK_WINDOW_TOPLEVEL); gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_title(GTK_WINDOW(window), get_string(IDS_AAGSMESSX11));
    gtk_widget_set_usize(GTK_WIDGET(window), 450, 300);

    gtk_signal_connect(GTK_OBJECT(window), "delete-event",
                       GTK_SIGNAL_FUNC(modal_delete), &rc);
    
    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show(vbox);
    table = gtk_table_new(2, 2, FALSE);
    gtk_table_set_row_spacing(GTK_TABLE(table), 0, 2);
    gtk_table_set_col_spacing(GTK_TABLE(table), 0, 2);
    gtk_widget_show(table);
   
    /* Create text */
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_NEVER,   /* horizontal policy */
                                   GTK_POLICY_NEVER);  /* vertical policy */
    text = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER(scrolled_window), text);
    gtk_table_attach(GTK_TABLE(table), scrolled_window, 0, 1, 0, 1,
	(GtkAttachOptions)(GTK_EXPAND | GTK_SHRINK | GTK_FILL),
	(GtkAttachOptions)(GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_widget_show(text);
    gtk_widget_show(scrolled_window);

    /* Add a vertical scroll bar to the GtkText widget */
    GtkAdjustment *vadjustment =
        gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolled_window));
    vscrollbar = gtk_vscrollbar_new(vadjustment);
    gtk_table_attach(GTK_TABLE(table), vscrollbar, 1, 2, 0, 1,
	(GtkAttachOptions)(GTK_FILL),
	(GtkAttachOptions)(GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_widget_show(vscrollbar);

    textbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
    gtk_text_buffer_get_iter_at_offset(textbuf, &textiter, 0);
    gtk_text_buffer_insert(textbuf, &textiter, twbuf, twend);

    /* connect to realize handler so we can scroll to
     * the end when the window is created
     */
    gtk_signal_connect_object(GTK_OBJECT(window), "show",
                        GTK_SIGNAL_FUNC(showmess_realize), 
			GTK_OBJECT(text));

    gtk_box_pack_start(GTK_BOX(vbox), table, TRUE, TRUE, 5);

    /* Create and place the buttons across the bottom */
    hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
    gtk_widget_show(hbox);

    button_ok = gtk_button_new_with_label(get_string(IDS_AAOK));
    gtk_box_pack_start(GTK_BOX(hbox), button_ok, TRUE, FALSE, 10);
    gtk_signal_connect(GTK_OBJECT(button_ok), "clicked",
			      GTK_SIGNAL_FUNC(modal_ok), &rc);
    gtk_widget_show(button_ok);

    button_help = gtk_button_new_with_label(get_string(IDS_AAHELP));
    gtk_box_pack_start(GTK_BOX(hbox), button_help, TRUE, FALSE, 10);
    gtk_signal_connect(GTK_OBJECT(button_help), "clicked",
			      GTK_SIGNAL_FUNC(modal_help), &rc);
    gtk_widget_show(button_help);

    /* show dialog and wait for OK, close */
    gtk_window_set_focus(GTK_WINDOW(window), button_ok);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_show(window);
    gtk_main();

    gtk_widget_destroy(window);
}

HWND gs_showmess_modeless(void)
{
    gs_addmess("gs_showmess_modeless: not implemented\n");
    return NULL;
}

/* Add string for Ghostscript message window */
/* Unix version removes all "\r" */
void
gs_addmess_count(const char *str, int count)
{
const char *s;
char *p;
int i, cr_count;
    /* if debugging, write to stdout */
    if (debug & DEBUG_LOG)
        fwrite(str, 1, count ,stdout);

    /* Count the \r that we need to remove */
    cr_count = 0;
    s = str;
    for (i=0; i<count; i++) {
	if (*s == '\r')
	    cr_count++;
	s++;
    }
    if (count - cr_count >= TWSCROLL)
	return;		/* too large */
    if (count - cr_count + twend >= TWLENGTH-1) {
	/* scroll buffer */
	twend -= TWSCROLL;
	memmove(twbuf, twbuf+TWSCROLL, twend);
    }
    p = twbuf+twend;
    for (i=0; i<count; i++) {
	if (*str == '\0') {
	    *p++ = ' ';	/* ignore null characters */
	    str++;
	}
	else if (*str == '\r') {
	    /* ignore \r */
	}
	else
	    *p++ = *str++;
    }
    twend += (count - cr_count);
    *(twbuf+twend) = '\0';
}

void 
gs_addmess(const char *str)
{
    gs_addmess_count(str, strlen(str));
}

int get_dsc_response(char *message)
{
    GtkWidget *window;		/* main dialog window */
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *button_ok, *button_cancel;
    GtkWidget *button_ignore, *button_help;
    int rc;

    nHelpTopic = IDS_TOPICDSCWARN;
    window=gtk_window_new(GTK_WINDOW_TOPLEVEL); gtk_window_set_decorated(GTK_WINDOW(window), FALSE);

    gtk_window_set_title(GTK_WINDOW(window), get_string(IDS_AADSC));

    gtk_signal_connect(GTK_OBJECT(window), "delete-event",
                       GTK_SIGNAL_FUNC(modal_delete), &rc);
    
    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show(vbox);
   
    /* Create a label */
    label = gtk_label_new(message);
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
    gtk_widget_show(label);

    /* Create and place the buttons across the bottom */
    hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
    gtk_widget_show(hbox);

    button_ok = gtk_button_new_with_label(get_string(IDS_AAOK));
    gtk_box_pack_start(GTK_BOX(hbox), button_ok, TRUE, TRUE, 10);
    gtk_signal_connect(GTK_OBJECT(button_ok), "clicked",
			      GTK_SIGNAL_FUNC(modal_ok), &rc);
    gtk_widget_show(button_ok);

    button_cancel = gtk_button_new_with_label(get_string(IDS_AACANCEL));
    gtk_box_pack_start(GTK_BOX(hbox), button_cancel, TRUE, TRUE, 10);
    gtk_signal_connect(GTK_OBJECT(button_cancel), "clicked",
			      GTK_SIGNAL_FUNC(modal_cancel), &rc);
    gtk_widget_show(button_cancel);

    button_ignore = gtk_button_new_with_label(get_string(IDS_AAIGNOREALLDSC));
    gtk_box_pack_start(GTK_BOX(hbox), button_ignore, TRUE, TRUE, 10);
    gtk_signal_connect(GTK_OBJECT(button_ignore), "clicked",
			      GTK_SIGNAL_FUNC(modal_ignore), &rc);
    gtk_widget_show(button_ignore);

    button_help = gtk_button_new_with_label(get_string(IDS_AAHELP));
    gtk_box_pack_start(GTK_BOX(hbox), button_help, TRUE, TRUE, 10);
    gtk_signal_connect(GTK_OBJECT(button_help), "clicked",
			      GTK_SIGNAL_FUNC(modal_help), &rc);
    gtk_widget_show(button_help);

    /* show dialog and wait for OK, Cancel, Ignore or close */
    gtk_window_set_focus(GTK_WINDOW(window), button_ok);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_show(window);
    gtk_main();

    gtk_widget_destroy(window);

    if (rc == IDOK) {
	rc = CDSC_RESPONSE_OK;
    }
    else if (rc == DSC_IGNORE_ALL) {
	rc = CDSC_RESPONSE_IGNORE_ALL;
    }
    else {
	rc = CDSC_RESPONSE_CANCEL;
    }
    return rc;
}

typedef struct {
    GtkWidget *gsver;
    GtkWidget *gsdll;
    GtkWidget *gsexe;
    GtkWidget *gslib;
    GtkWidget *gsopt;
} CFGENTRY;

void cfg_defaults(GtkWidget *w, gpointer data)
{
    CFGENTRY *entry = (CFGENTRY *)data;
    char buf[MAXSTR];
    sprintf(buf, "%d", GS_REVISION);
    gtk_entry_set_text(GTK_ENTRY(entry->gsver), buf);
    default_gsdll(buf);
    gtk_entry_set_text(GTK_ENTRY(entry->gsdll), buf);
    default_gsexe(buf);
    gtk_entry_set_text(GTK_ENTRY(entry->gsexe), buf);
    gtk_entry_set_text(GTK_ENTRY(entry->gslib), "");
    gtk_entry_set_text(GTK_ENTRY(entry->gsopt), "");
}

/* configure Ghostscript */
BOOL
install_gsdll(void)
{
    GtkWidget *window;		/* main dialog window */
    GtkWidget *vbox;
    GtkWidget *hbox;		/* for buttons */
    GtkWidget *gsver_label;
    GtkWidget *gsdll_label;
    GtkWidget *gsexe_label;
    GtkWidget *gslib_label;
    GtkWidget *gsopt_label;
    GtkWidget *printerdef_checkbox;
    GtkWidget *con;
    GtkWidget *button_ok, *button_cancel, *button_help;
    GtkWidget *button_defaults;
    CFGENTRY entry;
    char buf[MAXSTR];
    int rc = 0;

    nHelpTopic = IDS_TOPICADVANCEDCFG;
    window=gtk_window_new(GTK_WINDOW_TOPLEVEL); gtk_window_set_decorated(GTK_WINDOW(window), FALSE);

    gtk_window_set_title(GTK_WINDOW(window), get_string(IDS_AACONFIGUREGS));

    gtk_signal_connect(GTK_OBJECT(window), "delete-event",
                       GTK_SIGNAL_FUNC(modal_delete), &rc);
    
    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show(vbox);
   
    /* Ghostscript Version */
    gsver_label = gtk_label_new("Ghostscript Version");
    gtk_widget_show(gsver_label);
    con = gtk_alignment_new(0, 0, 0, 0);
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), gsver_label);
    gtk_box_pack_start(GTK_BOX(vbox), con, FALSE, FALSE, 0);
    entry.gsver = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry.gsver), 255);
    sprintf(buf, "%d", option.gsversion);
    gtk_entry_set_text(GTK_ENTRY(entry.gsver), buf);
    gtk_box_pack_start(GTK_BOX(vbox), entry.gsver, FALSE, FALSE, 0);
    gtk_widget_show(entry.gsver);
    gtk_signal_connect(GTK_OBJECT(entry.gsver), "activate",
                              GTK_SIGNAL_FUNC(modal_ok), &rc);

    /* show dialog and wait for OK, Cancel or close */

    /* Ghostscript EXE */
    gsdll_label = gtk_label_new(get_string(IDS_AAGHOSTSCRIPTSOC));
    gtk_widget_show(gsdll_label);
    con = gtk_alignment_new(0, 0, 0, 0);
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), gsdll_label);
    gtk_box_pack_start(GTK_BOX(vbox), con, FALSE, FALSE, 0);
    entry.gsdll = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry.gsdll), 255);
    gtk_entry_set_text(GTK_ENTRY(entry.gsdll), option.gsdll);
    gtk_box_pack_start(GTK_BOX(vbox), entry.gsdll, FALSE, FALSE, 0);
    gtk_widget_show(entry.gsdll);
    gtk_signal_connect(GTK_OBJECT(entry.gsdll), "activate",
                              GTK_SIGNAL_FUNC(modal_ok), &rc);
    /* Ghostscript EXE */
    gsexe_label = gtk_label_new(get_string(IDS_AAGHOSTSCRIPTEXEC));
    gtk_widget_show(gsexe_label);
    con = gtk_alignment_new(0, 0, 0, 0);
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), gsexe_label);
    gtk_box_pack_start(GTK_BOX(vbox), con, FALSE, FALSE, 0);
    entry.gsexe = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry.gsexe), 255);
    gtk_entry_set_text(GTK_ENTRY(entry.gsexe), option.gsexe);
    gtk_box_pack_start(GTK_BOX(vbox), entry.gsexe, FALSE, FALSE, 0);
    gtk_widget_show(entry.gsexe);
    gtk_signal_connect(GTK_OBJECT(entry.gsexe), "activate",
                              GTK_SIGNAL_FUNC(modal_ok), &rc);

    /* Ghostscript Include Path */
    gslib_label = gtk_label_new(get_string(IDS_AAGHOSTSCRIPTINCC));
    gtk_widget_show(gslib_label);
    con = gtk_alignment_new(0, 0, 0, 0);
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), gslib_label);
    gtk_box_pack_start(GTK_BOX(vbox), con, FALSE, FALSE, 0);
    entry.gslib = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry.gslib), 255);
    gtk_entry_set_text(GTK_ENTRY(entry.gslib), option.gsinclude);
    gtk_box_pack_start(GTK_BOX(vbox), entry.gslib, FALSE, FALSE, 0);
    gtk_widget_show(entry.gslib);
    gtk_signal_connect(GTK_OBJECT(entry.gslib), "activate",
                              GTK_SIGNAL_FUNC(modal_ok), &rc);

    /* Ghostscript Options */
    gsopt_label = gtk_label_new(get_string(IDS_AAGHOSTSCRIPTOTHERC));
    gtk_widget_show(gsopt_label);
    con = gtk_alignment_new(0, 0, 0, 0);
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), gsopt_label);
    gtk_box_pack_start(GTK_BOX(vbox), con, FALSE, FALSE, 0);
    entry.gsopt = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry.gsopt), 255);
    gtk_entry_set_text(GTK_ENTRY(entry.gsopt), option.gsother);
    gtk_box_pack_start(GTK_BOX(vbox), entry.gsopt, FALSE, FALSE, 0);
    gtk_widget_show(entry.gsopt);
    gtk_signal_connect(GTK_OBJECT(entry.gsopt), "activate",
                              GTK_SIGNAL_FUNC(modal_ok), &rc);

    printerdef_checkbox = gtk_check_button_new_with_label(get_string(IDS_AACOPYPRINTERDEF));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(printerdef_checkbox), 
		FALSE);
    gtk_widget_show(printerdef_checkbox);
    gtk_box_pack_start(GTK_BOX(vbox), printerdef_checkbox, FALSE, FALSE, 0);

    /* Create and place the buttons across the bottom */
    button_ok = gtk_button_new_with_label(get_string(IDS_AAOK));
    button_cancel = gtk_button_new_with_label(get_string(IDS_AACANCEL));
    button_help = gtk_button_new_with_label(get_string(IDS_AAHELP));
    button_defaults = gtk_button_new_with_label(get_string(IDS_AADEFAULTS));

    hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
    gtk_widget_show(hbox);

    gtk_box_pack_start(GTK_BOX(hbox), button_ok, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), button_cancel, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), button_help, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), button_defaults, TRUE, TRUE, 5);

    /* Connect our callbacks to the buttons */
    gtk_signal_connect(GTK_OBJECT(button_ok), "clicked",
                              GTK_SIGNAL_FUNC(modal_ok), &rc);
    gtk_signal_connect(GTK_OBJECT(button_cancel), "clicked",
                              GTK_SIGNAL_FUNC(modal_cancel), &rc);
    gtk_signal_connect(GTK_OBJECT(button_help), "clicked",
                              GTK_SIGNAL_FUNC(modal_help), &rc);

    gtk_signal_connect(GTK_OBJECT(button_defaults), "clicked",
		      GTK_SIGNAL_FUNC(cfg_defaults), &entry);

    gtk_widget_show(button_ok);
    gtk_widget_show(button_cancel);
    gtk_widget_show(button_help);
    gtk_widget_show(button_defaults);

    /* show dialog and wait for OK, Cancel or close */
    gtk_window_set_focus(GTK_WINDOW(window), entry.gsver);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_show(window);
    gtk_main();
 
    if (rc == IDOK) {
	option.gsversion = atoi(gtk_entry_get_text(GTK_ENTRY(entry.gsver)));
	if (option.gsversion < 510)
	    option.gsversion = 510;
	strncpy(option.gsdll, gtk_entry_get_text(GTK_ENTRY(entry.gsdll)),
	    sizeof(option.gsdll));
	strncpy(option.gsexe, gtk_entry_get_text(GTK_ENTRY(entry.gsexe)),
	    sizeof(option.gsexe));
	strncpy(option.gsinclude, gtk_entry_get_text(GTK_ENTRY(entry.gslib)),
	    sizeof(option.gsinclude));
	strncpy(option.gsother, gtk_entry_get_text(GTK_ENTRY(entry.gsopt)),
	    sizeof(option.gsother));
	if (gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(printerdef_checkbox)))
	    gsview_printer_profiles();
    }

    gtk_widget_destroy(window);

    return (rc == IDOK);
}
