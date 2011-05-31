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

/* gvxprn.c */
#include "gvx.h"
#include "en/gvclang.h"

char not_defined[] = "[Not defined]";

int print_gdi_width;
int print_gdi_height;
int print_gdi_xdpi;
int print_gdi_ydpi;
ULONG print_gdi_read_handle;
ULONG print_gdi_write_handle;

typedef struct {
    char device[MAXSTR];
    GtkWidget *prop;
    GtkWidget *value;
    GtkWidget *x_offset;
    GtkWidget *y_offset;
    struct prop_item_s *propitem;
    BOOL new_property;	/* for edit_prop_dialog */
    /* ignore_select_value is used to ignore repeat selection-change events */
    BOOL ignore_select_value;
} PROPVALUES;

void options_update(GtkWidget *w, gpointer data);
void resolution_update(GtkWidget *w, gpointer data);
void select_value(GtkWidget *w, gpointer data);
void update_properties(PROPVALUES *pv);
void update_values(GtkWidget *w, gpointer data);
void edit_prop_delete(GtkWidget *w, gpointer data);
void strip_spaces(char *s);
void edit_prop_dialog(GtkWidget *w, gpointer data);
void new_prop_dialog(GtkWidget *w, gpointer data);
void prop_dialog(GtkWidget *w, gpointer data);
void advanced_ps_dialog(GtkWidget *w, gpointer data);
int print_dialog_box(BOOL convert);
gint delete_print_message(GtkWidget *widget, GdkEvent event, gpointer data);
void percent_print_message(int percent);
void add_print_message(char *str, int len) ;
void close_print_message(void);
void show_print_message(void);

/* port==NULL means prompt for port or queue with dialog box */
int gp_printfile(char *filename, char *port)
{
    int rc;
    char command[MAXSTR+MAXSTR];
    char new_port[MAXSTR];
    if ((port==NULL) || (port[0] == '\0')) {
	strcpy(new_port, option.printer_queue);
	if (!query_string("Printer", new_port))
	    return FALSE;
	port = new_port;
    }
    strcpy(command, port);
    strcat(command, " ");
    strcat(command, filename);
    if (debug & DEBUG_GENERAL)
	gs_addmessf("Printing file \042%s\042 to printer \042%s\042/n", 
	    filename, port);
    rc = system(command);
    if ((rc == -1) || (rc == 127))
	gs_addmessf("gp_printfile: failed rc=%d\n", rc);
    else  {
	if (debug & DEBUG_GENERAL)
	    gs_addmessf("gp_printfile: rc=%d\n", rc);
    }
    return TRUE;
}

typedef struct {
    GtkWidget *resolution;
    GtkWidget *device;
    GtkWidget *options;
    BOOL bConvert;
} DEVRES;
    
void options_update(GtkWidget *w, gpointer data)
{
    DEVRES *devres = (DEVRES *)data;
    PROFILE *prf;
    char section[MAXSTR];
    char buf[MAXSTR];
    char *dev = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(devres->device)->entry));
    if ( (prf = profile_open(szIniFile)) != (PROFILE *)NULL ) {
	/* update printer options */
	strcpy(section, dev);
	strcat(section, " Options");
	profile_read_string(prf, section, "Options", "", buf, sizeof(buf)-2);
	gtk_entry_set_text(GTK_ENTRY(devres->options), buf);
	profile_close(prf);
    }
}

void resolution_update(GtkWidget *w, gpointer data)
{
    DEVRES *devres = (DEVRES *)data;
    GList *list = NULL;
    PROFILE *prf;
    char buf[MAXSTR];
    char *p, *s;
    char *dev = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(devres->device)->entry));

    if ( (prf = profile_open(szIniFile)) != (PROFILE *)NULL ) {
	/* look up resolutions */
	profile_read_string(prf, 
	    devres->bConvert ? CONVERTSECTION : DEVSECTION, 
	    dev, "", buf, sizeof(buf)-2);
	profile_close(prf);
    }
    else
	buf[0] = '\0';
    while ((*buf) && (buf[strlen(buf)-1]==' '))
	buf[strlen(buf)-1] = '\0';    /* remove trailing spaces */
    buf[strlen(buf)+1] = '\0';	/* double NULL at end */

    p = buf;
    while (p && *p!='\0') {
	s = p;
	while ((*p!='\0') && (*p!=','))
	    p++;
	*p++ = '\0';
	list = g_list_append(list, s);
    }
    if (list != NULL) {
	gtk_combo_set_popdown_strings(GTK_COMBO(devres->resolution), list);
	g_list_free(list);
	list = NULL;
    }
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(devres->resolution)->entry), buf);
}

void
select_value(GtkWidget *w, gpointer data)
{
    PROPVALUES *pv = (PROPVALUES *)data;
    char *p;
    char *v;
    int iprop;
    if (pv->ignore_select_value)
	return;
    p = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(pv->prop)->entry));
    v = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(pv->value)->entry));
    pv->ignore_select_value = TRUE;
    for (iprop=0; pv->propitem[iprop].name[0]; iprop++)
	if (strcmp(p, pv->propitem[iprop].name+1) == 0)
	    break;
    if (pv->propitem[iprop].name[0] == '\0')
	return;		/* can't find name */
    strncpy(pv->propitem[iprop].value, v, sizeof(pv->propitem[iprop].value));
    pv->ignore_select_value = FALSE;
}


void
update_properties(PROPVALUES *pv)
{
    GList *list = NULL;
    int iprop;
    if (pv->propitem) {
	free((char *)pv->propitem);
	pv->propitem = NULL;
    }
    pv->propitem = get_properties(pv->device);
    if ((pv->propitem != NULL) && (pv->propitem[0].name[0] != '\0')) {
	for (iprop=0; pv->propitem[iprop].name[0]; iprop++)
	    list = g_list_append(list, (pv->propitem[iprop].name)+1);
	gtk_combo_set_popdown_strings(GTK_COMBO(pv->prop), list);
	g_list_free(list);
	list = NULL;
	if (iprop)
	    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(pv->prop)->entry), 
		    (pv->propitem[0].name)+1);
    }
}

void
update_values(GtkWidget *w, gpointer data)
{
    PROPVALUES *pv = (PROPVALUES *)data;
    GList *list = NULL;
    char buf[MAXSTR];
    char *p, *value;
    char notdef[128];
    PROFILE *prf;
    char section[MAXSTR];
    int iprop;

    load_string(IDS_NOTDEFTAG, notdef, sizeof(notdef));
    pv->ignore_select_value = TRUE;
    /* now look up entry in INI file and update combo box */
    strcpy(section, pv->device);
    strcat(section, " values");
    p = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(pv->prop)->entry));
    for (iprop=0; pv->propitem[iprop].name[0]; iprop++)
	if (strcmp(p, pv->propitem[iprop].name+1) == 0)
	    break;
    if (pv->propitem[iprop].name[0] == '\0')
	return;		/* can't find name */
    if ( (prf = profile_open(szIniFile)) != (PROFILE *)NULL ) {
	profile_read_string(prf, section, pv->propitem[iprop].name, "", 
		buf, sizeof(buf)-2);
	profile_close(prf);
    }
    else
	buf[0] = '\0';
    while ((*buf) && (buf[strlen(buf)-1]==' '))
	buf[strlen(buf)-1] = '\0';    /* remove trailing spaces */
    buf[strlen(buf)+1] = '\0';	/* put double NULL at end */
    p = buf;
    list = g_list_append(list, notdef);
    if (*p != '\0') {
      while (*p!='\0') {
	value = p;
	while ((*p!='\0') && (*p!=','))
	    p++;
	*p++ = '\0';
	list = g_list_append(list, value);
      }
    }
    gtk_combo_set_popdown_strings(GTK_COMBO(pv->value), list);
    g_list_free(list);
    list = NULL;

    strcpy(buf, pv->propitem[iprop].value);

    if (strcmp(buf, not_defined)==0)
	strcpy(buf, notdef);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(pv->value)->entry), buf);
    pv->ignore_select_value = FALSE;
}

void
edit_prop_delete(GtkWidget *w, gpointer data)
{
    *((int *)data) = EDITPROP_DEL;
    gtk_main_quit();
}

void
strip_spaces(char *s)
{
char *d = s;
   while (*s) {
	if (*s != ' ')
	    *d++ = *s;
	s++;
   }
   *d = '\0';
}

