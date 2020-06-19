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
/* sk/gvxlang.c */

#include "gvx.h"
#include "gvxres.h"
#include "gvxlang.h"
#include "gvclang.h"

/* string table ids must be in order since we use a binary search */
STRING_ENTRY string_sk[] = {
#include "gvclang.rc"
#include "gvxlangh.rc"
};

int string_sk_len = sizeof(string_sk)/sizeof(STRING_ENTRY);

/* This is the GtkItemFactoryEntry structure used to generate new menus.
   Item 1: The menu path. The letter after the underscore indicates an
           accelerator key once the menu is open.
   Item 2: The accelerator key for the entry
   Item 3: The callback function.
   Item 4: The callback action.  This changes the parameters with
           which the function is called.  The default is 0.
   Item 5: The item type, used to define what kind of an item it is.
           Here are the possible values:

           NULL               -> "<Item>"
           ""                 -> "<Item>"
           "<Title>"          -> create a title item
           "<Item>"           -> create a simple item
           "<CheckItem>"      -> create a check item
           "<ToggleItem>"     -> create a toggle item
           "<RadioItem>"      -> create a radio item
           <path>             -> path of a radio item to link against
           "<Separator>"      -> create a separator
           "<Branch>"         -> create an item to hold sub items (optional)
           "<LastBranch>"     -> create a right justified branch 
*/

