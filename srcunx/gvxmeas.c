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

/* gvxmeas.cpp */

#include "gvx.h"
#include <math.h>
#include "en/gvclang.h"

float measure_lastx;
float measure_lasty;

static const char measure_fmt[] = "%.5g";

static const char *ctmfmt = "%.5g" ;

extern float fthreshold;
void measure_transform_point(float x, float y, float *px, float *py);
#define degrees(x) (x * 180.0 / 3.14159265358979)

typedef struct tagCTMTEXT {
    char xx[32];
    char xy[32];
    char yx[32];
    char yy[32];
    char tx[32];
    char ty[32];
} CTMTEXT;

typedef struct tagCTMDLG {
    GtkWidget *window;
    GtkWidget *unit_pt;
    GtkWidget *unit_mm;
    GtkWidget *unit_inch;
    GtkWidget *unit_custom;

    GtkWidget *ctmxx; 
    GtkWidget *ctmxy; 
    GtkWidget *ctmyx; 
    GtkWidget *ctmyy; 
    GtkWidget *ctmtx; 
    GtkWidget *ctmty; 

    GtkWidget *tx;
    GtkWidget *ty;
    GtkWidget *translate;
    GtkWidget *angle;
    GtkWidget *rotate;
    GtkWidget *sx;
    GtkWidget *sy;
    GtkWidget *scale;

    GtkWidget *initmatrix;
    GtkWidget *invertmatrix;

    GtkWidget *button_ok;
    GtkWidget *button_cancel;
    GtkWidget *button_help;

    MEASURE measure;
    MATRIX ctm;
} CTMDLG;


void measure_dialog_unitx(void);
void calc_ctm(GtkWidget *w, gpointer data);
gint measure_delete(GtkWidget *widget, GdkEvent *event, gpointer data);
void show_calc_dialog(void);
void update_ctm_text(CTMTEXT *t, MATRIX *ctm);
void update_dialog_ctm(CTMDLG *dlg, MATRIX *ctm);
void dialog_put_float(GtkWidget *entry, float fx);
BOOL dialog_get_float_error(CTMDLG *dlg, GtkWidget *entry, float *fres,
   BOOL error);
BOOL dialog_get_float(CTMDLG *dlg, GtkWidget *entry, float *fres);
BOOL dialog_get_ctm(CTMDLG *dlg, MATRIX *ctm, BOOL error);
void calc_enable_custom(CTMDLG *dlg, BOOL enable);
void calc_units(CTMDLG *dlg, int unit);
void calc_pts(GtkWidget *w, gpointer data);
void calc_mm(GtkWidget *w, gpointer data);
void calc_inch(GtkWidget *w, gpointer data);
void calc_custom(GtkWidget *w, gpointer data);
void calc_ro(GtkWidget *w, gpointer data);
void calc_tr(GtkWidget *w, gpointer data);
void calc_sc(GtkWidget *w, gpointer data);
void calc_ini(GtkWidget *w, gpointer data);
void calc_inv(GtkWidget *w, gpointer data);

GtkWidget *measure;		/* main dialog */
GtkWidget *measure_unit;	/* labels */
GtkWidget *measure_startx;
GtkWidget *measure_starty;
GtkWidget *measure_finishx;
GtkWidget *measure_finishy;
GtkWidget *measure_deltax;
GtkWidget *measure_deltay;
GtkWidget *measure_radius;
GtkWidget *measure_angle;

void
measure_update_last(void)
{
char buf[64];
float thisx, thisy;
    if (measure == NULL)
	return;
    measure_transform_point(measure_lastx, measure_lasty, &thisx, &thisy);
    if (fabs(thisx) < fthreshold)
        thisx = 0.0;
    if (fabs(thisy) < fthreshold)
        thisy = 0.0;
    sprintf(buf, measure_fmt, thisx);
    gtk_label_set_text(GTK_LABEL(measure_startx), buf);
    sprintf(buf, measure_fmt, thisx);
    gtk_label_set_text(GTK_LABEL(measure_starty), buf);
}

void
measure_setpoint(float x, float y)
{
    measure_lastx = x;
    measure_lasty = y;
    measure_update_last();
    measure_paint(x, y);
}