void
edit_prop_dialog(GtkWidget *w, gpointer data)
{
    PROPVALUES *pv = (PROPVALUES *)data;

    GtkWidget *window;		/* main dialog window */
    GtkWidget *hbox;		/* for margins */
    GtkWidget *hbox_type;	/* hbox for number/string type */
    GtkWidget *hbox_buttons;	/* hbox for buttons */
    GtkWidget *vbox;		/* vbox for controls */
    GtkWidget *prop_entry;
    GtkWidget *prop_label;
    GtkWidget *value_entry;
    GtkWidget *value_label;
    GtkWidget *con;	/* container for alignment */
    GtkWidget *button_number, *button_string;
    char name[MAXSTR];
    char value[MAXSTR];
    char section[MAXSTR];

    GtkWidget *button_ok, *button_cancel, *button_help;
    GtkWidget *button_delete;
    PROFILE *prf;
    char editpropname[MAXSTR];
    int rc = 0;
    if (pv->new_property) {
	editpropname[0]='\0';
	editpropname[1]='\0';
	pv->new_property = FALSE;
    }
    else {
	int iprop;
	char *p = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(pv->prop)->entry));
	for (iprop=0; pv->propitem[iprop].name[0]; iprop++)
	    if (strcmp(p, pv->propitem[iprop].name+1) == 0)
		break;
	if (pv->propitem[iprop].name[0] == '\0') {
	    editpropname[0]='\0';
	    editpropname[1]='\0';
	}
	else
	    strncpy(editpropname, pv->propitem[iprop].name, 
		sizeof(editpropname)-1);
    }

    window=gtk_window_new(GTK_WINDOW_DIALOG);

    gtk_window_set_title(GTK_WINDOW(window), get_string(IDS_AAEDITPROPERTIES));

    gtk_signal_connect(GTK_OBJECT(window),
                       "delete-event",
                       GTK_SIGNAL_FUNC(modal_delete),
                       &rc);
    
    hbox = gtk_hbox_new(FALSE, 4);
    gtk_widget_show(hbox);
    gtk_container_add(GTK_CONTAINER(window), hbox);

    vbox = gtk_vbox_new(FALSE, 2);
    gtk_widget_show(vbox);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 4);

    /* Type */
    prop_label = gtk_label_new(get_string(IDS_AAPROPERTYC));
    gtk_widget_show(prop_label);
    con = gtk_alignment_new(0, 0, 0, 0);	/* left alignment */
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), prop_label);
    gtk_box_pack_start(GTK_BOX(vbox), con, FALSE, FALSE, 0);

    hbox_type = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_type, FALSE, FALSE, 0);
    gtk_widget_show(hbox_type);

    button_number = gtk_radio_button_new_with_label(NULL, get_string(IDS_AANUMBER));
    gtk_widget_show(button_number);
    gtk_box_pack_start(GTK_BOX(hbox_type), button_number, TRUE, TRUE, 4);

    button_string = gtk_radio_button_new_with_label(
	gtk_radio_button_group(GTK_RADIO_BUTTON(button_number)),
	get_string(IDS_AASTRING));
    gtk_widget_show(button_string);
    gtk_box_pack_start(GTK_BOX(hbox_type), button_string, TRUE, TRUE, 4);

    if (*editpropname == 's')
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_string), TRUE);
    else
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_number), TRUE);

    /* Name */
    prop_label = gtk_label_new(get_string(IDS_AANAMEC));
    gtk_widget_show(prop_label);
    con = gtk_alignment_new(0, 0, 0, 0);	/* left alignment */
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), prop_label);
    gtk_box_pack_start(GTK_BOX(vbox), con, FALSE, FALSE, 0);
    prop_entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(prop_entry), MAXSTR-1);
    gtk_box_pack_start(GTK_BOX(vbox), prop_entry, FALSE, FALSE, 0);
    gtk_widget_show(prop_entry);
    gtk_entry_set_text(GTK_ENTRY(prop_entry), editpropname+1);

    /* Value */
    value_label = gtk_label_new(get_string(IDS_AAVALUESC));
    gtk_widget_show(value_label);
    con = gtk_alignment_new(0, 0, 0, 0);	/* left alignment */
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), value_label);
    gtk_box_pack_start(GTK_BOX(vbox), con, FALSE, FALSE, 0);
    value_entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(value_entry), MAXSTR-1);
    gtk_box_pack_start(GTK_BOX(vbox), value_entry, FALSE, FALSE, 0);
    gtk_widget_show(value_entry);

    if (*editpropname) {
	char section[MAXSTR];
	char buf[MAXSTR];
	strcpy(section, pv->device);
	strcat(section, " values");
	if ( (prf = profile_open(szIniFile)) != (PROFILE *)NULL ) {
	    profile_read_string(prf, section, editpropname, "", 
		    buf, sizeof(buf)-2);
	    profile_close(prf);
	    prf = NULL;
	}
	gtk_entry_set_text(GTK_ENTRY(value_entry), buf);
    }

    /* Buttons */
    /* Create and place the buttons in a vbox at the right */
    hbox_buttons = gtk_hbox_new(TRUE, 4);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_buttons, FALSE, FALSE, 8);
    gtk_widget_show(hbox_buttons);

    button_ok = gtk_button_new_with_label(get_string(IDS_AAOK));
    button_cancel = gtk_button_new_with_label(get_string(IDS_AACANCEL));
    button_help = gtk_button_new_with_label(get_string(IDS_AAHELP));
    button_delete = gtk_button_new_with_label(get_string(IDS_AADELETE));

    gtk_box_pack_start(GTK_BOX(hbox_buttons), button_ok, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(hbox_buttons), button_cancel, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(hbox_buttons), button_help, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(hbox_buttons), button_delete, TRUE, TRUE, 5);

    /* Connect our callbacks to the buttons */
    gtk_signal_connect(GTK_OBJECT(button_ok), "clicked",
                              GTK_SIGNAL_FUNC(modal_ok), &rc);
    gtk_signal_connect(GTK_OBJECT(button_cancel), "clicked",
                              GTK_SIGNAL_FUNC(modal_cancel), &rc);
    gtk_signal_connect(GTK_OBJECT(button_help), "clicked",
                              GTK_SIGNAL_FUNC(modal_help), &rc);
    gtk_signal_connect(GTK_OBJECT(button_delete), "clicked",
                              GTK_SIGNAL_FUNC(edit_prop_delete), &rc);

    gtk_widget_show(button_ok);
    gtk_widget_show(button_cancel);
    gtk_widget_show(button_help);
    gtk_widget_show(button_delete);

    /* show dialog and wait for OK, Cancel or close */
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_show(window);
    gtk_main();

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button_string)))
	strcpy(name, "s");
    else
	strcpy(name, "d");
    strncpy(name+1, gtk_entry_get_text(GTK_ENTRY(prop_entry)), sizeof(name)-2);
    strncpy(value, gtk_entry_get_text(GTK_ENTRY(value_entry)), sizeof(name)-1);
    strip_spaces(name);
    strip_spaces(value);
    strcpy(section, pv->device);
    strcat(section, " values");

    if (rc == IDOK) {
	/* save new setting */
	if ( (prf = profile_open(szIniFile)) != (PROFILE *)NULL ) {
	    if ((strlen(name)>1) && strlen(value)) {
		profile_write_string(prf, section, name, value);
		strtok(value, ",");
		profile_write_string(prf, pv->device, name, value);
	    }
	    profile_close(prf);
	}
    }
    else if (rc == EDITPROP_DEL) {
	/* delete entry */
	if ( (prf = profile_open(szIniFile)) != (PROFILE *)NULL ) {
	    if (strlen(name)>1) {
		profile_write_string(prf, section, name, NULL);
		profile_write_string(prf, pv->device, name, NULL);
	    }
	    profile_close(prf);
	}
    }

    gtk_widget_destroy(window);
    update_properties(pv);
    update_values(w, data);
}


void
new_prop_dialog(GtkWidget *w, gpointer data)
{
    PROPVALUES *pv = (PROPVALUES *)data;
    pv->new_property = TRUE;
    edit_prop_dialog(w, data);
}

