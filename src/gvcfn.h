/* Copyright (C) 1993-1997, Russell Lang.  All rights reserved.
  
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

/* gvcfn.h */
/* Common function prototypes for GSview */


/* in gvpm.c or gvwin.c */
void copy_clipboard(void);
BOOL get_cursorpos(float *x, float *y);
void scroll_to_find(void);

/* in gvpinit.c  or gvwinit.c */
void show_buttons(void);
void delete_buttons(void);
int gsview_create_objects(char *name);
BOOL load_language(int language);
void change_language(void);
void check_language(void);
void unload_zlib(void);
BOOL load_zlib(void);
int config_wizard(void);

/* in gvwdde.c */
int gsview_progman(char *name, char *gsviewpath, 
	int gsver, char *gspath, char *gsargs);

/* in gvcinit.c */
int gsview_printer_profiles(void);
void init_options(void);
void gsview_initc(LPSTR cmdline);
void init_check_menu(void);
void default_gsdll(char *buf);
void default_gsinclude(char *buf);
void install_default(HWND hwnd);
int gsview_changed(void);

/* in gvcmisc.c */
void error_message(char *str);
void info_init(HWND hwnd);
void read_profile(char *str);
void write_profile(void);

/* in gvwmisc or gvpmisc.c */
void post_img_message(int message, int param);
void get_help(void);
int message_box(char *str, int icon);
void delayed_message_box(int id, int icon);
void check_menu_item(int menuid, int itemid, BOOL checked);
int get_menu_string(int menuid, int itemid, char *str, int len);
int load_string(int id, char *str, int len);
void play_system_sound(char *id);
void play_sound(int i);
void info_wait(int id);
int send_prolog(int resource);
void profile_create_section(PROFILE *prf, char *section, int id);
int gs_chdir(char *dirname);
char * gs_getcwd(char *dirname, int size);

/* in gvcdisp.c */
void transform_cursorpos(float *x, float *y);
void transform_point(float *x, float *y);
void itransform_point(float *x, float *y);
int real_depth(int depth);
int get_paper_size_index(void);
void gs_size(PENDING *pend);
void gs_resize(void);
void gs_magnify(float scale);
void gsview_orientation(int new_orientation);
void gsview_media(int new_media);
void gsview_unit(int new_unit);
void gsview_endfile(void);
void e_free_psfile(PSFILE *ppsfile);
PSFILE *gsview_openfile(char *filename);
void gsview_select(void);
void gsview_selectfile(char *filename);
void gsview_display(void);
void gsview_displayfile(char *filename);
void send_orientation_prolog(FILE *f);
void send_epswarn_prolog(FILE *f);
void fix_orientation(FILE *f);
FILE * gp_open_scratch_file(const char *prefix, char *fname, const char *mode);
void reload_if_changed(void);
BOOL dfreopen(void);
void dfclose(void);
BOOL dsc_scan(PSFILE *psf);
void dsc_getpages(FILE *f, int first, int last);
void dsc_header(FILE *f);
void dsc_dopage(void);
void dsc_skip(int skip);
int map_page(int page);
void psfile_free(PSFILE *);
char * psfile_name(PSFILE *psf);

/* in gvpdisp.c or gvwdisp.c */
BOOL psfile_changed(PSFILE *psf);
void psfile_savestat(PSFILE *psf);

/* in gvccmd.c */
int gsview_command(int command);
BOOL not_open(void);
BOOL not_dsc(void);
BOOL order_is_special(void);
void gserror(UINT id, LPSTR str, UINT icon, int sound);
void pserror(char *str);
int not_implemented(void);
void gsview_check_usersize(void);
void gsview_unzoom(void);
void gsview_language(int new_language);

/* in gvpdlg.c or gvwdlg.c */
BOOL get_filename(char *filename, BOOL save, int filter, int title, int help);
BOOL get_string(char *, char *);
BOOL get_page(int *ppage, BOOL multiple, BOOL allpages);
void show_info(void);
void show_about(void);
BOOL get_bbox(void);
BOOL pstoeps_warn(void);
void change_sounds(void);
BOOL install_gsdll(void);
void display_settings(void);
BOOL get_pdf2ps_options(void);
void gs_showmess(void);
HWND gs_showmess_modeless(void);
void gs_addmess_count(char GVFAR *str, int count);
void gs_addmess(char GVFAR *str);

/* in gvcprf.c or gvwprf.c */
PROFILE * profile_open(char *filename);
int profile_read_string(PROFILE *prf, char *section, char *entry, char *def, char *buffer, int len);
BOOL profile_write_string(PROFILE *prf, char *section, char *entry, char *value);
BOOL profile_close(PROFILE *prf);

/* in gvceps.c */
void ps_to_eps(void);
/* see gvceps.h */

/* in gvpeps.c or gvwclip.c */
void clip_convert(void);
void paste_to_file(void);

/* in gvcprn.c */
struct prop_item_s * get_properties(char *device);
void gsview_spool(char *, char *);
void psfile_extract(FILE *f);
char *get_devices(void);
void print_cleanup(void);
void gsview_saveas(void);
void gsview_extract(void);
int enum_upp_path(char *path, char *buffer, int len);
char *uppname_to_model(char *buffer, char *name);
char *uppmodel_to_name(char *buffer, char *model);
BOOL gsview_cprint(char *cfname, char *optfname);

/* in gvctext.c */
void gsview_text_extract(void);
void gsview_text_extract_slow(void);
void gsview_text_find(void);
void gsview_text_findnext(void);
void free_text_index(void);
BOOL make_text_index(void);
int word_find(int x, int y);
BOOL wildmatch(char *w, char *s);

/* in gvcdll.c */
int d_orientation(int pagenum);
int gs_execute(char GVFAR *str, int len);
int gs_printf(const char *fmt, ...);
void gs_process(void);
int gs_page_skip(int skip);
char * gs_argnext(char *argline, char *arg);
int gs_dll_init(GSDLL_CALLBACK callback, char *devname);
int callback_pstotext(char GVFAR *str, unsigned long count);

/* in gvpdll.c or gvwdll.c */
extern GSDLL gsdll;
void gs_load_dll_cleanup(void);
BOOL gs_load_dll(void);
BOOL gs_free_dll(void);
BOOL gsdll_close(void);
#ifdef _Windows
#ifdef __WIN32__
int _export gsdll_callback(int message, char *str, unsigned long count);
#else
int _far _export gsdll_callback(int message, char FAR *str, unsigned long count);
#endif
#else
int gsdll_callback(int message, char *str, unsigned long count);
#endif
int get_message(void);
int peek_message(void);
void wait_event(void);
void begin_crit_section(void);
void end_crit_section(void);
void request_mutex(void);
void release_mutex(void);
int load_pstotext(void);
int unload_pstotext(void);
int send_pstotext_prolog(HINSTANCE hmodule, int resource);

/* gvcpdf.c */
int pdf_head(void);
int pdf_makedoc(int first, int last);
int pdf_trailer(void);
int pdf_page(int pagenum);
int pdf_checktag(LPSTR str, int len);
int pdf_extract(FILE *f);
BOOL gsview_pdf2ps_common(char *psname, char *optname, char *output);
int pdf_orientation(void);
BOOL pdf_get_link(int index, PDFLINK *link);
void pdf_free_link(void);
BOOL is_link(float x, float y, PDFLINK *link);


/* gvwprn.c or gvpprn.c */
void gsview_print(void);
void gsview_pdf2ps(char *output);
