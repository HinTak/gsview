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

/* gvxlang.h */

/* Language resources for Unix */

#define STRINGTABLE
#define BEGIN
#define END
#define GSVS(x,y) {x,y},

#define GS_CMD ((GtkItemFactoryCallback)gsview_wcmd)

typedef struct {
    int id;
    const char *str;
} STRING_ENTRY;

/* like struct GtkItemFactoryEntry except the strings are const */
typedef struct {
   const gchar *path;
   const gchar *accelerator;
   GtkItemFactoryCallback callback;
   guint callback_action;
   const gchar *item_type;
  /* Extra data for some item types:
   *  ImageItem  -> pointer to inlined pixbuf stream
   *  StockItem  -> name of stock item
   */
  gconstpointer extra_data;
} MENU_ENTRY;

/* Catalan */
extern STRING_ENTRY string_ct[];
extern int string_ct_len;
extern MENU_ENTRY menu_ct[];
extern int menu_ct_len;

/* German */
extern STRING_ENTRY string_de[];
extern int string_de_len;
extern MENU_ENTRY menu_de[];
extern int menu_de_len;

/* English */
extern STRING_ENTRY string_en[];
extern int string_en_len;
extern MENU_ENTRY menu_en[];
extern int menu_en_len;

/* Spanish */
extern STRING_ENTRY string_es[];
extern int string_es_len;
extern MENU_ENTRY menu_es[];
extern int menu_es_len;

/* French */
extern STRING_ENTRY string_fr[];
extern int string_fr_len;
extern MENU_ENTRY menu_fr[];
extern int menu_fr_len;

/* Italian */
extern STRING_ENTRY string_it[];
extern int string_it_len;
extern MENU_ENTRY menu_it[];
extern int menu_it_len;

/* Greek */
extern STRING_ENTRY string_gr[];
extern int string_gr_len;
extern MENU_ENTRY menu_gr[];
extern int menu_gr_len;

/* Dutch */
extern STRING_ENTRY string_nl[];
extern int string_nl_len;
extern MENU_ENTRY menu_nl[];
extern int menu_nl_len;

/* Russian */
extern STRING_ENTRY string_ru[];
extern int string_ru_len;
extern MENU_ENTRY menu_ru[];
extern int menu_ru_len;

/* Swedish */
extern STRING_ENTRY string_se[];
extern int string_se_len;
extern MENU_ENTRY menu_se[];
extern int menu_se_len;

/* Slovak */
extern STRING_ENTRY string_sk[];
extern int string_sk_len;
extern MENU_ENTRY menu_sk[];
extern int menu_sk_len;

void check_string_order(STRING_ENTRY *st, int stlen);