void prop_dialog(GtkWidget *w, gpointer data)
{
    DEVRES *devres = (DEVRES *)data;

    GtkWidget *window;	/* main dialog window */
    GtkWidget *hbox;	/* hbox for controls on left, buttons on right */
    GtkWidget *vbox;	/* vbox on right for buttons */
    GtkWidget *vbox_left;
    GtkWidget *prop;
    GtkWidget *prop_label;
    GtkWidget *value;
    GtkWidget *value_label;
    GtkWidget *con;	/* container for alignment */
    GtkWidget *hbox_offset;
    GtkWidget *offset_label;
    GtkWidget *x_label;
    GtkWidget *y_label;
    GtkWidget *x_offset;
    GtkWidget *y_offset;
    GtkWidget *button_ok, *button_cancel, *button_help;
    GtkWidget *button_edit, *button_new;
    guint prop_signal;
    guint value_signal;
    PROFILE *prf;
    PROPVALUES pv;
    int rc = 0;
    int iprop;

    memset(&pv, 0, sizeof(PROPVALUES));
    strncpy(pv.device, 
	gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(devres->device)->entry)),
	sizeof(pv.device)-1);

    window=gtk_window_new(GTK_WINDOW_DIALOG);

    gtk_window_set_title(GTK_WINDOW(window), get_string(IDS_AAPROPERTIES));

    gtk_signal_connect(GTK_OBJECT(window),
                       "delete-event",
                       GTK_SIGNAL_FUNC(modal_delete),
                       &rc);
    
    hbox = gtk_hbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_container_add(GTK_CONTAINER(window), hbox);
    gtk_widget_show(hbox);

    vbox_left = gtk_vbox_new(FALSE, 2);
    gtk_widget_show(vbox_left);
    gtk_box_pack_start(GTK_BOX(hbox), vbox_left, TRUE, TRUE, 4);

    /* properties */
    prop_label = gtk_label_new(get_string(IDS_AAPROPERTYC));
    gtk_widget_show(prop_label);
    con = gtk_alignment_new(0, 0, 0, 0);	/* left alignment */
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), prop_label);
    gtk_box_pack_start(GTK_BOX(vbox_left), con, FALSE, FALSE, 0);
    prop = gtk_combo_new();
    gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(prop)->entry), FALSE);
    gtk_widget_show(prop);
    gtk_box_pack_start(GTK_BOX(vbox_left), prop, FALSE, FALSE, 0);
    pv.prop = prop;

    /* values */
    value_label = gtk_label_new(get_string(IDS_AAVALUEC));
    gtk_widget_show(value_label);
    con = gtk_alignment_new(0, 0, 0, 0);	/* left alignment */
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), value_label);
    gtk_box_pack_start(GTK_BOX(vbox_left), con, FALSE, FALSE, 0);
    value = gtk_combo_new();
    gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(value)->entry), FALSE);
    gtk_widget_show(value);
    gtk_box_pack_start(GTK_BOX(vbox_left), value, FALSE, FALSE, 0);
    pv.value = value;

    /* page offset offset */
    hbox_offset = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox_offset);
    gtk_box_pack_start(GTK_BOX(vbox_left), hbox_offset, FALSE, FALSE, 4);

    offset_label = gtk_label_new(get_string(IDS_AAPAGEOFFSETPT));
    gtk_widget_show(offset_label);
    con = gtk_alignment_new(0, 0, 0, 0);	/* left alignment */
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), offset_label);
    gtk_box_pack_start(GTK_BOX(hbox_offset), con, FALSE, FALSE, 0);

    x_label = gtk_label_new(get_string(IDS_AAXC));
    gtk_widget_show(x_label);
    con = gtk_alignment_new(1, 0, 0, 0);	/* right alignment */
    gtk_container_add(GTK_CONTAINER(con), x_label);
    gtk_widget_show(con);
    gtk_box_pack_start(GTK_BOX(hbox_offset), con, FALSE, FALSE, 8);

    x_offset = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(x_offset), 32);
    gtk_widget_show(x_offset);
    con = gtk_alignment_new(0, 0, 0, 0);	/* left alignment */
    gtk_container_add(GTK_CONTAINER(con), x_offset);
    gtk_widget_show(con);
    gtk_box_pack_start(GTK_BOX(hbox_offset), con, FALSE, FALSE, 8);
    pv.x_offset = x_offset;
    gtk_signal_connect(GTK_OBJECT(x_offset), "size-request",
			  GTK_SIGNAL_FUNC(width_percent), (gpointer)50);


    y_label = gtk_label_new(get_string(IDS_AAYC));
    gtk_widget_show(y_label);
    con = gtk_alignment_new(1, 0, 0, 0);	/* right alignment */
    gtk_container_add(GTK_CONTAINER(con), y_label);
    gtk_widget_show(con);
    gtk_box_pack_start(GTK_BOX(hbox_offset), con, FALSE, FALSE, 8);

    y_offset = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(y_offset), 32);
    gtk_widget_show(y_offset);
    con = gtk_alignment_new(0, 0, 0, 0);	/* left alignment */
    gtk_container_add(GTK_CONTAINER(con), y_offset);
    gtk_widget_show(con);
    gtk_box_pack_start(GTK_BOX(hbox_offset), con, FALSE, FALSE, 8);
    pv.y_offset = y_offset;
    gtk_signal_connect(GTK_OBJECT(y_offset), "size-request",
			  GTK_SIGNAL_FUNC(width_percent), (gpointer)50);

    if ( (prf = profile_open(szIniFile)) != (PROFILE *)NULL ) {
	char section[MAXSTR];
	char buf[MAXSTR];
	strcpy(section, pv.device);
	strcat(section, " Options");
	profile_read_string(prf, section, "Xoffset", "0", buf, sizeof(buf)-2);
	gtk_entry_set_text(GTK_ENTRY(x_offset), buf);
	profile_read_string(prf, section, "Yoffset", "0", buf, sizeof(buf)-2);
	gtk_entry_set_text(GTK_ENTRY(y_offset), buf);
	profile_close(prf);
    }

    /* update values when property name changed */
    update_properties(&pv);
    prop_signal = gtk_signal_connect(GTK_OBJECT(GTK_COMBO(prop)->list), 
	"selection-changed", GTK_SIGNAL_FUNC(update_values), &pv);
    update_values(GTK_COMBO(prop)->list, &pv);

    /* remember selected values */
    value_signal = gtk_signal_connect(GTK_OBJECT(GTK_COMBO(value)->list), 
	"selection-changed", GTK_SIGNAL_FUNC(select_value), &pv);

    /* Create and place the buttons in a vbox at the right */
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);
    gtk_widget_show(vbox);

    button_ok = gtk_button_new_with_label(get_string(IDS_AAOK));
    button_cancel = gtk_button_new_with_label(get_string(IDS_AACANCEL));
    button_help = gtk_button_new_with_label(get_string(IDS_AAHELP));
    button_edit = gtk_button_new_with_label(get_string(IDS_AAEDIT));
    button_new = gtk_button_new_with_label(get_string(IDS_AANEW));

    gtk_box_pack_start(GTK_BOX(vbox), button_ok, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), button_cancel, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), button_help, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), button_edit, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), button_new, FALSE, FALSE, 5);

    /* Connect our callbacks to the buttons */
    gtk_signal_connect(GTK_OBJECT(button_ok), "clicked",
                              GTK_SIGNAL_FUNC(modal_ok), &rc);
    gtk_signal_connect(GTK_OBJECT(button_cancel), "clicked",
                              GTK_SIGNAL_FUNC(modal_cancel), &rc);
    gtk_signal_connect(GTK_OBJECT(button_help), "clicked",
                              GTK_SIGNAL_FUNC(modal_help), &rc);
    gtk_signal_connect(GTK_OBJECT(button_edit), "clicked",
                              GTK_SIGNAL_FUNC(edit_prop_dialog), &pv);
    gtk_signal_connect(GTK_OBJECT(button_new), "clicked",
                              GTK_SIGNAL_FUNC(new_prop_dialog), &pv);

    gtk_widget_show(button_ok);
    gtk_widget_show(button_cancel);
    gtk_widget_show(button_help);
    gtk_widget_show(button_edit);
    gtk_widget_show(button_new);

    /* show dialog and wait for OK, Cancel or close */
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_show(window);
    gtk_main();

    if (rc == IDOK) {
	if ( (prf = profile_open(szIniFile)) != (PROFILE *)NULL ) {
	    char section[MAXSTR];
	    for (iprop=0; pv.propitem && pv.propitem[iprop].name[0]; iprop++) {
		profile_write_string(prf, pv.device, 
		    pv.propitem[iprop].name, pv.propitem[iprop].value);
	    }
	    strcpy(section, pv.device);
	    strcat(section, " Options");
	    profile_write_string(prf, section, "Xoffset", 
	    	gtk_entry_get_text(GTK_ENTRY(x_offset)));
	    profile_write_string(prf, section, "Yoffset", 
	    	gtk_entry_get_text(GTK_ENTRY(x_offset)));
	    profile_close(prf);
	}
    }
    /* we must disconnect these signals before destroying widget,
     * otherwise they will be called after widget is destroyed.
     */
    gtk_signal_disconnect(GTK_OBJECT(GTK_COMBO(prop)->list), prop_signal);
    gtk_signal_disconnect(GTK_OBJECT(GTK_COMBO(value)->list), value_signal);

    gtk_widget_destroy(window);
    free((char *)pv.propitem);
    pv.propitem = NULL;
}

