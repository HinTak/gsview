/* Copyright (C) 2000-2002, Ghostgum Software Pty Ltd.  All rights reserved.

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
/* se/gvxlang.c */

#include "gvx.h"
#include "gvxres.h"
#include "gvxlang.h"
#include "se/gvclang.h"

/* string table ids must be in order since we use a binary search */
STRING_ENTRY string_se[] = {
#include "gvclang.rc"
#include "gvxlangh.rc"
};

int string_se_len = sizeof(string_se)/sizeof(STRING_ENTRY);

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

MENU_ENTRY menu_se[] = {
  { "/_Arkiv", 			NULL, 	NULL, 	IDM_FILEMENU, "<Branch>" },
  { "/Arkiv/_Öppna...",     	"Ö", 	GS_CMD,	IDM_OPEN, NULL },
  { "/Arkiv/_Välj Fil...",    "V", 	GS_CMD,	IDM_SELECT, NULL },
  { "/Arkiv/S_para Som...",     	"p", 	GS_CMD,	IDM_SAVEAS, NULL },
  { "/Arkiv/_Stäng",     	"S", 	GS_CMD,	IDM_CLOSE, NULL },
  { "/Arkiv/_Info...",     	"I", 	GS_CMD,	IDM_INFO, NULL },
  { "/Arkiv/sep1",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Arkiv/_Konvertera...",     	NULL, 	GS_CMD,	IDM_CONVERTFILE, NULL },
  { "/Arkiv/_Extrahera...",     	"E", 	GS_CMD,	IDM_EXTRACT, NULL },
  { "/Arkiv/PS till EPS",     	NULL, 	GS_CMD,	IDM_PSTOEPS, NULL },
  { "/Arkiv/sep2",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Arkiv/S_kriv ut...",     	"k", 	GS_CMD,	IDM_PRINT, NULL },
  { "/Arkiv/Skriv ut Fil...",     	NULL, 	GS_CMD,	IDM_SPOOL, NULL },
  { "/Arkiv/sep3",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Arkiv/Visa _Meddelanden",     	"M", 	GS_CMD,	IDM_GSMESS, NULL },
  { "/Arkiv/sep4",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Arkiv/1LASTFILE",     	NULL, 	GS_CMD,	IDM_LASTFILE1, NULL },
  { "/Arkiv/2LASTFILE",     	NULL, 	GS_CMD,	IDM_LASTFILE2, NULL },
  { "/Arkiv/3LASTFILE",     	NULL, 	GS_CMD,	IDM_LASTFILE3, NULL },
  { "/Arkiv/4LASTFILE",     	NULL, 	GS_CMD,	IDM_LASTFILE4, NULL },
  { "/Arkiv/sep5",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Arkiv/A_vsluta",     		NULL, 	GS_CMD,	IDM_EXIT, NULL },

  { "/_Editera",      		NULL,   NULL, 	IDM_EDITMENU, "<Branch>" },
/*
  { "/Editera/_Kopiera",     		"<control>C", GS_CMD, IDM_COPYCLIP, NULL },
  { "/Editera/sep1",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Editera/Konvertera _Bitmap",    NULL,	GS_CMD,	IDM_CONVERT, NULL },
*/
  { "/Editera/Spara Bild som BMP",     	NULL,	GS_CMD,	IDM_PASTETO, NULL },
  { "/Editera/sep2",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Editera/_Lägg till EPS Förhandsvisning",   NULL,   NULL, 		0, "<Branch>" },
  { "/Editera/Lägg till EPS Förhandsvisning/_Interchange", NULL,		GS_CMD, IDM_MAKEEPSI, NULL },
  { "/Editera/Lägg till EPS Förhandsvisning/TIFF 4", NULL,		GS_CMD, IDM_MAKEEPST4, NULL },
  { "/Editera/Lägg till EPS Förhandsvisning/_TIFF 6 okomprimerad", NULL,	GS_CMD, IDM_MAKEEPST6U, NULL },
  { "/Editera/Lägg till EPS Förhandsvisning/TIFF 6 _packbitar", NULL,	GS_CMD, IDM_MAKEEPST6P, NULL },
  { "/Editera/Lägg till EPS Förhandsvisning/_Windows Metafil", NULL,	GS_CMD, IDM_MAKEEPSW, NULL },
  { "/Editera/Lägg till EPS Förhandsvisning/_Användardefinierad Förhandsvisning",NULL,GS_CMD, IDM_MAKEEPSU, NULL },
  { "/Editera/_Extrahera EPS",   NULL,   NULL, 	IDM_EXTEPSMENU, "<Branch>" },
  { "/Editera/Extrahera EPS/_PostScript", NULL,		GS_CMD, IDM_EXTRACTPS, NULL },
  { "/Editera/Extrahera EPS/_Förhandsvisning", NULL,			GS_CMD, IDM_EXTRACTPRE, NULL },
  { "/Editera/sep3",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Editera/Mäta", NULL,				GS_CMD, IDM_MEASURE, NULL },
  { "/Editera/Konvertera till vektorformat...", NULL,		GS_CMD, IDM_PSTOEDIT, NULL },
  { "/Editera/_Text extrahera...", NULL,			GS_CMD, IDM_TEXTEXTRACT, NULL },
  { "/Editera/_Finn...", "<control>F",			GS_CMD, IDM_TEXTFIND, NULL },
/*
  { "/Editera/Finn _Nästa", "<control>G",			GS_CMD, IDM_TEXTFINDNEXT, NULL },
*/
  { "/Editera/Finn _Nästa", "F3",			GS_CMD, IDM_TEXTFINDNEXT, NULL },

  { "/O_ptioner",      		NULL,   NULL, 		0, "<Branch>" },
  { "/Optioner/Enkel Konfiguration...", 	NULL,		GS_CMD, IDM_CFG, NULL },
  { "/Optioner/Avancerad Konfiguration...", 	NULL,		GS_CMD, IDM_GSCOMMAND, NULL },
  { "/Optioner/Ljud...", 		NULL,		GS_CMD, IDM_SOUNDS, NULL },
  { "/Optioner/_Enhet",      		NULL,   NULL, 		0, "<Branch>" },
  { "/Optioner/Enhet/_pt", 		NULL,		GS_CMD, IDM_UNITPT, "<CheckItem>" },
  { "/Optioner/Enhet/_mm", 		NULL,		GS_CMD, IDM_UNITMM, "<CheckItem>" },
  { "/Optioner/Enhet/_inch", 		NULL,		GS_CMD, IDM_UNITINCH, "<CheckItem>" },
  { "/Optioner/Enhet/sep1",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Optioner/Enhet/_Hög Upplösning", 	NULL,		GS_CMD, IDM_UNITFINE, "<CheckItem>" },
  { "/Optioner/_Språk",      		NULL,   NULL, 		0, "<Branch>" },
  { "/Optioner/Språk/_English", 	NULL,		GS_CMD, IDM_LANGEN, "<CheckItem>" },
  { "/Optioner/Språk/Catalan", 	NULL,		GS_CMD, IDM_LANGCT, "<CheckItem>" },
  { "/Optioner/Språk/_Deutsch", 	NULL,		GS_CMD, IDM_LANGDE, "<CheckItem>" },
  { "/Optioner/Språk/_Greek", 	NULL,		GS_CMD, IDM_LANGGR, "<CheckItem>" },
  { "/Optioner/Språk/E_spañol", 	NULL,		GS_CMD, IDM_LANGES, "<CheckItem>" },
  { "/Optioner/Språk/_Français", 	NULL,		GS_CMD, IDM_LANGFR, "<CheckItem>" },
  { "/Optioner/Språk/_Italiano", 	NULL,		GS_CMD, IDM_LANGIT, "<CheckItem>" },
  { "/Optioner/Språk/_Nederlands", 	NULL,		GS_CMD, IDM_LANGNL, "<CheckItem>" },
  { "/Optioner/Språk/Russian", 		NULL,		GS_CMD, IDM_LANGRU, "<CheckItem>" },
  { "/Optioner/Språk/Slovak", 		NULL,		GS_CMD, IDM_LANGSK, "<CheckItem>" },
  { "/Optioner/Språk/_Svenska", 	NULL,		GS_CMD, IDM_LANGSE, "<CheckItem>" },
  { "/Optioner/PStillText",      		NULL,   NULL, 		0, "<Branch>" },
  { "/Optioner/PStillText/Avaktiverad", 	NULL,		GS_CMD, IDM_PSTOTEXTDIS, "<CheckItem>" },
  { "/Optioner/PStillText/Normal", 	NULL,		GS_CMD, IDM_PSTOTEXTNORM, "<CheckItem>" },
  { "/Optioner/PStillText/Dvips Cork Encoding", 	NULL,	GS_CMD, IDM_PSTOTEXTCORK, "<CheckItem>" },
  { "/Optioner/DSC Varningar",      		NULL,   NULL, 		0, "<Branch>" },
  { "/Optioner/DSC Varning/Av", 	NULL,		GS_CMD, IDM_DSC_OFF, "<CheckItem>" },
  { "/Optioner/DSC Varning/Fel", 	NULL,		GS_CMD, IDM_DSC_ERROR, "<CheckItem>" },
  { "/Optioner/DSC Varning/Varningar", 	NULL,		GS_CMD, IDM_DSC_WARN, "<CheckItem>" },
  { "/Optioner/DSC Varning/Alla", 	NULL,		GS_CMD, IDM_DSC_INFO, "<CheckItem>" },
  { "/Optioner/Spara Inställningar _Nu", 	NULL,		GS_CMD, IDM_SETTINGS, NULL },
  { "/Optioner/sep1",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Optioner/_Spara Inställningar vid Avslutning", 	NULL,		GS_CMD, IDM_SAVESETTINGS, "<CheckItem>" },
  { "/Optioner/Sä_krare", 			NULL,		GS_CMD, IDM_SAFER, "<CheckItem>" },
  { "/Optioner/Spara Senaste _Katalog", 	NULL,		GS_CMD, IDM_SAVEDIR, "<CheckItem>" },
  { "/Optioner/_Knapprad", 		NULL,		GS_CMD, IDM_BUTTONSHOW, "<CheckItem>" },
/*  { "/Optioner/_Anpassa Fönster till Sida", 	NULL,		GS_CMD, IDM_FITPAGE, "<CheckItem>" }, */
/*  { "/Optioner/Snabb Öppning", 		NULL,		GS_CMD, IDM_QUICK_OPEN, "<CheckItem>" }, */
  { "/Optioner/Automatisk _Uppdatering", 	NULL,		GS_CMD, IDM_AUTOREDISPLAY, "<CheckItem>" },
  { "/Optioner/EPS _Klipp", 		NULL,		GS_CMD, IDM_EPSFCLIP, "<CheckItem>" },
  { "/Optioner/EPS _Varning", 		NULL,		GS_CMD, IDM_EPSFWARN, "<CheckItem>" },
  { "/Optioner/_Ignorera DSC", 		NULL,		GS_CMD, IDM_IGNOREDSC, "<CheckItem>" },
  { "/Optioner/Visa Bounding Bo_x", 	NULL,		GS_CMD, IDM_SHOWBBOX, "<CheckItem>" },

  { "/_Visa",      		NULL,   NULL, 		0, "<Branch>" },
  { "/Visa/_Nästa Sida", 	"plus",	GS_CMD, 	IDM_NEXT, NULL },
  { "/Visa/Före_gående Sida", 	"minus",	GS_CMD, 	IDM_PREV, NULL },
  { "/Visa/_Gå till Sida", 	"G",	GS_CMD, 	IDM_GOTO, NULL },
  { "/Visa/Gå _Bakåt", 		"B",	GS_CMD, 	IDM_GOBACK, NULL },
  { "/Visa/Gå _Framåt",	NULL,	GS_CMD, 	IDM_GOFWD, NULL },
  { "/Visa/_Uppdatera", 	"R",	GS_CMD, 	IDM_REDISPLAY, NULL },
  { "/Visa/sep1",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Visa/Nästa Sida och Hem", "space",	GS_CMD, 	IDM_NEXTHOME, NULL },
  { "/Visa/Föregående Sida och Hem", "<control>H",GS_CMD, 	IDM_PREVHOME, NULL },
  { "/Visa/sep2",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Visa/Full _Skärm", 	"F4",	GS_CMD, 	IDM_FULLSCREEN, NULL },
  { "/Visa/Anpassa _Fönster", 	"F6",	GS_CMD, 	IDM_FITWIN, NULL },

  { "/_Orientering",      		NULL,   NULL, 		0, "<Branch>" },
  { "/Orientering/_Auto", 		NULL,	GS_CMD, 	IDM_AUTOORIENT, "<CheckItem>" },
  { "/Orientering/sep1",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Orientering/_Porträtt", 		NULL,	GS_CMD, 	IDM_PORTRAIT, "<CheckItem>" },
  { "/Orientering/_Landskap", 		NULL,	GS_CMD, 	IDM_LANDSCAPE, "<CheckItem>" },
  { "/Orientering/_Upp-och-ner", 	NULL,	GS_CMD, 	IDM_UPSIDEDOWN, "<CheckItem>" },
  { "/Orientering/_Omvänt Landskap", 		NULL,	GS_CMD, 	IDM_SEASCAPE, "<CheckItem>" },
  { "/Orientering/sep2",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Orientering/S_kifta Landskap", 	NULL,	GS_CMD, 	IDM_SWAPLANDSCAPE, "<CheckItem>" },

  { "/_Media", 			NULL,   NULL, 		0, "<Branch>" },
  { "/Media/_Display Inställningar...", NULL,GS_CMD, 	IDM_DISPLAYSETTINGS, NULL },
  { "/Media/sep1", 		NULL,   NULL, 		0, "<Separator>" },
  { "/Media/_Rotera Media", 	NULL,	GS_CMD, 	IDM_MEDIAROTATE, "<CheckItem>"},
  { "/Media/sep2",     		NULL,   NULL, 		0, "<Separator>" },
  { "/Media/11x17", 		NULL,	GS_CMD, 	IDM_11x17, "<CheckItem>" },
  { "/Media/A3", 		NULL,	GS_CMD, 	IDM_A3, "<CheckItem>" },
  { "/Media/A4", 		NULL,	GS_CMD, 	IDM_A4, "<CheckItem>" },
  { "/Media/A5", 		NULL,	GS_CMD, 	IDM_A5, "<CheckItem>" },
  { "/Media/B4", 		NULL,	GS_CMD, 	IDM_B4, "<CheckItem>" },
  { "/Media/B5", 		NULL,	GS_CMD, 	IDM_B5, "<CheckItem>" },
  { "/Media/Ledger", 		NULL,	GS_CMD, 	IDM_LEDGER, "<CheckItem>" },
  { "/Media/Legal", 		NULL,	GS_CMD, 	IDM_LEGAL, "<CheckItem>" },
  { "/Media/Letter", 		NULL,	GS_CMD, 	IDM_LETTER, "<CheckItem>" },
  { "/Media/Note", 		NULL,	GS_CMD, 	IDM_NOTE, "<CheckItem>" },
  { "/Media/Användardefinierad...", 	NULL,	GS_CMD, 	IDM_USERSIZE, "<CheckItem>"},
  { "/Media/USERSIZE1", 	NULL,	GS_CMD, 	IDM_USERSIZE1, "<CheckItem>"},
  { "/Media/USERSIZE2", 	NULL,	GS_CMD, 	IDM_USERSIZE2, "<CheckItem>"},
  { "/Media/USERSIZE3", 	NULL,	GS_CMD, 	IDM_USERSIZE3, "<CheckItem>"},
  { "/Media/USERSIZE4", 	NULL,	GS_CMD, 	IDM_USERSIZE4, "<CheckItem>"},
  { "/Media/USERSIZE5", 	NULL,	GS_CMD, 	IDM_USERSIZE5, "<CheckItem>"},
  { "/Media/USERSIZE6", 	NULL,	GS_CMD, 	IDM_USERSIZE6, "<CheckItem>"},
  { "/Media/USERSIZE7", 	NULL,	GS_CMD, 	IDM_USERSIZE7, "<CheckItem>"},
  { "/Media/USERSIZE8", 	NULL,	GS_CMD, 	IDM_USERSIZE8, "<CheckItem>"},
  { "/Media/USERSIZE9", 	NULL,	GS_CMD, 	IDM_USERSIZE9, "<CheckItem>"},
  { "/Media/USERSIZE10", 	NULL,	GS_CMD, 	IDM_USERSIZE10, "<CheckItem>"},
  { "/Media/USERSIZE11", 	NULL,	GS_CMD, 	IDM_USERSIZE11, "<CheckItem>"},
  { "/Media/USERSIZE12", 	NULL,	GS_CMD, 	IDM_USERSIZE12, "<CheckItem>"},
  { "/Media/USERSIZE13", 	NULL,	GS_CMD, 	IDM_USERSIZE13, "<CheckItem>"},

  { "/_Hjälp",      		"F1",   NULL, 		0, "<Branch>" },
  { "/Hjälp/_Innehåll", 		NULL,	GS_CMD, 	IDM_HELPCONTENT, NULL },
  { "/Hjälp/_Sök efter Hjälp för", NULL,	GS_CMD, 	IDM_HELPSEARCH, NULL },
  { "/Hjälp/_Tangenter Hjälp", 	NULL,	GS_CMD, 	IDM_HELPKEYS, NULL },
  { "/Hjälp/sep1",		NULL,   NULL, 		0, "<Separator>" },
  { "/Hjälp/_Registrera...", 	NULL,	GS_CMD, 	IDM_REGISTER, NULL },
  { "/Hjälp/_Om...", 		NULL,	GS_CMD, 	IDM_ABOUT, NULL },
};

int menu_se_len = sizeof (menu_se) / sizeof (menu_se[0]);