/* This is called from motion_notify_event */
void
measure_paint(float x, float y)
{
float lastx, lasty;
float thisx, thisy;
float deltax, deltay;
float radius, angle;
char buf[64];
    if (measure == NULL)
	return;
    measure_transform_point(x, y, &thisx, &thisy);
    if (fabs(thisx) < fthreshold)
        thisx = 0.0;
    if (fabs(thisy) < fthreshold)
        thisy = 0.0;
    measure_transform_point(measure_lastx, measure_lasty, &lastx, &lasty);
    if (fabs(lastx) < fthreshold)
        lastx = 0.0;
    if (fabs(lasty) < fthreshold)
        lasty = 0.0;
    deltax = thisx - lastx;
    deltay = thisy - lasty;
    if (fabs(deltax) < fthreshold)
        deltax = 0.0;
    if (fabs(deltay) < fthreshold)
        deltay = 0.0;
    radius = sqrt(deltax*deltax + deltay*deltay);
    if ((deltax == 0.0) && (deltay == 0.0))
	angle = 0.0;
    else
	angle = degrees( atan2(deltay, deltax) );

    sprintf(buf, measure_fmt, thisx);
    gtk_label_set_text(GTK_LABEL(measure_finishx), buf);
    sprintf(buf, measure_fmt, thisy);
    gtk_label_set_text(GTK_LABEL(measure_finishy), buf);
    sprintf(buf, measure_fmt, deltax);
    gtk_label_set_text(GTK_LABEL(measure_deltax), buf);
    sprintf(buf, measure_fmt, deltay);
    gtk_label_set_text(GTK_LABEL(measure_deltay), buf);
    sprintf(buf, measure_fmt, radius);
    gtk_label_set_text(GTK_LABEL(measure_radius), buf);
    sprintf(buf, "%.2f", angle);
    gtk_label_set_text(GTK_LABEL(measure_angle), buf);
}


void
measure_dialog_unitx(void)
{
    if (measure_unit)
	gtk_label_set_text(GTK_LABEL(measure_unit), 
	    get_string(IDS_UNITNAME + option.measure.unit - IDM_UNITPT)); 
}

void calc_ctm(GtkWidget *w, gpointer data)
{
    show_calc_dialog();
    measure_dialog_unitx();
    measure_update_last();
}

gint measure_delete(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    measure_close();
    return TRUE;
}

void measure_close(void)
{
    if (measure) {
	if (debug & DEBUG_GENERAL)
	    gs_addmess("measure_close:\n");
	gtk_widget_destroy(measure);
	measure = NULL;
	measure_startx = NULL;
	measure_starty = NULL;
	measure_finishx = NULL;
	measure_finishy = NULL;
	measure_deltax = NULL;
	measure_deltay = NULL;
	measure_radius = NULL;
	measure_angle = NULL;
    }
}