void
advanced_ps_dialog(GtkWidget *w, gpointer data)
{
    GtkWidget *window;		/* main dialog window */
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *check_ctrld_before;
    GtkWidget *check_ctrld_after;
    GtkWidget *label;
    GtkWidget *con;
    GtkWidget *entry_prolog;
    GtkWidget *entry_epilog;
    GtkWidget *hbox_buttons;		/* for buttons */
    GtkWidget *button_ok, *button_cancel, *button_help;
    int rc = 0;

    /* for PostScript printers, provide options for sending
     * Ctrl+D before and after job, and sending a prolog
     * and epilog file.
     * These are set using the Advanced button on the Printer
     * Setup dialog, only enabled for PostScript printer.
     */
/* currently enabled always */
    PROFILE *prf;
    char buf[MAXSTR];
    char section[MAXSTR];
    int prectrld=0;
    int postctrld=0;

    window=gtk_window_new(GTK_WINDOW_DIALOG);

    gtk_window_set_title(GTK_WINDOW(window), get_string(IDS_AAADVANCEDPSOPT));

    gtk_signal_connect(GTK_OBJECT(window),
                       "delete-event",
                       GTK_SIGNAL_FUNC(modal_delete),
                       &rc);

    hbox = gtk_hbox_new(TRUE, 4);
    gtk_container_add(GTK_CONTAINER(window), hbox);
    gtk_widget_show(hbox);
    
    vbox = gtk_vbox_new(FALSE, 4);
    gtk_widget_show(vbox);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 4);

    check_ctrld_before = gtk_check_button_new_with_label(
	get_string(IDS_AASENDCTRLDBEFORE));
    gtk_widget_show(check_ctrld_before);
    gtk_box_pack_start(GTK_BOX(vbox), check_ctrld_before, FALSE, FALSE, 4);

    check_ctrld_after = gtk_check_button_new_with_label(
	get_string(IDS_AASENDCTRLDAFTER));
    gtk_widget_show(check_ctrld_after);
    gtk_box_pack_start(GTK_BOX(vbox), check_ctrld_after, FALSE, FALSE, 4);

    label = gtk_label_new(get_string(IDS_AAPROLOGFILE));
    gtk_widget_show(label);
    con = gtk_alignment_new(0, 0, 0, 0);	/* left alignment */
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), label);
    gtk_box_pack_start(GTK_BOX(vbox), con, FALSE, FALSE, 0);

    entry_prolog = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_prolog), MAXSTR-1);
    gtk_box_pack_start(GTK_BOX(vbox), entry_prolog, FALSE, FALSE, 0);
    gtk_widget_show(entry_prolog);

    label = gtk_label_new(get_string(IDS_AAEPILOGFILE));
    gtk_widget_show(label);
    con = gtk_alignment_new(0, 0, 0, 0);	/* left alignment */
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), label);
    gtk_box_pack_start(GTK_BOX(vbox), con, FALSE, FALSE, 0);

    entry_epilog = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_epilog), MAXSTR-1);
    gtk_box_pack_start(GTK_BOX(vbox), entry_epilog, FALSE, FALSE, 0);
    gtk_widget_show(entry_epilog);


    /* Create and place the buttons in a hbox at the bottom */
    hbox_buttons = gtk_hbox_new(TRUE, 4);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_buttons, FALSE, FALSE, 8);
    gtk_widget_show(hbox_buttons);

    button_ok = gtk_button_new_with_label(get_string(IDS_AAOK));
    button_cancel = gtk_button_new_with_label(get_string(IDS_AACANCEL));
    button_help = gtk_button_new_with_label(get_string(IDS_AAHELP));

    gtk_box_pack_start(GTK_BOX(hbox_buttons), button_ok, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(hbox_buttons), button_cancel, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(hbox_buttons), button_help, TRUE, TRUE, 5);

    /* Connect our callbacks to the buttons */
    gtk_signal_connect(GTK_OBJECT(button_ok), "clicked",
                              GTK_SIGNAL_FUNC(modal_ok), &rc);
    gtk_signal_connect(GTK_OBJECT(button_cancel), "clicked",
                              GTK_SIGNAL_FUNC(modal_cancel), &rc);
    gtk_signal_connect(GTK_OBJECT(button_help), "clicked",
                              GTK_SIGNAL_FUNC(modal_help), &rc);

    gtk_widget_show(button_ok);
    gtk_widget_show(button_cancel);
    gtk_widget_show(button_help);

    /* initialise fields */
strcpy(section, "Options");
/* section should be printer name ?? */
/* maybe get this from parent widget/data */

    if ( (prf = profile_open(szIniFile)) != (PROFILE *)NULL ) {
	profile_read_string(prf, section, "PreCtrlD", "0", buf, 
	    sizeof(buf)-2);
	if (sscanf(buf, "%d", &prectrld) != 1)
	    prectrld = 0;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_ctrld_before), 
	    prectrld);
	profile_read_string(prf, section, "PostCtrlD", "0", buf, 
	    sizeof(buf)-2);
	if (sscanf(buf, "%d", &postctrld) != 1)
	    postctrld = 0;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_ctrld_after), 
	    postctrld);
	profile_read_string(prf, section, "Prolog", "", buf, 
	   sizeof(buf)-2);
        gtk_entry_set_text(GTK_ENTRY(entry_prolog), buf);
	profile_read_string(prf, section, "Epilog", "", buf, 
	   sizeof(buf)-2);
        gtk_entry_set_text(GTK_ENTRY(entry_epilog), buf);
	profile_close(prf);
    }

    /* show dialog and wait for OK, Cancel or close */
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_show(window);
    gtk_main();

    if ( (rc == IDOK) && 
         ((prf = profile_open(szIniFile)) != (PROFILE *)NULL) ) {
	int prectrld;
	int postctrld;
	/* save settings */
	prectrld = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(check_ctrld_before));
	profile_write_string(prf, section, "PreCtrlD", prectrld ? "1" : "0");
	postctrld = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(check_ctrld_after));
	profile_write_string(prf, section, "PostCtrlD", postctrld ? "1" : "0");
	profile_write_string(prf, section, "Prolog", 
	    gtk_entry_get_text(GTK_ENTRY(entry_prolog)));
	profile_write_string(prf, section, "Epilog", 
	    gtk_entry_get_text(GTK_ENTRY(entry_epilog)));
	profile_close(prf);
    }
    gtk_widget_destroy(window);
}