MENU_ENTRY menu_sk[] = {
  { "/_Súbor", 			NULL, 	NULL, 	IDM_FILEMENU, "<Branch>" },
  { "/Súbor/_Otvoriť...",     	"O", 	GS_CMD,	IDM_OPEN, NULL },
  { "/Súbor/_Vybrať súbor...",    "S", 	GS_CMD,	IDM_SELECT, NULL },
  { "/Súbor/Uložiť _ako...",     	"A", 	GS_CMD,	IDM_SAVEAS, NULL },
  { "/Súbor/_Zavrieť",     	"C", 	GS_CMD,	IDM_CLOSE, NULL },
  { "/Súbor/_Informácie...",     	"I", 	GS_CMD,	IDM_INFO, NULL },
  { "/Súbor/sep1",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Súbor/_Konvertovať...",     	NULL, 	GS_CMD,	IDM_CONVERTFILE, NULL },
  { "/Súbor/_Extrahovať...",     	"E", 	GS_CMD,	IDM_EXTRACT, NULL },
  { "/Súbor/PS do EPS",     	NULL, 	GS_CMD,	IDM_PSTOEPS, NULL },
  { "/Súbor/sep2",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Súbor/_Tlačiť...",     	"P", 	GS_CMD,	IDM_PRINT, NULL },
  { "/Súbor/Tlačiť súbor...",     	NULL, 	GS_CMD,	IDM_SPOOL, NULL },
  { "/Súbor/sep3",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Súbor/Zobraziť správy",     	"M", 	GS_CMD,	IDM_GSMESS, NULL },
  { "/Súbor/sep4",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Súbor/1LASTFILE",     	NULL, 	GS_CMD,	IDM_LASTFILE1, NULL },
  { "/Súbor/2LASTFILE",     	NULL, 	GS_CMD,	IDM_LASTFILE2, NULL },
  { "/Súbor/3LASTFILE",     	NULL, 	GS_CMD,	IDM_LASTFILE3, NULL },
  { "/Súbor/4LASTFILE",     	NULL, 	GS_CMD,	IDM_LASTFILE4, NULL },
  { "/Súbor/sep5",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Súbor/_Skončiť",     		NULL, 	GS_CMD,	IDM_EXIT, NULL },

  { "/Úpr_avy",      		NULL,   NULL, 	IDM_EDITMENU, "<Branch>" },
/*
  { "/Úpravy/_Kopírovať",     		"<control>C", GS_CMD, IDM_COPYCLIP, NULL },
  { "/Úpravy/sep1",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Úpravy/Konvertovať _bitovú mapu",    NULL,	GS_CMD,	IDM_CONVERT, NULL },
*/
  { "/Úpravy/Uložiť obrázok ako bitovú mapu",     	NULL,	GS_CMD,	IDM_PASTETO, NULL },
  { "/Úpravy/sep2",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Úpravy/_Pridať EPS náhľad",   NULL,   NULL, 		0, "<Branch>" },
  { "/Úpravy/Pridať EPS náhľad/_Interchange", NULL,		GS_CMD, IDM_MAKEEPSI, NULL },
  { "/Úpravy/Pridať EPS náhľad/TIFF 4", NULL,		GS_CMD, IDM_MAKEEPST4, NULL },
  { "/Úpravy/Pridať EPS náhľad/_TIFF 6 bez kompresie", NULL,	GS_CMD, IDM_MAKEEPST6U, NULL },
  { "/Úpravy/Pridať EPS náhľad/TIFF 6 _balené bity", NULL,	GS_CMD, IDM_MAKEEPST6P, NULL },
  { "/Úpravy/Pridať EPS náhľad/_Windows Meta súbor", NULL,	GS_CMD, IDM_MAKEEPSW, NULL },
  { "/Úpravy/Pridať EPS náhľad/_Používateľský náhľad",NULL,GS_CMD, IDM_MAKEEPSU, NULL },
  { "/Úpravy/_Extrahovať EPS",   NULL,   NULL, 	IDM_EXTEPSMENU, "<Branch>" },
  { "/Úpravy/Extrahovať EPS/_PostScript", NULL,		GS_CMD, IDM_EXTRACTPS, NULL },
  { "/Úpravy/Extrahovať EPS/_Náhľad", NULL,			GS_CMD, IDM_EXTRACTPRE, NULL },
  { "/Úpravy/sep3",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Úpravy/Measure", NULL,				GS_CMD, IDM_MEASURE, NULL },
  { "/Úpravy/Konvertovať na vektorový formát...", NULL,		GS_CMD, IDM_PSTOEDIT, NULL },
  { "/Úpravy/_Extrahovať text...", NULL,			GS_CMD, IDM_TEXTEXTRACT, NULL },
  { "/Úpravy/_Hľadať...", "<control>F",			GS_CMD, IDM_TEXTFIND, NULL },
/*
  { "/Úpravy/Hľadať ď_alej", "<control>G",			GS_CMD, IDM_TEXTFINDNEXT, NULL },
*/
  { "/Úpravy/Hľadať ď_alej", "F3",			GS_CMD, IDM_TEXTFINDNEXT, NULL },

  { "/_Možnosti",      		NULL,   NULL, 		0, "<Branch>" },
  { "/Možnosti/Jednoduché nastavenia...", 	NULL,		GS_CMD, IDM_CFG, NULL },
  { "/Možnosti/Rozšírené nastavenia...", 	NULL,		GS_CMD, IDM_GSCOMMAND, NULL },
  { "/Možnosti/Zvuky...", 		NULL,		GS_CMD, IDM_SOUNDS, NULL },
  { "/Možnosti/_Jendotky",      		NULL,   NULL, 		0, "<Branch>" },
  { "/Možnosti/Jendotky/_pt", 		NULL,		GS_CMD, IDM_UNITPT, "<CheckItem>" },
  { "/Možnosti/Jendotky/_mm", 		NULL,		GS_CMD, IDM_UNITMM, "<CheckItem>" },
  { "/Možnosti/Jendotky/_inch", 		NULL,		GS_CMD, IDM_UNITINCH, "<CheckItem>" },
  { "/Možnosti/Jendotky/sep1",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Možnosti/Jendotky/_Fine Resolution", 	NULL,		GS_CMD, IDM_UNITFINE, "<CheckItem>" },
  { "/Možnosti/_Jazyk",      		NULL,   NULL, 		0, "<Branch>" },
  { "/Možnosti/Jazyk/_English", 	NULL,		GS_CMD, IDM_LANGEN, "<CheckItem>" },
  { "/Možnosti/Jazyk/Catalan", 	NULL,		GS_CMD, IDM_LANGCT, "<CheckItem>" },
  { "/Možnosti/Jazyk/_Deutsch", 	NULL,		GS_CMD, IDM_LANGDE, "<CheckItem>" },
  { "/Možnosti/Jazyk/_Greek", 	NULL,		GS_CMD, IDM_LANGGR, "<CheckItem>" },
  { "/Možnosti/Jazyk/E_spańol", 	NULL,		GS_CMD, IDM_LANGES, "<CheckItem>" },
  { "/Možnosti/Jazyk/_Français", 	NULL,		GS_CMD, IDM_LANGFR, "<CheckItem>" },
  { "/Možnosti/Jazyk/_Italiano", 	NULL,		GS_CMD, IDM_LANGIT, "<CheckItem>" },
  { "/Možnosti/Jazyk/_Nederlands", 	NULL,		GS_CMD, IDM_LANGNL, "<CheckItem>" },
  { "/Možnosti/Jazyk/Russian", 	NULL,		GS_CMD, IDM_LANGRU, "<CheckItem>" },
  { "/Možnosti/Jazyk/Slovak", 	NULL,		GS_CMD, IDM_LANGSK, "<CheckItem>" },
  { "/Možnosti/Jazyk/Svenska", 	NULL,		GS_CMD, IDM_LANGSE, "<CheckItem>" },
  { "/Možnosti/PStoText",      		NULL,   NULL, 		0, "<Branch>" },
  { "/Možnosti/PStoText/Vypnutý", 	NULL,		GS_CMD, IDM_PSTOTEXTDIS, "<CheckItem>" },
  { "/Možnosti/PStoText/Normálny", 	NULL,		GS_CMD, IDM_PSTOTEXTNORM, "<CheckItem>" },
  { "/Možnosti/PStoText/Dvips Cork enkódovanie", 	NULL,	GS_CMD, IDM_PSTOTEXTCORK, "<CheckItem>" },
  { "/Možnosti/DSC upozornenia",      		NULL,   NULL, 		0, "<Branch>" },
  { "/Možnosti/DSC upozornenia/Vypnuté", 	NULL,		GS_CMD, IDM_DSC_OFF, "<CheckItem>" },
  { "/Možnosti/DSC upozornenia/Chyby", 	NULL,		GS_CMD, IDM_DSC_ERROR, "<CheckItem>" },
  { "/Možnosti/DSC upozornenia/Upozornenia", 	NULL,		GS_CMD, IDM_DSC_WARN, "<CheckItem>" },
  { "/Možnosti/DSC upozornenia/Všetky", 	NULL,		GS_CMD, IDM_DSC_INFO, "<CheckItem>" },
  { "/Možnosti/Uložiť nastavenia _teraz", 	NULL,		GS_CMD, IDM_SETTINGS, NULL },
  { "/Možnosti/sep1",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Možnosti/_Uložiť nastavenia pri skončení", 	NULL,		GS_CMD, IDM_SAVESETTINGS, "<CheckItem>" },
  { "/Možnosti/Sa_fer", 			NULL,		GS_CMD, IDM_SAFER, "<CheckItem>" },
  { "/Možnosti/Save Last _Directory", 	NULL,		GS_CMD, IDM_SAVEDIR, "<CheckItem>" },
  { "/Možnosti/_Button Bar", 		NULL,		GS_CMD, IDM_BUTTONSHOW, "<CheckItem>" },
/*  { "/Možnosti/_Fit Window to Page", 	NULL,		GS_CMD, IDM_FITPAGE, "<CheckItem>" }, */
/*  { "/Možnosti/Quick Open", 		NULL,		GS_CMD, IDM_QUICK_OPEN, "<CheckItem>" }, */
  { "/Možnosti/Auto _Redisplay", 	NULL,		GS_CMD, IDM_AUTOREDISPLAY, "<CheckItem>" },
  { "/Možnosti/EPS _Clip", 		NULL,		GS_CMD, IDM_EPSFCLIP, "<CheckItem>" },
  { "/Možnosti/EPS _Warn", 		NULL,		GS_CMD, IDM_EPSFWARN, "<CheckItem>" },
  { "/Možnosti/_Ignorovať DSC", 		NULL,		GS_CMD, IDM_IGNOREDSC, "<CheckItem>" },
  { "/Možnosti/Show Bounding Bo_x", 	NULL,		GS_CMD, IDM_SHOWBBOX, "<CheckItem>" },

  { "/_View",      		NULL,   NULL, 		0, "<Branch>" },
  { "/View/_Next Page", 	"plus",	GS_CMD, 	IDM_NEXT, NULL },
  { "/View/Pre_vious Page", 	"minus",	GS_CMD, 	IDM_PREV, NULL },
  { "/View/_Goto Page", 	"G",	GS_CMD, 	IDM_GOTO, NULL },
  { "/View/Go _Back", 		"B",	GS_CMD, 	IDM_GOBACK, NULL },
  { "/View/Go _Forward",	NULL,	GS_CMD, 	IDM_GOFWD, NULL },
  { "/View/_Redisplay", 	"R",	GS_CMD, 	IDM_REDISPLAY, NULL },
  { "/View/sep1",     		NULL,   NULL, 		0, "<Separator>" },
  { "/View/Next Page and Home", "space",	GS_CMD, 	IDM_NEXTHOME, NULL },
  { "/View/Previous Page and Home", "<control>H",GS_CMD, 	IDM_PREVHOME, NULL },
  { "/View/sep2",     		NULL,   NULL, 		0, "<Separator>" },
  { "/View/Full _Screen", 	"F4",	GS_CMD, 	IDM_FULLSCREEN, NULL },
  { "/View/Fit _Window", 	"F6",	GS_CMD, 	IDM_FITWIN, NULL },

  { "/_Orientácia",      		NULL,   NULL, 		0, "<Branch>" },
  { "/Orientácia/_Automatická", 		NULL,	GS_CMD, 	IDM_AUTOORIENT, "<CheckItem>" },
  { "/Orientácia/sep1",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Orientácia/_Portrait", 		NULL,	GS_CMD, 	IDM_PORTRAIT, "<CheckItem>" },
  { "/Orientácia/_Landscape", 		NULL,	GS_CMD, 	IDM_LANDSCAPE, "<CheckItem>" },
  { "/Orientácia/_Obrátene", 	NULL,	GS_CMD, 	IDM_UPSIDEDOWN, "<CheckItem>" },
  { "/Orientácia/_Seascape", 		NULL,	GS_CMD, 	IDM_SEASCAPE, "<CheckItem>" },
  { "/Orientácia/sep2",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Orientácia/S_wap Landscape", 	NULL,	GS_CMD, 	IDM_SWAPLANDSCAPE, "<CheckItem>" },

  { "/_Formát", 			NULL,   NULL, 		0, "<Branch>" },
  { "/Formát/_Nastavenia zobrazenia...", NULL,GS_CMD, 	IDM_DISPLAYSETTINGS, NULL },
  { "/Formát/sep1", 		NULL,   NULL, 		0, "<Separator>" },
  { "/Formát/_Rotovať médium", 	NULL,	GS_CMD, 	IDM_MEDIAROTATE, "<CheckItem>"},
  { "/Formát/sep2",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Formát/11x17", 		NULL,	GS_CMD, 	IDM_11x17, "<CheckItem>" },
  { "/Formát/A3", 		NULL,	GS_CMD, 	IDM_A3, "<CheckItem>" },
  { "/Formát/A4", 		NULL,	GS_CMD, 	IDM_A4, "<CheckItem>" },
  { "/Formát/A5", 		NULL,	GS_CMD, 	IDM_A5, "<CheckItem>" },
  { "/Formát/B4", 		NULL,	GS_CMD, 	IDM_B4, "<CheckItem>" },
  { "/Formát/B5", 		NULL,	GS_CMD, 	IDM_B5, "<CheckItem>" },
  { "/Formát/Ledger", 		NULL,	GS_CMD, 	IDM_LEDGER, "<CheckItem>" },
  { "/Formát/Legal", 		NULL,	GS_CMD, 	IDM_LEGAL, "<CheckItem>" },
  { "/Formát/Letter", 		NULL,	GS_CMD, 	IDM_LETTER, "<CheckItem>" },
  { "/Formát/Note", 		NULL,	GS_CMD, 	IDM_NOTE, "<CheckItem>" },
  { "/Formát/Používateľom definovaný...", 	NULL,	GS_CMD, 	IDM_USERSIZE, "<CheckItem>"},
  { "/Formát/USERSIZE1", 	NULL,	GS_CMD, 	IDM_USERSIZE1, "<CheckItem>"},
  { "/Formát/USERSIZE2", 	NULL,	GS_CMD, 	IDM_USERSIZE2, "<CheckItem>"},
  { "/Formát/USERSIZE3", 	NULL,	GS_CMD, 	IDM_USERSIZE3, "<CheckItem>"},
  { "/Formát/USERSIZE4", 	NULL,	GS_CMD, 	IDM_USERSIZE4, "<CheckItem>"},
  { "/Formát/USERSIZE5", 	NULL,	GS_CMD, 	IDM_USERSIZE5, "<CheckItem>"},
  { "/Formát/USERSIZE6", 	NULL,	GS_CMD, 	IDM_USERSIZE6, "<CheckItem>"},
  { "/Formát/USERSIZE7", 	NULL,	GS_CMD, 	IDM_USERSIZE7, "<CheckItem>"},
  { "/Formát/USERSIZE8", 	NULL,	GS_CMD, 	IDM_USERSIZE8, "<CheckItem>"},
  { "/Formát/USERSIZE9", 	NULL,	GS_CMD, 	IDM_USERSIZE9, "<CheckItem>"},
  { "/Formát/USERSIZE10", 	NULL,	GS_CMD, 	IDM_USERSIZE10, "<CheckItem>"},
  { "/Formát/USERSIZE11", 	NULL,	GS_CMD, 	IDM_USERSIZE11, "<CheckItem>"},
  { "/Formát/USERSIZE12", 	NULL,	GS_CMD, 	IDM_USERSIZE12, "<CheckItem>"},
  { "/Formát/USERSIZE13", 	NULL,	GS_CMD, 	IDM_USERSIZE13, "<CheckItem>"},

  { "/_Pomocník",      		"F1",   NULL, 		0, "<Branch>" },
  { "/Pomocník/_Obsah", 		NULL,	GS_CMD, 	IDM_HELPCONTENT, NULL },
  { "/Pomocník/_Register", NULL,	GS_CMD, 	IDM_HELPSEARCH, NULL },
  { "/Pomocník/_Klávesové skratky", 	NULL,	GS_CMD, 	IDM_HELPKEYS, NULL },
  { "/Pomocník/sep1",		NULL,   NULL, 		0, "<Separator>" },
  { "/Pomocník/_Čo je...", 		NULL,	GS_CMD, 	IDM_ABOUT, NULL },
};

int menu_sk_len = sizeof (menu_sk) / sizeof (menu_sk[0]);