void 
measure_show(void)
{
    /* create measure dialog */
    GtkWidget *table;
    GtkWidget *con;
    GtkWidget *label;
    GtkWidget *button_change;
    int y = 0;

    if (measure != NULL) {
	if (debug & DEBUG_GENERAL)
	    gs_addmess("measure_show: showing dialog\n");
	gtk_widget_show(measure);
	return;
    }

    measure_lastx = 0.0;
    measure_lasty = 0.0;

    measure=gtk_window_new(GTK_WINDOW_TOPLEVEL); gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(measure), 10);
    gtk_window_set_title(GTK_WINDOW(measure), get_string(IDS_AAMEASURE));

    table = gtk_table_new(4, 6, FALSE);
    gtk_container_add(GTK_CONTAINER(measure), table);

    /* row 0 */
    label = gtk_label_new(get_string(IDS_AAUNITC));
    gtk_widget_show(label);
    con = gtk_alignment_new(0, 0, 0, 0);
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), label);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 0, 1, y, y+1);

    measure_unit = gtk_label_new("");
    measure_dialog_unitx();
    gtk_widget_show(measure_unit);
    con = gtk_alignment_new(0, 0, 0, 0);
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), measure_unit);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 1, 2, y, y+1);

    button_change = gtk_button_new_with_label(get_string(IDS_AACHANGE));
    gtk_widget_show(button_change);
    gtk_table_attach_defaults(GTK_TABLE(table), button_change, 2, 4, y, y+1);
    y++;

    /* row 1, blank line and set minimum spacing */
    label = gtk_label_new("          ");
    gtk_widget_show(label);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, y, y+1);
    label = gtk_label_new("             ");
    gtk_widget_show(label);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 1, 2, y, y+1);
    label = gtk_label_new("             ");
    gtk_widget_show(label);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 2, 3, y, y+1);
    y++;


    /* row 2, start x y */
    label = gtk_label_new(get_string(IDS_AASTARTC));
    gtk_widget_show(label);
    con = gtk_alignment_new(0, 0, 0, 0);
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), label);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 0, 1, y, y+1);

    measure_startx = gtk_label_new("x");
    gtk_widget_show(measure_startx);
    con = gtk_alignment_new(1, 0, 0, 0);
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), measure_startx);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 1, 2, y, y+1);

    measure_starty = gtk_label_new("y");
    gtk_widget_show(measure_starty);
    con = gtk_alignment_new(1, 0, 0, 0);
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), measure_starty);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 2, 3, y, y+1);
    y++;

    /* row 3, finish x y */
    label = gtk_label_new(get_string(IDS_AAFINISHC));
    gtk_widget_show(label);
    con = gtk_alignment_new(0, 0, 0, 0);
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), label);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 0, 1, y, y+1);

    measure_finishx = gtk_label_new("x");
    gtk_widget_show(measure_finishx);
    con = gtk_alignment_new(1, 0, 0, 0);
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), measure_finishx);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 1, 2, y, y+1);

    measure_finishy = gtk_label_new("y");
    gtk_widget_show(measure_finishy);
    con = gtk_alignment_new(1, 0, 0, 0);
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), measure_finishy);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 2, 3, y, y+1);
    y++;

    /* row 4, delta x y */
    label = gtk_label_new(get_string(IDS_AADELTAC));
    gtk_widget_show(label);
    con = gtk_alignment_new(0, 0, 0, 0);
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), label);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 0, 1, y, y+1);

    measure_deltax = gtk_label_new("x");
    gtk_widget_show(measure_deltax);
    con = gtk_alignment_new(1, 0, 0, 0);
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), measure_deltax);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 1, 2, y, y+1);

    measure_deltay = gtk_label_new("y");
    gtk_widget_show(measure_deltay);
    con = gtk_alignment_new(1, 0, 0, 0);
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), measure_deltay);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 2, 3, y, y+1);
    y++;


    /* row 5, len radius angle */
    label = gtk_label_new(get_string(IDS_AALENGTHC));
    gtk_widget_show(label);
    con = gtk_alignment_new(0, 0, 0, 0);
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), label);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 0, 1, y, y+1);

    measure_radius = gtk_label_new("r");
    gtk_widget_show(measure_radius);
    con = gtk_alignment_new(1, 0, 0, 0);
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), measure_radius);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 1, 2, y, y+1);

    measure_angle = gtk_label_new("th");
    gtk_widget_show(measure_angle);
    con = gtk_alignment_new(1, 0, 0, 0);
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), measure_angle);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 2, 3, y, y+1);

    label = gtk_label_new("\260");
    gtk_widget_show(label);
    con = gtk_alignment_new(0, 0, 0, 0);
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), label);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 3, 4, y, y+1);

    gtk_widget_show(GTK_WIDGET(table));
    gtk_signal_connect(GTK_OBJECT(measure), "delete-event",
	GTK_SIGNAL_FUNC(measure_delete), NULL);
    gtk_signal_connect(GTK_OBJECT(button_change), "clicked",
	GTK_SIGNAL_FUNC(calc_ctm), NULL);

    measure_setpoint(0.0, 0.0);
    gtk_widget_show(GTK_WIDGET(measure));

    if (debug & DEBUG_GENERAL)
	gs_addmessf("measure_show: created dialog\n");
}