int print_dialog_box(BOOL convert)
{
    GtkWidget *window;		/* main dialog window */
    GtkWidget *hbox;		/* hbox for list on left, buttons on right */
    GtkWidget *vbox;		/* vbox on right for buttons */
    GtkWidget *vbox_dev;
    GtkWidget *device;
    GtkWidget *dev_label;
    GtkWidget *vbox_res;
    GtkWidget *resolution;
    GtkWidget *res_label;
    GtkWidget *hbox_devres;
    GtkWidget *button_reverse=NULL;
    GtkWidget *options_label;
    GtkWidget *options;
    GtkWidget *vbox_left;
    GtkWidget *combo_fixed_media=NULL;
    GtkWidget *hbox_ps=NULL;
    GtkWidget *button_psprint=NULL;
    GtkWidget *button_psadv=NULL;
    GtkWidget *button_printfile=NULL;
    GtkWidget *printer_label;
    GtkWidget *printer=NULL;
    GtkWidget *scrolled_window, *clist=NULL;	/* page list */
    GtkWidget *button_ok, *button_cancel, *button_prop, *button_help;
    GtkWidget *button_all=NULL, *button_odd=NULL, *button_even=NULL;    
    GtkWidget *con;
    guint res_signal, options_signal;
    int rc = 0;
    int i;
    char *devs, *dev;
    GList *list, *l;
    DEVRES dr;
    PROFILE *prf;

    if (psfile.dsc != (CDSC *)NULL) {
	psfile.page_list.current = psfile.pagenum-1;
	psfile.page_list.multiple = TRUE;
	if (psfile.page_list.select != (BOOL *)NULL)
	    for (i=0; i< (int)(psfile.dsc->page_count); i++)
		psfile.page_list.select[i] = TRUE;
    }

    window=gtk_window_new(GTK_WINDOW_DIALOG);
    /* Should we set a a default size? */

    gtk_window_set_title(GTK_WINDOW(window), 
	convert ? get_string(IDS_AACONVERT) : get_string(IDS_AAPRINTERSETUP));

    gtk_signal_connect(GTK_OBJECT(window),
                       "delete-event",
                       GTK_SIGNAL_FUNC(modal_delete),
                       &rc);
    
    hbox = gtk_hbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_container_add(GTK_CONTAINER(window), hbox);
    gtk_widget_show(hbox);


    vbox_left = gtk_vbox_new(FALSE, 2);
    gtk_widget_show(vbox_left);
    gtk_box_pack_start(GTK_BOX(hbox), vbox_left, TRUE, TRUE, 4);

    hbox_devres = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox_devres);
    gtk_box_pack_start(GTK_BOX(vbox_left), hbox_devres, FALSE, FALSE, 4);

    /* Device */
    vbox_dev = gtk_vbox_new(FALSE, 2);
    gtk_widget_show(vbox_dev);
    dev_label = gtk_label_new(get_string(IDS_AADEVICEC));
    gtk_widget_show(dev_label);
    con = gtk_alignment_new(0, 0, 0, 0);	/* left alignment */
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), dev_label);
    gtk_box_pack_start(GTK_BOX(vbox_dev), con, FALSE, FALSE, 0);
    device = gtk_label_new("DEVICE COMBO BOX");

    list = NULL;
    dev = devs = get_devices(convert);
    while ( dev!=(char *)NULL && strlen(dev)!=0) {
	list = g_list_append(list, dev);
	dev += strlen(dev) + 1;
    }
    device = gtk_combo_new();
    if (list != NULL) {
	gtk_combo_set_popdown_strings(GTK_COMBO(device), list);
	g_list_free(list);
	list = NULL;
    }
    free(devs);
    gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(device)->entry), TRUE);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(device)->entry), 
	convert ? option.convert_device : option.printer_device);
    gtk_widget_show(device);
    gtk_box_pack_start(GTK_BOX(vbox_dev), device, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_devres), vbox_dev, TRUE, TRUE, 0);
    gtk_signal_connect(GTK_OBJECT(GTK_COMBO(device)->entry), 
	"size-request", GTK_SIGNAL_FUNC(width_percent), (gpointer)70);

    /* Resolution */
    vbox_res = gtk_vbox_new(FALSE, 2);
    gtk_widget_show(vbox_res);
    res_label = gtk_label_new(get_string(IDS_AARESOLUTIONC));
    gtk_widget_show(res_label);
    con = gtk_alignment_new(0, 0, 0, 0);	/* left alignment */
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), res_label);
    gtk_box_pack_start(GTK_BOX(vbox_res), con, FALSE, FALSE, 0);
    resolution = gtk_combo_new();
    gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(resolution)->entry), TRUE);
    gtk_widget_show(resolution);
    gtk_box_pack_start(GTK_BOX(vbox_res), resolution, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_devres), vbox_res, TRUE, TRUE, 0);
    gtk_signal_connect(GTK_OBJECT(GTK_COMBO(resolution)->entry), 
	"size-request", GTK_SIGNAL_FUNC(width_percent), (gpointer)70);

    /* Fixed media */
    combo_fixed_media = gtk_combo_new();
    {	const char *s;
	char *p;
	GList *list = NULL;

 	for (i=0; i<3; i++) {
	    s = get_string(IDS_PAGESIZE_VARIABLE+i);
	    p = (char *)malloc(strlen(s)+1);
	    if (p) {
		strcpy(p, s);
		list = g_list_append(list, p);
	    }
	}

	gtk_combo_set_popdown_strings(GTK_COMBO(combo_fixed_media), list);
	l = list;
	while (l) {
	    free(l->data);
	    l = g_list_next(l);
	}
	g_list_free(list);
	list = NULL;
    }
    gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(combo_fixed_media)->entry), FALSE);
    gtk_widget_show(combo_fixed_media);
    gtk_box_pack_start(GTK_BOX(vbox_left), combo_fixed_media, FALSE, FALSE, 4);

    i = convert ? option.convert_fixed_media : option.print_fixed_media;
    if (i < 0)
	i = 0;
    if (i >=3)
	i = 2;
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_fixed_media)->entry),
	 get_string(IDS_PAGESIZE_VARIABLE+i));


    /* Reverse */
    button_reverse = gtk_check_button_new_with_label(get_string(IDS_AAREVERSE));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_reverse), 
	option.print_reverse);
    gtk_widget_show(button_reverse);
    gtk_box_pack_start(GTK_BOX(vbox_left), button_reverse, FALSE, FALSE, 4);

    if (!convert) {
	/* Print */
	button_printfile = gtk_check_button_new_with_label(get_string(IDS_AAPRINTTOFILE));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_printfile), 
	    option.print_to_file);
	gtk_widget_show(button_printfile);
	gtk_box_pack_start(GTK_BOX(vbox_left), button_printfile, FALSE, 
	    FALSE, 4);

	hbox_ps = gtk_hbox_new(FALSE, 5);
	gtk_container_set_border_width(GTK_CONTAINER(hbox_ps), 0);
	gtk_container_add(GTK_CONTAINER(vbox_left), hbox_ps);
	gtk_widget_show(hbox_ps);
	button_psprint = gtk_check_button_new_with_label(
	    get_string(IDS_AAPSPRINTER));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_psprint), 
	    (option.print_method == PRINT_PS));
	gtk_widget_show(button_psprint);
	gtk_box_pack_start(GTK_BOX(hbox_ps), button_psprint, FALSE, FALSE, 0);
        button_psadv = gtk_button_new_with_label(get_string(IDS_AAADVANCED));
	gtk_widget_show(button_psadv);
	gtk_box_pack_end(GTK_BOX(hbox_ps), button_psadv, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(button_psadv), "clicked",
			  GTK_SIGNAL_FUNC(advanced_ps_dialog), NULL);

	printer_label = gtk_label_new(get_string(IDS_AAQUEUEC));
	gtk_widget_show(printer_label);
	con = gtk_alignment_new(0, 0, 0, 0);	/* left alignment */
	gtk_widget_show(con);
	gtk_container_add(GTK_CONTAINER(con), printer_label);
	gtk_box_pack_start(GTK_BOX(vbox_left), con, FALSE, FALSE, 0);
	printer = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(printer), 255);
	gtk_entry_set_text(GTK_ENTRY(printer), option.printer_queue);
	gtk_box_pack_start(GTK_BOX(vbox_left), printer, FALSE, FALSE, 4);
	gtk_widget_show(printer);
    }

    /* Options */
    options_label = gtk_label_new(get_string(IDS_AAOPTIONSC));
    gtk_widget_show(options_label);
    con = gtk_alignment_new(0, 0, 0, 0);	/* left alignment */
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), options_label);
    gtk_box_pack_start(GTK_BOX(vbox_left), con, FALSE, FALSE, 0);
    options = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(options), 255);
    gtk_entry_set_text(GTK_ENTRY(options), "");
    gtk_box_pack_start(GTK_BOX(vbox_left), options, FALSE, FALSE, 4);
    gtk_widget_show(options);

    /* update resolutions and options when device changed */
    dr.device = device;
    dr.resolution = resolution;
    dr.options = options;
    dr.bConvert = convert;
    res_signal = gtk_signal_connect(GTK_OBJECT(GTK_COMBO(device)->list), 
	"selection-changed", GTK_SIGNAL_FUNC(resolution_update), &dr);
    resolution_update(GTK_COMBO(device)->list, &dr);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(resolution)->entry), 
	convert ? option.convert_resolution : option.printer_resolution);
    options_signal = gtk_signal_connect(GTK_OBJECT(GTK_COMBO(device)->list), 
	"selection-changed", GTK_SIGNAL_FUNC(options_update), &dr);
    options_update(GTK_COMBO(device)->list, &dr);

    if (psfile.dsc != (CDSC *)NULL) {
        char buf[MAXSTR];
	char *title;
	/* Create a scrolled window to pack the CList widget into */
	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_signal_connect(GTK_OBJECT(scrolled_window), "size-request",
			  GTK_SIGNAL_FUNC(width_percent), (gpointer)180);

	gtk_box_pack_start(GTK_BOX(hbox), scrolled_window, TRUE, TRUE, 0);
	gtk_widget_show (scrolled_window);

	strncpy(buf, get_string(IDS_AAPAGES), sizeof(buf)-1);
	title = buf;
	/* Create the CList for the pages */
	clist = gtk_clist_new_with_titles(1, (gchar **)&title);

	gtk_clist_set_shadow_type (GTK_CLIST(clist), GTK_SHADOW_OUT);

	/* We must set the default columnn width, in pixels */
	gtk_clist_set_column_width (GTK_CLIST(clist), 0, 64);

	/* Add the CList widget to the horizontal box and show it. */
	gtk_container_add(GTK_CONTAINER(scrolled_window), clist);
	gtk_widget_show(clist);
    }

    /* Create and place the buttons in a vbox at the right */
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);
    gtk_widget_show(vbox);

    button_ok = gtk_button_new_with_label(get_string(IDS_AAOK));
    button_cancel = gtk_button_new_with_label(get_string(IDS_AACANCEL));
    button_prop = gtk_button_new_with_label(get_string(IDS_AAPROPERTIES));
    button_help = gtk_button_new_with_label(get_string(IDS_AAHELP));

    gtk_box_pack_start(GTK_BOX(vbox), button_ok, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), button_cancel, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), button_prop, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), button_help, FALSE, FALSE, 5);

    /* Connect our callbacks to the buttons */
    gtk_signal_connect(GTK_OBJECT(button_ok), "clicked",
                              GTK_SIGNAL_FUNC(modal_ok), &rc);
    gtk_signal_connect(GTK_OBJECT(button_cancel), "clicked",
                              GTK_SIGNAL_FUNC(modal_cancel), &rc);
    gtk_signal_connect(GTK_OBJECT(button_help), "clicked",
                              GTK_SIGNAL_FUNC(modal_help), &rc);
    gtk_signal_connect(GTK_OBJECT(button_prop), "clicked",
                              GTK_SIGNAL_FUNC(prop_dialog), &dr);

    gtk_widget_show(button_ok);
    gtk_widget_show(button_cancel);
    gtk_widget_show(button_prop);
    gtk_widget_show(button_help);

    if (psfile.dsc != (CDSC *)NULL) {
	button_all = gtk_button_new_with_label(get_string(IDS_AAALLPAGES));
	button_odd = gtk_button_new_with_label(get_string(IDS_AAODDPAGES));
	button_even = gtk_button_new_with_label(get_string(IDS_AAEVENPAGES));
	gtk_box_pack_start(GTK_BOX(vbox), button_all, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), button_odd, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), button_even, FALSE, FALSE, 5);
	gtk_signal_connect(GTK_OBJECT(button_all), "clicked",
				  GTK_SIGNAL_FUNC(selpage_all),
				  (gpointer) clist);
	gtk_signal_connect(GTK_OBJECT(button_odd), "clicked",
				  GTK_SIGNAL_FUNC(selpage_odd),
				  (gpointer) clist);
	gtk_signal_connect(GTK_OBJECT(button_even), "clicked",
				  GTK_SIGNAL_FUNC(selpage_even),
				  (gpointer) clist);
	gtk_widget_show(button_all);
	gtk_widget_show(button_odd);
	gtk_widget_show(button_even);
    }

    if (psfile.dsc != (CDSC *)NULL) {
	char buf[64];
	char *p = buf;
	for (i=0; i< (int)(psfile.dsc->page_count); i++) {
	    page_ordlabel(buf, i);
	    gtk_clist_append( (GtkCList *) clist, &p);
	    if (psfile.page_list.select[i]
		|| (i == psfile.page_list.current))
		gtk_clist_select_row(GTK_CLIST(clist), i, 0);
	}
	gtk_clist_set_selection_mode(GTK_CLIST(clist), GTK_SELECTION_EXTENDED);
	selpage_all(GTK_BUTTON(button_all), (gpointer)clist);
    }

    /* show dialog and wait for OK, Cancel or close */
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_show(window);
    gtk_main();

    if (rc == IDOK) {
	char buf[MAXSTR];
	if (psfile.dsc != (CDSC *)NULL) {
	    GList *l = GTK_CLIST(clist)->selection;
	    for (i = 0; i < (int)psfile.dsc->page_count; i++)
		psfile.page_list.select[i] = FALSE;
	    while (l) {
		psfile.page_list.current = (int)(l->data);
		psfile.page_list.select[(int)(l->data)] = TRUE;
		l = g_list_next(l);
	    }
	    option.print_reverse = 
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button_reverse));
	}
	if (convert) {
	    strncpy(option.convert_device, 
		gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(device)->entry)),
		sizeof(option.convert_device)-1);
	    strncpy(option.convert_resolution, 
		gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(resolution)->entry)),
		sizeof(option.convert_resolution)-1);
	    strncpy(buf, gtk_entry_get_text( GTK_ENTRY(
		GTK_COMBO(combo_fixed_media)->entry)), sizeof(buf)-1);
	    for (i=0; i<3; i++) {
		if (strcmp(buf, get_string(IDS_PAGESIZE_VARIABLE+i)) == 0)
		    option.convert_fixed_media = i;
	    }
	}
	else {
	    strncpy(option.printer_device, 
		gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(device)->entry)),
		sizeof(option.printer_device)-1);
	    strncpy(option.printer_resolution, 
		gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(resolution)->entry)),
		sizeof(option.printer_resolution)-1);
	    strncpy(buf, gtk_entry_get_text( GTK_ENTRY(
		GTK_COMBO(combo_fixed_media)->entry)), sizeof(buf)-1);
	    for (i=0; i<3; i++) {
		if (strcmp(buf, get_string(IDS_PAGESIZE_VARIABLE+i)) == 0)
		    option.print_fixed_media = i;
	    }
	    option.print_to_file = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(button_printfile));
	    if (gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(button_psprint)))
	        option.print_method = PRINT_PS;
	    else
	        option.print_method = PRINT_GS;
	    strncpy(option.printer_queue, 
		gtk_entry_get_text(GTK_ENTRY(printer)),
		sizeof(option.printer_queue)-1);
	}
	if ( (prf = profile_open(szIniFile)) != (PROFILE *)NULL ) {
	    char section[MAXSTR];
	    strcpy(section, convert ? option.convert_device :
		option.printer_device);
	    strcat(section, " Options");
	    profile_write_string(prf, section, "Options", 
	    	gtk_entry_get_text(GTK_ENTRY(options)));
	    profile_close(prf);
	}
    }

    gtk_signal_disconnect(GTK_OBJECT(GTK_COMBO(device)->list), res_signal);
    gtk_signal_disconnect(GTK_OBJECT(GTK_COMBO(device)->list), options_signal);

    gtk_widget_destroy(window);

    return (rc == IDOK);
}

