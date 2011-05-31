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
} MENU_ENTRY;

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

void check_string_order(STRING_ENTRY *st, int stlen);