void update_ctm_text(CTMTEXT *t, MATRIX *ctm)
{
  /* round small values to zero */
  if (fabs(ctm->xx) < fthreshold)
    ctm->xx = 0.0;
  if (fabs(ctm->xy) < fthreshold)
    ctm->xy = 0.0;
  if (fabs(ctm->yx) < fthreshold)
    ctm->yx = 0.0;
  if (fabs(ctm->yy) < fthreshold)
    ctm->yy = 0.0;
  if (fabs(ctm->tx) < fthreshold)
    ctm->tx = 0.0;
  if (fabs(ctm->ty) < fthreshold)
    ctm->ty = 0.0;

  sprintf(t->xx, ctmfmt, ctm->xx);
  sprintf(t->xy, ctmfmt, ctm->xy);
  sprintf(t->yx, ctmfmt, ctm->yx);
  sprintf(t->yy, ctmfmt, ctm->yy);
  sprintf(t->tx, ctmfmt, ctm->tx);
  sprintf(t->ty, ctmfmt, ctm->ty);
}

void update_dialog_ctm(CTMDLG *dlg, MATRIX *ctm)
{
    CTMTEXT t;
    update_ctm_text(&t, ctm);
    gtk_entry_set_text(GTK_ENTRY(dlg->ctmxx), t.xx);
    gtk_entry_set_text(GTK_ENTRY(dlg->ctmxy), t.xy);
    gtk_entry_set_text(GTK_ENTRY(dlg->ctmyx), t.yx);
    gtk_entry_set_text(GTK_ENTRY(dlg->ctmyy), t.yy);
    gtk_entry_set_text(GTK_ENTRY(dlg->ctmtx), t.tx);
    gtk_entry_set_text(GTK_ENTRY(dlg->ctmty), t.ty);
}

void dialog_put_float(GtkWidget *entry, float fx)
{
    char buf[64];
    sprintf(buf, ctmfmt, fx);
    gtk_entry_set_text(GTK_ENTRY(entry), buf);
}


BOOL dialog_get_float_error(CTMDLG *dlg, GtkWidget *entry, float *fres, 
   BOOL error)
{
  float ftemp ;
  char *s;

  s = gtk_entry_get_text(GTK_ENTRY(entry));
  if ((1 > sscanf(s,"%g",&ftemp)) && error)
  {
    gserror(IDS_INVALIDNUMBER, NULL, MB_ICONEXCLAMATION, SOUND_NONUMBER) ;
    gtk_window_set_focus(GTK_WINDOW(dlg->window), entry);
    gtk_editable_select_region(GTK_EDITABLE(entry), 0, -1);
  }
  else
  {
    *fres = ftemp ;
    return TRUE ;
  }
  return FALSE;
}

BOOL dialog_get_float(CTMDLG *dlg, GtkWidget *entry, float *fres)
{
  return dialog_get_float_error(dlg, entry, fres, TRUE);
}

BOOL dialog_get_ctm(CTMDLG *dlg, MATRIX *ctm, BOOL error)
{
BOOL result = TRUE;
   if (result)
       result = dialog_get_float_error(dlg, dlg->ctmxx, &ctm->xx, error);
   if (result)
       result = dialog_get_float_error(dlg, dlg->ctmxy, &ctm->xy, error);
   if (result)
       result = dialog_get_float_error(dlg, dlg->ctmyx, &ctm->yx, error);
   if (result)
       result = dialog_get_float_error(dlg, dlg->ctmyy, &ctm->yy, error);
   if (result)
       result = dialog_get_float_error(dlg, dlg->ctmtx, &ctm->tx, error);
   if (result)
       result = dialog_get_float_error(dlg, dlg->ctmty, &ctm->ty, error);
   return result;
}

void calc_enable_custom(CTMDLG *dlg, BOOL enable)
{
    gtk_widget_set_sensitive(dlg->tx, enable);
    gtk_widget_set_sensitive(dlg->ty, enable);
    gtk_widget_set_sensitive(dlg->translate, enable);
    gtk_widget_set_sensitive(dlg->angle, enable);
    gtk_widget_set_sensitive(dlg->rotate, enable);
    gtk_widget_set_sensitive(dlg->sx, enable);
    gtk_widget_set_sensitive(dlg->sy, enable);
    gtk_widget_set_sensitive(dlg->scale, enable);
    gtk_widget_set_sensitive(dlg->initmatrix, enable);
    gtk_widget_set_sensitive(dlg->invertmatrix, enable);
}

