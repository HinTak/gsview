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

#include "gvx.h"
#include "en/gvclang.h"

void add_line(GtkWidget *w, const char *str);
void online_reg(GtkWidget *w, gpointer data);
void modal_reg(GtkWidget *w, gpointer data);
int nag_dialog(void);
int reg_dialog(void);

BOOL write_registration(unsigned int reg_receipt, unsigned int reg_number, 
  char *reg_name)
{
    char profile[MAXSTR];
    const char *section = INISECTION;
    PROFILE *prf;
    prf = profile_open(szIniFile);
    profile_write_string(prf, section, "RegistrationName", reg_name);
    sprintf(profile, "%u", reg_receipt);
    profile_write_string(prf, section, "RegistrationReceipt", profile);
    sprintf(profile, "%u", (reg_number ^ 0xffff));
    profile_write_string(prf, section, "RegistrationNumber", profile);
    profile_close(prf);
    return TRUE;
}

BOOL read_registration(unsigned int *preg_receipt, unsigned int *preg_number,
  char *reg_name, int reg_len)
{
    unsigned int i;
    char profile[MAXSTR];
    const char *section = INISECTION;
    PROFILE *prf;
    prf = profile_open(szIniFile);
    profile_read_string(prf, section, "RegistrationReceipt", "", 
	    profile, sizeof(profile));
    if (sscanf(profile,"%u", &i) == 1)
	*preg_receipt = i;
    profile_read_string(prf, section, "RegistrationNumber", "", 
	    profile, sizeof(profile));
    if (sscanf(profile,"%u", &i) == 1)
	*preg_number = i ^ 0xffff;
    profile_read_string(prf, section, "RegistrationName", "", 
	    reg_name, reg_len);
    profile_close(prf);
    return TRUE;
}

void add_line(GtkWidget *w, const char *str)
{
    GtkWidget *label = label = gtk_label_new(str);
    GtkWidget *align = gtk_alignment_new(0, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), label);
    gtk_box_pack_start(GTK_BOX(w), align, FALSE, FALSE, 0);
    gtk_widget_show(label);
    gtk_widget_show(align);
}

void online_reg(GtkWidget *w, gpointer data)
{
    if (debug & DEBUG_GENERAL)
	gs_addmess("online_reg:\n");
    exec_program("netscape", "http://www.ghostgum.com.au/");
}

void modal_reg(GtkWidget *w, gpointer data)
{
    if (debug & DEBUG_GENERAL)
	gs_addmess("modal_reg:\n");
    *((int *)data) = NAG_REGISTER;
    gtk_main_quit();
}

int nag_dialog(void)
{
    GtkWidget *window;		/* main dialog window */
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *button_ok = NULL;
    GtkWidget *button_reg = NULL;
    GtkWidget *button_help = NULL;
    char buf[MAXSTR];
    int rc = 0;

    window=gtk_window_new(GTK_WINDOW_DIALOG);

    gtk_window_set_title(GTK_WINDOW(window), get_string(IDS_AAGSVIEWREG));
    gtk_signal_connect(GTK_OBJECT(window), "delete-event",
                       GTK_SIGNAL_FUNC(modal_delete), &rc);
    
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show(vbox);
   
    add_line(vbox, get_string(IDS_AANAG1));
    add_line(vbox, get_string(IDS_AANAG2));
    add_line(vbox, get_string(IDS_AANAG3));
    add_line(vbox, get_string(IDS_AANAG4));
    add_line(vbox, get_string(IDS_AANAG5));
    add_line(vbox, "");

    sprintf(buf, "%s  %s", get_string(IDS_AAREGTOC), registration_name);
    add_line(vbox, buf);
    strcpy(buf, get_string(IDS_AANUMBERC));
    if (registration_receipt != 0) {
	sprintf(buf, "%s  %u", get_string(IDS_AANUMBERC), registration_receipt);
    }
    add_line(vbox, buf);
    add_line(vbox, "");

    /* Create and place OK button at the bottom */
    hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
    gtk_widget_show(hbox);

    button_ok = gtk_button_new_with_label(get_string(IDS_AAOK));
    gtk_box_pack_start(GTK_BOX(hbox), button_ok, TRUE, TRUE, 10);
    gtk_signal_connect(GTK_OBJECT(button_ok), "clicked",
			  GTK_SIGNAL_FUNC(modal_ok), &rc);
    gtk_widget_show(button_ok);

    button_reg = gtk_button_new_with_label(get_string(IDS_AAREGISTERNOW));
    gtk_box_pack_start(GTK_BOX(hbox), button_reg, TRUE, TRUE, 10);
    gtk_signal_connect(GTK_OBJECT(button_reg), "clicked",
			      GTK_SIGNAL_FUNC(modal_reg), &rc);
    gtk_widget_show(button_reg);

    button_help = gtk_button_new_with_label(get_string(IDS_AAHELP));
    gtk_box_pack_start(GTK_BOX(hbox), button_help, TRUE, TRUE, 10);
    gtk_signal_connect(GTK_OBJECT(button_help), "clicked",
			      GTK_SIGNAL_FUNC(modal_help), &rc);
    gtk_widget_show(button_help);

    /* show dialog and wait for OK or NAG_REGISTER */
    gtk_window_set_focus(GTK_WINDOW(window), button_ok);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_show(window);
    gtk_main();
 
    gtk_widget_destroy(window);
    return rc;
}