GtkWidget *print_window;
GtkWidget *print_text;
/* should have a label which show % printed */

gint delete_print_message(GtkWidget *widget, GdkEvent event, gpointer data)
{
    if (debug & DEBUG_GENERAL)
	gs_addmess("delete_print_message:\n");
    print_window = NULL;
    print_text = NULL;
    /* emit the "destroy" signal */
    return FALSE;
}

void percent_print_message(int percent)
{
    if (print_window != NULL) {
	char buf[MAXSTR];
	sprintf(buf, "%d%% - %s", percent, get_string(IDS_AAGSVIEWPRINT));
	gtk_window_set_title(GTK_WINDOW(print_window), buf);
    }
}

void add_print_message(char *str, int len) 
{
   if (debug & DEBUG_GENERAL)
	write(fileno(stdout), str, len);
   if (print_text != NULL) 
       gtk_text_insert(GTK_TEXT(print_text), NULL, NULL, NULL, str, len); 
}

void close_print_message(void)
{
    if (print_window != NULL) {
        gtk_widget_hide(print_window);
        gtk_widget_destroy(print_window);
    }
    print_window = NULL;
    print_text = NULL;
}

void show_print_message(void)
{
    GtkWidget *table;
    GtkWidget *vscrollbar;

    close_print_message();	/* close any existing window */

    print_window=gtk_window_new(GTK_WINDOW_DIALOG);
    gtk_window_set_title(GTK_WINDOW(print_window), get_string(IDS_AAGSVIEWPRINT));
    gtk_widget_set_usize(GTK_WIDGET(print_window), 450, 300);

    gtk_signal_connect(GTK_OBJECT(print_window), "delete-event",
                       GTK_SIGNAL_FUNC(delete_print_message), NULL);
    
    table = gtk_table_new(2, 2, FALSE);
    gtk_table_set_row_spacing(GTK_TABLE(table), 0, 2);
    gtk_table_set_col_spacing(GTK_TABLE(table), 0, 2);
    gtk_container_add(GTK_CONTAINER(print_window), table);
    gtk_widget_show(table);
   
    /* Create text */
    print_text = gtk_text_new(NULL, NULL);
    gtk_table_attach(GTK_TABLE(table), print_text, 0, 1, 0, 1,
	(GtkAttachOptions)(GTK_EXPAND | GTK_SHRINK | GTK_FILL),
	(GtkAttachOptions)(GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_widget_show(print_text);

    /* Add a vertical scroll bar to the GtkText widget */
    vscrollbar = gtk_vscrollbar_new(GTK_TEXT(print_text)->vadj);
    gtk_table_attach(GTK_TABLE(table), vscrollbar, 1, 2, 0, 1,
	(GtkAttachOptions)(GTK_FILL),
	(GtkAttachOptions)(GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_widget_show(vscrollbar);

    gtk_widget_show(print_window);
    /* modeless dialog */
}


int gs_print_pipe_stdin[2] = {-1, -1};
int gs_print_pipe_stdout[2] = {-1, -1};
int gs_print_pipe_stderr[2] = {-1, -1};
gint gs_print_pipe_stdin_tag = -1;
gint gs_print_pipe_stdout_tag = -1;
gint gs_print_pipe_stderr_tag = -1;
int gs_print_file;
char *gs_print_buffer;
int gs_print_buffer_index;	/* offset to next byte to send */
int gs_print_buffer_count;	/* remaining bytes to send */
int gs_print_buffer_length;	/* length of buffer */

long gs_print_bytes_done;
long gs_print_bytes_size;
int gs_print_percent;

void print_start_gs(void);
int print_stop_gs(void);
void print_check_zombie(void);
void print_stop_stdin(void);
void print_start_stdin(void);
void print_stop_stdout(void);
void print_start_stdout(void);
void print_stop_stderr(void);
void print_start_stderr(void);
void print_write_fn(gpointer data, gint fd, GdkInputCondition condition);
void print_read_stdout_fn(gpointer data, gint fd, GdkInputCondition condition);

int print_stop_gs(void)
{
    BOOL keep_message = FALSE;
    if (debug & DEBUG_GENERAL)
	gs_addmess("print_stop_gs\n");
    print_stop_stdin();
    if (gs_print_pipe_stdin[1] != -1) {
	close(gs_print_pipe_stdin[1]);
	gs_print_pipe_stdin[1] = -1;
    }
    print_stop_stdout();
    if (gs_print_pipe_stdout[0] != -1) {
	close(gs_print_pipe_stdout[0]);
	gs_print_pipe_stdout[0] = -1;
    }
    print_stop_stderr();
    if (gs_print_pipe_stderr[0] != -1) {
	close(gs_print_pipe_stderr[0]);
	gs_print_pipe_stderr[0] = -1;
    }

    if (printer.pid > 0) {
	int rc;
	int status = 0;
	/* get termination code */
	usleep(100000);	/* allow a little time for child to finish */
	rc = waitpid(printer.pid, &status, WNOHANG);
	if (debug & DEBUG_GENERAL)
	    gs_addmessf("print_stop_gs: waitpid rc=%d status=%d errno=%d\n", rc, status, errno);
	if (rc > 0) {
	    /* child process has ended */
	    if (WIFEXITED(status)) {
		if (debug & DEBUG_GENERAL)
		    gs_addmessf("print_stop_gs: Ghostscript (zombie) exit code %d\n", 
		    WEXITSTATUS(status));
		if (WEXITSTATUS(status) != 0) {
		    gs_showmess();	/* show error message */
		    keep_message = TRUE;
	 	}
/* TESTING TESTING TESTING */
/* should show printer window */
	    }
	    else {
		if (debug & DEBUG_GENERAL)
		    gs_addmessf("print_stop_gs: child ended, but rc=%d, status=%d\n", 
			rc, status); 
	    }
	}
	else if (rc == 0) {
	    /* child has not yet ended */
	    if (debug & DEBUG_GENERAL)
		gs_addmess("print_stop_gs: killing Ghostscript\n");
	    kill(printer.pid, SIGTERM);
	    waitpid(printer.pid, &status, 0);
	}
	else {
	    /* error - no such process */
	    if (debug & DEBUG_GENERAL)
		gs_addmessf("print_stop_gs: waitpid rc=%d errno=%d\n", rc, errno);
	}

	printer.pid = 0;
    }

    if (gs_print_file > 0) {
	close(gs_print_file);
	gs_print_file = 0;
    }
    if (printer.psname[0]) {
	if (!(debug & DEBUG_GENERAL))
	    unlink(printer.psname);
	printer.psname[0] = '\0';
    }
    if (printer.optname[0]) {
	if (!(debug & DEBUG_GENERAL))
	    unlink(printer.optname);
	printer.optname[0] = '\0';
    }
    if (gs_print_buffer) {
	free(gs_print_buffer);
	gs_print_buffer = NULL;
    }
    gs_print_buffer_length = 0;
    gs_print_buffer_count = 0;
    gs_print_buffer_index = 0;

    if (!keep_message && !(debug & DEBUG_GENERAL))
	close_print_message();

    return 0;
}


/* see if print has finished */
/* May need to call this before gsview_command() to cleanup
 * before next print 
 */
void print_check_zombie(void)
{
    /* if process is a zombie, close and delete temporary files */
    if (printer.pid > 0) {
	/* check if Ghostscript has exited prematurely */
	int status = 0;
	if (waitpid(printer.pid, &status, WNOHANG) > 0) {
	    if (debug & DEBUG_GENERAL)
		gs_addmessf("print_check_zombie: calling print_stop_gs()\n");
	    /* Printing Ghostscript has exited, release resources */
	    printer.pid = -1;
	    print_stop_gs();
	}
    }
}

/* Asynchronous write to GS print stdin.
 * This is called from event loop when a write is possible 
 * on gs_print_stdin_pipe.
 */
void print_write_fn(gpointer data, gint fd, GdkInputCondition condition)
{
    if (fd != gs_print_pipe_stdin[1]) {
	gs_addmess("print_write_fn: called with wrong source\n");
	return;
    }
    if (gs_print_file <=0) {
	gs_addmess("print_write_fn: gs_print_file is closed\n");
	return;
    }
    if (gs_print_pipe_stdin[1] == -1) {
	gs_addmess("print_write_fn: gs_print_pipe_stdin[1] is closed\n");
	return;
    }
    if (condition == GDK_INPUT_EXCEPTION) {
	/* complain */
	gs_addmess("print_write_fn: exception\n");
	close(gs_print_pipe_stdin[1]);
	gs_print_pipe_stdin[1] = -1;
	return;
    }
    else if (condition == GDK_INPUT_WRITE) {
        int pcdone;
	int bytes_written = 0;
	do {
	    if (gs_print_buffer_count == 0) {
		gs_print_buffer_count = read(gs_print_file,
			gs_print_buffer, gs_print_buffer_length);
	    }
	    if (gs_print_buffer_count == 0) {
		print_stop_stdin();
		close(gs_print_file);
		gs_print_file = -1;
	        if (gs_print_pipe_stdin[1] != -1) {
			close(gs_print_pipe_stdin[1]);
			gs_print_pipe_stdin[1] = -1;
		}
		break;
	    }
	    
	    bytes_written = write(gs_print_pipe_stdin[1], 
		gs_print_buffer+gs_print_buffer_index, gs_print_buffer_count);
	    if (bytes_written == -1) {
		break;	/* come back later */
	    }
	    else if (bytes_written == 0) {
		/* pipe probably closed */
		gs_addmess("print_write_fn: wrote 0 bytes to GS\n");
		print_check_zombie();
	    }
	    else {
		gs_print_buffer_count -= bytes_written;
		if (gs_print_buffer_count == 0)
		   gs_print_buffer_index = 0;
		else
		   gs_print_buffer_index += bytes_written;

		gs_print_bytes_done += bytes_written;
		pcdone = (int)(gs_print_bytes_done * 100 / gs_print_bytes_size);
		if (pcdone != gs_print_percent) {
		    gs_print_percent = pcdone;
		    percent_print_message(gs_print_percent);
		}
	    }

	} while (bytes_written > 0);
    }
    else {
	gs_addmessf("print_write_fn: unknown condition %d\n", condition);
    }
}

/* Asynchronous read of GS print stdout/stderr.
 * This is called from event loop when a read is possible 
 * on gs_print_stdout_pipe or gs_print_stderr_pipe.
 */
void print_read_stdout_fn(gpointer data, gint fd, GdkInputCondition condition)
{
    int is_stdout = (fd == gs_print_pipe_stdout[0]);
    int is_stderr = (fd == gs_print_pipe_stderr[0]);

    if (!is_stdout && !is_stderr) {
	gs_addmess("print_read_stdout_fn: called with wrong source\n");
	return;
    }

    if (condition == GDK_INPUT_EXCEPTION) {
	/* complain */
	gs_addmess("print_read_stdout_fn: exception\n");
	if (is_stdout)
	    print_stop_stdout();
	else
	    print_stop_stderr();
	return;
    }
    else if (condition == GDK_INPUT_READ) {
	char buf[256];
	int bytes_read = 0;
	do {
	    bytes_read = read(fd, buf, sizeof(buf));
	    if (bytes_read == -1) {
		if (errno == EAGAIN) {
		    break;	/* come back later */
		}
		else {
		    gs_addmessf("print_read_stdout_fn: read from GS failed, errno=%d\n", errno);
		}
	    }
	    else if (bytes_read == 0) {
		gs_addmessf("print_read_stdout_fn: read 0 bytes from GS %s\n",
			is_stdout ? "stdout" : "stderr");
		print_check_zombie();
	    }
	    else {
		add_print_message(buf, bytes_read);
	    }

	} while (bytes_read > 0);
    }
    else {
	gs_addmessf("print_read_stdout_fn: unknown condition %d\n", condition);
    }
}

void print_stop_stdin(void)
{
    if (gs_print_pipe_stdin_tag >=0) {
        gdk_input_remove(gs_print_pipe_stdin_tag);
	gs_print_pipe_stdin_tag = -1;
	if (debug & DEBUG_GENERAL)
	    gs_addmess("print_stop_stdin:\n");
    }
}

void print_start_stdin(void)
{
    if (gs_print_pipe_stdin_tag >=0)
	return;
    gs_print_pipe_stdin_tag = gdk_input_add(gs_print_pipe_stdin[1], 
	(GdkInputCondition)(GDK_INPUT_WRITE | GDK_INPUT_EXCEPTION),
	print_write_fn, 0);
    if (debug & DEBUG_GENERAL)
	gs_addmess("print_start_stdin:\n");
}

void print_stop_stdout(void)
{
    if (gs_print_pipe_stdout_tag >=0) {
        gdk_input_remove(gs_print_pipe_stdout_tag);
	gs_print_pipe_stdout_tag = -1;
	if (debug & DEBUG_GENERAL)
	    gs_addmess("print_stop_stdout:\n");
    }
}

void print_start_stdout(void)
{
    if (gs_print_pipe_stdout_tag >=0)
	return;
    gs_print_pipe_stdout_tag = gdk_input_add(gs_print_pipe_stdout[0], 
	(GdkInputCondition)(GDK_INPUT_READ | GDK_INPUT_EXCEPTION),
	print_read_stdout_fn, 0);
    if (debug & DEBUG_GENERAL)
	gs_addmess("print_start_stdout:\n");
}

void print_stop_stderr(void)
{
    if (gs_print_pipe_stderr_tag >=0) {
        gdk_input_remove(gs_print_pipe_stderr_tag);
	gs_print_pipe_stderr_tag = -1;
	if (debug & DEBUG_GENERAL)
	    gs_addmess("print_stop_stderr:\n");
    }
}

void print_start_stderr(void)
{
    if (gs_print_pipe_stderr_tag >=0)
	return;
    gs_print_pipe_stderr_tag = gdk_input_add(gs_print_pipe_stderr[0], 
	(GdkInputCondition)(GDK_INPUT_READ | GDK_INPUT_EXCEPTION),
	print_read_stdout_fn, 0);
    if (debug & DEBUG_GENERAL)
	gs_addmess("print_start_stderr:\n");
}


/* print will fork another process to run Ghostscript.
 * stdin will be obtained from main process
 * stdout/err will be written to a window owned by the main process.
 */
void print_start_gs(void)
{
    char *nargv[10];
    char dash_string[] = "-";
    char dashc_string[] = "-c";
    char quit_string[] = "quit";

    gs_print_file = open(printer.psname, O_RDONLY);
    if (gs_print_file == -1) {
	print_stop_gs();
	return;
    }
    gs_print_bytes_size = lseek(gs_print_file, 0, SEEK_END);
    lseek(gs_print_file, 0, SEEK_SET);
    gs_print_bytes_done = 0;
    gs_print_percent = 0;
    gs_print_buffer_count = 0;
    gs_print_buffer_index = 0;
    gs_print_buffer_length = 4096;
    gs_print_buffer = (char *)malloc(gs_print_buffer_length);
    if (gs_print_buffer == NULL) {
	print_stop_gs();
	return;
    }

    /* redirect stdio of Ghostscript */
    if (pipe(gs_print_pipe_stdin)) {
	gs_addmessf("Could not open pipe for stdin, errno=%d\n", errno);
	print_stop_gs();
	return;
    }
    if (pipe(gs_print_pipe_stdout)) {
	gs_addmessf("Could not open pipe for stdout, errno=%d\n", errno);
	print_stop_gs();
	return;
    }
    if (pipe(gs_print_pipe_stderr)) {
	gs_addmessf("Could not open pipe for stderr, errno=%d\n", errno);
	print_stop_gs();
	return;
    }
    show_print_message();

    printer.pid = fork();
    if (printer.pid == 0) {
	char atopt[MAXSTR];
	strcpy(atopt, "@");
	strcat(atopt, printer.optname);
	/* child */
	close(gs_print_pipe_stdin[1]);	/* close write handle */
	dup2(gs_print_pipe_stdin[0], 0);/* duplicate and make it stdin */
	close(gs_print_pipe_stdin[0]);	/* close original read handle */

	close(gs_print_pipe_stdout[0]);
	dup2(gs_print_pipe_stdout[1], 1);/* duplicate and make it stdout */
	close(gs_print_pipe_stdout[1]);

	close(gs_print_pipe_stderr[0]);
	dup2(gs_print_pipe_stderr[1], 2);/* duplicate and make it stderr */
	close(gs_print_pipe_stderr[1]);

	/* replace with Ghostscript */
	/* start Ghostscript */
{
char buf[MAXSTR];
FILE *f = fopen(printer.optname, "r");
while (fgets(buf, sizeof(buf), f))
    fprintf(stdout, "option: %s", buf);
fclose(f);
}
	nargv[0] = option.gsexe;
	nargv[1] = atopt;
	nargv[2] = dash_string;	/* read stdin */
	nargv[3] = dashc_string;
	nargv[4] = quit_string;
	nargv[5] = NULL;
	if (debug & DEBUG_GENERAL)
	    fprintf(stdout, "child: starting gs for printing\n");
	if (execvp(nargv[0], nargv) == -1) {
	    int j;
	    int err = errno;
	    /* write to stdout, which will be captured by GSview */
	    fprintf(stdout, "Failed to start Ghostscript process\n");
	    for (j=0; nargv[j]; j++)
		fprintf(stdout, " %s", nargv[j]);
	    fputc('\n', stdout);
	    fprintf(stdout, "  errno=%d\n", err);
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

	close(gs_print_pipe_stdin[0]);	/* close read handle */
	gs_print_pipe_stdin[0] = -1;
	/* make pipe non blocking */
	flags = fcntl(gs_print_pipe_stdin[1], F_GETFL, 0);
	if (fcntl(gs_print_pipe_stdin[1], F_SETFL, flags | O_NONBLOCK)) {
	    gs_addmessf("Could not set stdin pipe to non-blocking, errno=%d\n", errno);
	    print_stop_gs();
	    return;
        }

	close(gs_print_pipe_stdout[1]);	/* close write handle */
	gs_print_pipe_stdout[1] = -1;
	/* make pipe non blocking */
	flags = fcntl(gs_print_pipe_stdout[0], F_GETFL, 0);
	if (fcntl(gs_print_pipe_stdout[0], F_SETFL, flags | O_NONBLOCK)) {
	    gs_addmessf("Could not set stdout pipe to non-blocking, errno=%d\n", errno);
	    print_stop_gs();
	    return;
        }
	print_start_stdout();

	close(gs_print_pipe_stderr[1]);	/* close write handle */
	gs_print_pipe_stdout[1] = -1;
	/* make pipe non blocking */
	flags = fcntl(gs_print_pipe_stderr[0], F_GETFL, 0);
	if (fcntl(gs_print_pipe_stderr[0], F_SETFL, flags | O_NONBLOCK)) {
	    gs_addmessf("Could not set stderr pipe to non-blocking, errno=%d\n", errno);
	    print_stop_gs();
	    return;
        }
	print_start_stderr();

	/* Trigger async write */ 
	print_start_stdin();

	if (debug & DEBUG_GENERAL)
	    gs_addmess("print_start_gs: parent\n");
    } 
}

/* print a range of pages using a Ghostscript device */
void
gsview_print(BOOL convert)
{
    if (psfile.name[0] == '\0') {
	gserror(IDS_NOTOPEN, NULL, MB_ICONEXCLAMATION, SOUND_NOTOPEN);
	return;
    }

    if (option.print_method == PRINT_GDI)
	option.print_method = PRINT_GS;
    
    nHelpTopic = convert ? IDS_TOPICCONVERT : IDS_TOPICPRINT;
    if (print_dialog_box(convert) != IDOK)
	    return;

    if (!convert)
	option.print_reverse = psfile.page_list.reverse;

    if (!gsview_cprint(printer.psname, printer.optname, convert)) {
	gs_addmess("gsview_print: gsview_cprint failed\n");
	return;
    }
    if (debug & DEBUG_GENERAL)
	gs_addmessf("gsview_print: optname=%s psname=%s\n", printer.optname, printer.psname);

    /* start GS print */
    print_start_gs();

    return;
}