void calc_units(CTMDLG *dlg, int unit)
{
    dlg->measure.unit = unit;
    if (unit != IDM_UNITCUSTOM) {
	matrix_set_unit(&dlg->ctm, unit);
	update_dialog_ctm(dlg, &dlg->ctm) ;
    }
    calc_enable_custom(dlg, unit == IDM_UNITCUSTOM);
}

void calc_pts(GtkWidget *w, gpointer data)
{
    calc_units((CTMDLG *)data, IDM_UNITPT);
}

void calc_mm(GtkWidget *w, gpointer data)
{
    calc_units((CTMDLG *)data, IDM_UNITMM);
}

void calc_inch(GtkWidget *w, gpointer data)
{
    calc_units((CTMDLG *)data, IDM_UNITINCH);
}

void calc_custom(GtkWidget *w, gpointer data)
{
    calc_units((CTMDLG *)data, IDM_UNITCUSTOM);
}

void calc_ro(GtkWidget *w, gpointer data)
{
    float angle;
    CTMDLG *dlg = (CTMDLG *)data;
    if (dialog_get_ctm(dlg, &dlg->ctm, TRUE) 
	&& dialog_get_float(dlg, dlg->angle, &angle)) {
	matrix_rotate(&dlg->ctm, angle, &dlg->ctm) ;
	update_dialog_ctm(dlg, &dlg->ctm) ;
    }
}

void calc_tr(GtkWidget *w, gpointer data)
{
    float tx, ty;
    CTMDLG *dlg = (CTMDLG *)data;
    if (dialog_get_ctm(dlg, &dlg->ctm, TRUE) 
      && dialog_get_float(dlg, dlg->tx, &tx)
      && dialog_get_float(dlg, dlg->ty, &ty)) {
	matrix_translate(&dlg->ctm, tx, ty, &dlg->ctm);
	update_dialog_ctm(dlg, &dlg->ctm) ;
    }
}

void calc_sc(GtkWidget *w, gpointer data)
{
    float sx, sy;
    CTMDLG *dlg = (CTMDLG *)data;
    if (dialog_get_ctm(dlg, &dlg->ctm, TRUE) 
     && dialog_get_float(dlg, dlg->sx, &sx)
     && dialog_get_float(dlg, dlg->sy, &sy)) {
	matrix_scale(&dlg->ctm, sx, sy, &dlg->ctm);
	update_dialog_ctm(dlg, &dlg->ctm) ;
    }
}

void calc_ini(GtkWidget *w, gpointer data)
{
    CTMDLG *dlg = (CTMDLG *)data;
    matrix_set_unit(&dlg->ctm, IDM_UNITPT) ;
    update_dialog_ctm(dlg, &dlg->ctm) ;
}

void calc_inv(GtkWidget *w, gpointer data)
{
    CTMDLG *dlg = (CTMDLG *)data;
    if (!dialog_get_ctm(dlg, &dlg->ctm, TRUE))
	return;
    if (-1 == matrix_invert(&dlg->ctm, &dlg->ctm))
	gserror(IDS_CANTINVERT, NULL, MB_ICONEXCLAMATION, SOUND_NONUMBER) ;
    else
	update_dialog_ctm(dlg, &dlg->ctm) ;
}