int reg_dialog(void)
{
    unsigned int reg_num;
    unsigned int reg_receipt;
    char *reg_name;
    GtkWidget *window;		/* main dialog window */
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *table;
    GtkWidget *con;
    GtkWidget *label;
    GtkWidget *entry_name;
    GtkWidget *entry_receipt;
    GtkWidget *entry_num;
    GtkWidget *button_online;
    GtkWidget *button_ok = NULL;
    GtkWidget *button_cancel = NULL;
    GtkWidget *button_help = NULL;
    int rc = 0;

    window=gtk_window_new(GTK_WINDOW_DIALOG);

    gtk_window_set_title(GTK_WINDOW(window), get_string(IDS_AAGSVIEWREG));
    gtk_signal_connect(GTK_OBJECT(window), "delete-event",
                       GTK_SIGNAL_FUNC(modal_delete), &rc);
    
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show(vbox);
   
    add_line(vbox, get_string(IDS_AAREG1));

    table = gtk_table_new(3, 5, FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 5);
    gtk_widget_show(table);

    gtk_table_set_row_spacing(GTK_TABLE(table), 0, 5);
    label = gtk_label_new(get_string(IDS_AAREGTOC));
    gtk_widget_show(label);
    con = gtk_alignment_new(1, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(con), label);
    gtk_widget_show(con);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 0, 1, 0, 1);
    label = gtk_label_new("  ");
    gtk_widget_show(label);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 1, 2, 0, 1);
    entry_name = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_name), 255);
    gtk_entry_set_text(GTK_ENTRY(entry_name), "");
    gtk_widget_show(entry_name);
    gtk_table_attach_defaults(GTK_TABLE(table), entry_name, 2, 5, 0, 1);
    gtk_signal_connect(GTK_OBJECT(entry_name), "activate",
                              GTK_SIGNAL_FUNC(modal_ok), &rc);

    gtk_table_set_row_spacing(GTK_TABLE(table), 1, 5);
    label = gtk_label_new(get_string(IDS_AANUMBERC));
    gtk_widget_show(label);
    con = gtk_alignment_new(1, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(con), label);
    gtk_widget_show(con);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 0, 1, 1, 2);

    entry_receipt = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_receipt), 255);
    gtk_entry_set_text(GTK_ENTRY(entry_receipt), "");
    gtk_widget_show(entry_receipt);
    gtk_signal_connect(GTK_OBJECT(entry_receipt), "size-request",
		  GTK_SIGNAL_FUNC(width_percent), (gpointer)50);
    con = gtk_alignment_new(0, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(con), entry_receipt);
    gtk_widget_show(con);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 2, 3, 1, 2); 
    gtk_signal_connect(GTK_OBJECT(entry_receipt), "activate",
                              GTK_SIGNAL_FUNC(modal_ok), &rc);

    label = gtk_label_new(" - ");
    gtk_widget_show(label);
    con = gtk_alignment_new(0, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(con), label);
    gtk_widget_show(con);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 3, 4, 1, 2);

    entry_num = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_num), 255);
    gtk_entry_set_text(GTK_ENTRY(entry_num), "");
    gtk_widget_show(entry_num);
    gtk_signal_connect(GTK_OBJECT(entry_num), "size-request",
			  GTK_SIGNAL_FUNC(width_percent), (gpointer)50);
    con = gtk_alignment_new(0, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(con), entry_num);
    gtk_widget_show(con);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 4, 5, 1, 2);
    gtk_signal_connect(GTK_OBJECT(entry_num), "activate",
                              GTK_SIGNAL_FUNC(modal_ok), &rc);

    button_online = gtk_button_new_with_label(get_string(IDS_AAONLINEREG));
    gtk_table_attach_defaults(GTK_TABLE(table), button_online, 2, 5, 2, 3); 
    gtk_signal_connect(GTK_OBJECT(button_online), "clicked",
			  GTK_SIGNAL_FUNC(online_reg), &rc);
    gtk_widget_show(button_online);

    /* Create and place OK button at the bottom */
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

    button_help = gtk_button_new_with_label(get_string(IDS_AAHELP));
    gtk_box_pack_start(GTK_BOX(hbox), button_help, TRUE, TRUE, 10);
    gtk_signal_connect(GTK_OBJECT(button_help), "clicked",
			      GTK_SIGNAL_FUNC(modal_help), &rc);
    gtk_widget_show(button_help);

    /* show dialog and wait for OK or Cancel */
    gtk_window_set_focus(GTK_WINDOW(window), entry_name);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_show(window);
    gtk_main();

    /* get settings here */
    reg_receipt = atoi(gtk_entry_get_text(GTK_ENTRY(entry_receipt)));
    reg_num = atoi(gtk_entry_get_text(GTK_ENTRY(entry_num)));
    reg_name = gtk_entry_get_text(GTK_ENTRY(entry_name));

    if (rc == IDOK) {
	if ((reg_receipt != 0) && 
	    (reg_num == make_reg(reg_receipt)) &&
	    (strlen(reg_name) > 0) ) {
	    strncpy(registration_name, reg_name, sizeof(registration_name));
	    registration_receipt = reg_receipt;
	    write_registration(registration_receipt,
		reg_num, registration_name);
	}
	else {
	    message_box(get_string(IDS_INVALIDREG), 0);
	}
    }
 
    gtk_widget_destroy(window);
    return rc;
}

BOOL registration_nag(void)
{
    nHelpTopic = IDS_TOPICREG;
    if (nag_dialog() == NAG_REGISTER) {
	if (reg_dialog() == IDOK)
	    return TRUE;
    }
    return FALSE;
}