void show_calc_dialog(void)
{
    int rc;
    CTMDLG dlg;
    GtkWidget *hbox_unit;
    GtkWidget *vbox;
    GtkWidget *table;
    GtkWidget *label;
    GtkWidget *con;
    GtkWidget *hbox;
    int y;

    dlg.window=gtk_window_new(GTK_WINDOW_TOPLEVEL); gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(dlg.window), 10);
    gtk_window_set_title(GTK_WINDOW(dlg.window), get_string(IDS_AACALCXFORM));
    gtk_signal_connect(GTK_OBJECT(dlg.window), "delete-event",
                       GTK_SIGNAL_FUNC(modal_delete), &rc);

    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(dlg.window), vbox);
    gtk_widget_show(vbox);

    /* radio buttons for units */
    hbox_unit = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_unit, FALSE, FALSE, 5);
    gtk_widget_show(hbox_unit);

    dlg.unit_pt = gtk_radio_button_new_with_label(NULL, get_string(IDS_UNITPT));
    gtk_box_pack_start(GTK_BOX(hbox_unit), dlg.unit_pt, TRUE, FALSE, 5);
    gtk_signal_connect(GTK_OBJECT(dlg.unit_pt), "clicked",
			  GTK_SIGNAL_FUNC(calc_pts), &dlg);
    gtk_widget_show(dlg.unit_pt);

    dlg.unit_mm = gtk_radio_button_new_with_label_from_widget(
	GTK_RADIO_BUTTON(dlg.unit_pt), get_string(IDS_UNITMM));
    gtk_box_pack_start(GTK_BOX(hbox_unit), dlg.unit_mm, TRUE, FALSE, 5);
    gtk_signal_connect(GTK_OBJECT(dlg.unit_mm), "clicked",
			  GTK_SIGNAL_FUNC(calc_mm), &dlg);
    gtk_widget_show(dlg.unit_mm);

    dlg.unit_inch = gtk_radio_button_new_with_label_from_widget(
	GTK_RADIO_BUTTON(dlg.unit_pt), get_string(IDS_UNITINCH));
    gtk_box_pack_start(GTK_BOX(hbox_unit), dlg.unit_inch, TRUE, FALSE, 5);
    gtk_signal_connect(GTK_OBJECT(dlg.unit_inch), "clicked",
			  GTK_SIGNAL_FUNC(calc_inch), &dlg);
    gtk_widget_show(dlg.unit_inch);

    dlg.unit_custom = gtk_radio_button_new_with_label_from_widget(
	GTK_RADIO_BUTTON(dlg.unit_pt), get_string(IDS_UNITCUSTOM));
    gtk_box_pack_start(GTK_BOX(hbox_unit), dlg.unit_custom, TRUE, FALSE, 5);
    gtk_signal_connect(GTK_OBJECT(dlg.unit_custom), "clicked",
			  GTK_SIGNAL_FUNC(calc_custom), &dlg);
    gtk_widget_show(dlg.unit_custom);


    /* table for CTM etc */
     y=0;
    table = gtk_table_new(3, 8, FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 5);
    gtk_widget_show(table);
    gtk_table_set_col_spacing(GTK_TABLE(table), 0, 5);
    gtk_table_set_col_spacing(GTK_TABLE(table), 1, 10);

    label = gtk_label_new("CTM");
    gtk_widget_show(label);
    con = gtk_alignment_new(0, 0, 0, 0);
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), label);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 0, 1, y, y+1);
    y++;

    dlg.ctmxx = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(dlg.ctmxx), 32);
    gtk_table_attach_defaults(GTK_TABLE(table), dlg.ctmxx, 0, 1, y, y+1);
    gtk_signal_connect(GTK_OBJECT(dlg.ctmxx), "size-request",
			  GTK_SIGNAL_FUNC(width_percent), (gpointer)50);
    gtk_widget_show(dlg.ctmxx);

    dlg.ctmxy = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(dlg.ctmxy), 32);
    gtk_table_attach_defaults(GTK_TABLE(table), dlg.ctmxy, 1, 2, y, y+1);
    gtk_signal_connect(GTK_OBJECT(dlg.ctmxy), "size-request",
			  GTK_SIGNAL_FUNC(width_percent), (gpointer)50);
    gtk_widget_show(dlg.ctmxy);

    dlg.ctmtx = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(dlg.ctmtx), 32);
    gtk_table_attach_defaults(GTK_TABLE(table), dlg.ctmtx, 2, 3, y, y+1);
    gtk_signal_connect(GTK_OBJECT(dlg.ctmtx), "size-request",
			  GTK_SIGNAL_FUNC(width_percent), (gpointer)50);
    gtk_widget_show(dlg.ctmtx);
    y++;

    dlg.ctmyx = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(dlg.ctmyx), 32);
    gtk_table_attach_defaults(GTK_TABLE(table), dlg.ctmyx, 0, 1, y, y+1);
    gtk_signal_connect(GTK_OBJECT(dlg.ctmyx), "size-request",
			  GTK_SIGNAL_FUNC(width_percent), (gpointer)50);
    gtk_widget_show(dlg.ctmyx);

    dlg.ctmyy = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(dlg.ctmyy), 32);
    gtk_table_attach_defaults(GTK_TABLE(table), dlg.ctmyy, 1, 2, y, y+1);
    gtk_signal_connect(GTK_OBJECT(dlg.ctmyy), "size-request",
			  GTK_SIGNAL_FUNC(width_percent), (gpointer)50);
    gtk_widget_show(dlg.ctmyy);

    dlg.ctmty = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(dlg.ctmty), 32);
    gtk_table_attach_defaults(GTK_TABLE(table), dlg.ctmty, 2, 3, y, y+1);
    gtk_signal_connect(GTK_OBJECT(dlg.ctmty), "size-request",
			  GTK_SIGNAL_FUNC(width_percent), (gpointer)50);
    gtk_widget_show(dlg.ctmty);
    y++;

    label = gtk_label_new(get_string(IDS_AACUSTOM));
    gtk_widget_show(label);
    con = gtk_alignment_new(0, 0, 0, 0);
    gtk_widget_show(con);
    gtk_container_add(GTK_CONTAINER(con), label);
    gtk_table_attach_defaults(GTK_TABLE(table), con, 0, 1, y, y+1);
    gtk_table_set_row_spacing(GTK_TABLE(table), y-1, 10);
    y++;

    dlg.tx = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(dlg.tx), 32);
    gtk_table_attach_defaults(GTK_TABLE(table), dlg.tx, 0, 1, y, y+1);
    gtk_signal_connect(GTK_OBJECT(dlg.tx), "size-request",
			  GTK_SIGNAL_FUNC(width_percent), (gpointer)50);
    gtk_widget_show(dlg.tx);

    dlg.ty = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(dlg.ty), 32);
    gtk_table_attach_defaults(GTK_TABLE(table), dlg.ty, 1, 2, y, y+1);
    gtk_signal_connect(GTK_OBJECT(dlg.ty), "size-request",
			  GTK_SIGNAL_FUNC(width_percent), (gpointer)50);
    gtk_widget_show(dlg.ty);

    dlg.translate = gtk_button_new_with_label(get_string(IDS_AATRANSLATE));
    gtk_table_attach_defaults(GTK_TABLE(table), dlg.translate, 2, 3, y, y+1);
    gtk_signal_connect(GTK_OBJECT(dlg.translate), "clicked",
                              GTK_SIGNAL_FUNC(calc_tr), &dlg);
    gtk_widget_show(dlg.translate);
    y++;

    dlg.angle = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(dlg.angle), 32);
    gtk_table_attach_defaults(GTK_TABLE(table), dlg.angle, 1, 2, y, y+1);
    gtk_signal_connect(GTK_OBJECT(dlg.angle), "size-request",
			  GTK_SIGNAL_FUNC(width_percent), (gpointer)50);
    gtk_widget_show(dlg.angle);

    dlg.rotate = gtk_button_new_with_label(get_string(IDS_AAROTATE));
    gtk_table_attach_defaults(GTK_TABLE(table), dlg.rotate, 2, 3, y, y+1);
    gtk_signal_connect(GTK_OBJECT(dlg.rotate), "clicked",
                              GTK_SIGNAL_FUNC(calc_ro), &dlg);
    gtk_widget_show(dlg.rotate);
    y++;

    dlg.sx = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(dlg.sx), 32);
    gtk_table_attach_defaults(GTK_TABLE(table), dlg.sx, 0, 1, y, y+1);
    gtk_signal_connect(GTK_OBJECT(dlg.sx), "size-request",
			  GTK_SIGNAL_FUNC(width_percent), (gpointer)50);
    gtk_widget_show(dlg.sx);

    dlg.sy = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(dlg.sy), 32);
    gtk_table_attach_defaults(GTK_TABLE(table), dlg.sy, 1, 2, y, y+1);
    gtk_signal_connect(GTK_OBJECT(dlg.sy), "size-request",
			  GTK_SIGNAL_FUNC(width_percent), (gpointer)50);
    gtk_widget_show(dlg.sy);

    dlg.scale = gtk_button_new_with_label(get_string(IDS_AASCALE));
    gtk_table_attach_defaults(GTK_TABLE(table), dlg.scale, 2, 3, y, y+1);
    gtk_signal_connect(GTK_OBJECT(dlg.scale), "clicked",
                              GTK_SIGNAL_FUNC(calc_sc), &dlg);
    gtk_widget_show(dlg.scale);
    y++;

    dlg.initmatrix = gtk_button_new_with_label(get_string(IDS_AAINITMATRIX));
    gtk_table_attach_defaults(GTK_TABLE(table), dlg.initmatrix, 0, 1, y, y+1);
    gtk_signal_connect(GTK_OBJECT(dlg.initmatrix), "clicked",
                              GTK_SIGNAL_FUNC(calc_ini), &dlg);
    gtk_widget_show(dlg.initmatrix);

    dlg.invertmatrix = gtk_button_new_with_label(get_string(IDS_AAINVERTMATRIX));
    gtk_table_attach_defaults(GTK_TABLE(table), dlg.invertmatrix, 1, 2, y, y+1);
    gtk_signal_connect(GTK_OBJECT(dlg.invertmatrix), "clicked",
                              GTK_SIGNAL_FUNC(calc_inv), &dlg);
    gtk_widget_show(dlg.invertmatrix);
    gtk_table_set_row_spacing(GTK_TABLE(table), y-1, 5);
    y++;

    /* buttons OK cancel etc */
    hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
    gtk_widget_show(hbox);

    dlg.button_ok = gtk_button_new_with_label(get_string(IDS_AAOK));
    dlg.button_cancel = gtk_button_new_with_label(get_string(IDS_AACANCEL));
    dlg.button_help = gtk_button_new_with_label(get_string(IDS_AAHELP));

    gtk_box_pack_start(GTK_BOX(hbox), dlg.button_ok, TRUE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(hbox), dlg.button_cancel, TRUE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(hbox), dlg.button_help, TRUE, TRUE, 10);

    /* Connect our callbacks to the three buttons */
    gtk_signal_connect(GTK_OBJECT(dlg.button_ok), "clicked",
                              GTK_SIGNAL_FUNC(modal_ok), &rc);
    gtk_signal_connect(GTK_OBJECT(dlg.button_cancel), "clicked",
                              GTK_SIGNAL_FUNC(modal_cancel), &rc);
    gtk_signal_connect(GTK_OBJECT(dlg.button_help), "clicked",
                              GTK_SIGNAL_FUNC(modal_help), &rc);

    gtk_widget_show(dlg.button_ok);
    gtk_widget_show(dlg.button_cancel);
    gtk_widget_show(dlg.button_help);

    calc_units(&dlg, option.measure.unit);
    switch(dlg.measure.unit) {
	case IDM_UNITMM:
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg.unit_mm), TRUE);
	    break;
	case IDM_UNITINCH:
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg.unit_inch), TRUE);
	    break;
	case IDM_UNITCUSTOM:
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg.unit_custom), TRUE);
	    break;
	default:
	case IDM_UNITPT:
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg.unit_pt), TRUE);
	    break;
    }
    dlg.ctm =  option.ctm;
    update_dialog_ctm(&dlg, &dlg.ctm);
    dialog_put_float(dlg.tx, option.measure.tx);
    dialog_put_float(dlg.ty, option.measure.ty);
    dialog_put_float(dlg.angle, option.measure.rotate);
    dialog_put_float(dlg.sx, option.measure.sx);
    dialog_put_float(dlg.sy, option.measure.sy);

    /* show dialog and wait for OK, Cancel or close */
    gtk_window_set_focus(GTK_WINDOW(window), dlg.button_ok);
    gtk_window_set_modal(GTK_WINDOW(dlg.window), TRUE);
    gtk_widget_show(dlg.window);
    gtk_main();
 
    if (rc == IDOK) {
        if (dialog_get_ctm(&dlg, &dlg.ctm, TRUE)) {
	    option.ctm = dlg.ctm;
	    option.measure.unit = dlg.measure.unit;
	    dialog_get_float_error(&dlg, dlg.tx, &option.measure.tx, FALSE);
	    dialog_get_float_error(&dlg, dlg.ty, &option.measure.ty, FALSE);
	    dialog_get_float_error(&dlg, dlg.angle, &option.measure.rotate, FALSE);
	    dialog_get_float_error(&dlg, dlg.sx, &option.measure.sx, FALSE);
	    dialog_get_float_error(&dlg, dlg.sy, &option.measure.sy, FALSE);
	}
    }

    gtk_widget_destroy(dlg.window);

    return;
}
