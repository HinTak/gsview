/* Copyright (C) 1993-2012, Ghostgum Software Pty Ltd.  All rights reserved.
  
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

/* sk\gvclang.h */
/* Common Slovak language defines */

#define AASLOVAK "Slovak"

#define GSVIEW_COPYRIGHT1 "Autorské práva (C) 1993-2012 Ghostgum Software Pty Ltd."
#define GSVIEW_COPYRIGHT2 "Všetky práva vyhradené."
#define GSVIEW_COPYRIGHT3 "Pozrite si súbor LICENCE pre viac informácií."

#define GS_COPYRIGHT1 "Autorské práva (C) 2011 Artifex Software, Inc."
#define GS_COPYRIGHT2 "California, U.S.A.  Všetky práva vyhradené."
#define GS_COPYRIGHT3 "Pozrite si súbor PUBLIC pre viac informácií."

/* Buttons */
#ifdef UNIX
#define AAOK "OK"
#define AACANCEL "Zrušiť"
#define AAEDIT "Úpravy"
#define AADEFAULTS "Štandardné"
#define AAHELP "Pomocník"
#define AANEW "Nový"
#define AATEST "Test"
#define AAYES "Áno"
#define AANO "Nie"
#define AANEXTGT "Ďalej >"
#define AABACKLT "< Naspäť"
#define AAFINISHGT "Dokončiť >"
#define AAEXIT "Skončiť"
#else
#ifdef _Windows
#define AAOK "&OK"
#define AACANCEL "&Zrušiť"
#define AAEDIT "Úpr&avy"
#define AADEFAULTS "&Štandardné"
#define AAHELP "&Pomocník"
#define AANEW "&Nový"
#define AATEST "&Test"
#define AAYES "Án&o"
#define AANO "&Nie"
#define AANEXTGT "Ď&alej >"
#define AABACKLT "< Na&späť"
#define AAFINISHGT "&Dokončiť >"
#define AAEXIT "&Skončiť"
#else
#define AAOK "~OK"
#define AACANCEL "~Zrušiť"
#define AADEFAULTS "Š~tandardné"
#define AAEDIT "Úpr~avy"
#define AAHELP "~Pomocník"
#define AANEW "~Nový"
#define AATEST "~Test"
#define AAYES "Án~o"
#define AANO "~Nie"
#define AANEXTGT "Ďa~lej >"
#define AABACKLT "< Na~späť"
#define AAFINISHGT "~Dokončiť >"
#define AAEXIT "~Skončiť"
#endif
#endif

/* About dialog box */
#define AAABOUTWIN "Čo je..."
#define AAABOUTPM "Čo je PM GSview"
#define AAABOUTX11 "Čo je X11 GSview"
#define AAABOUTGSV16SPL "Čo je GSV16SPL"
#define AAABOUTGSVIEWPRINT "Čo je GSview tlač"
#define AACOPY1 "Verzia:"
#define AACOPY2 "Grafické prostredie pre program Ghostscript"
#define AACOPY4 "Tento program je distribuovaný NO WARRANTY OF ANY KIND."
#define AACOPY5 "No author or distributor accepts any responsibility for the"
#define AACOPY6 "consequences of using it, or for whether it serves any particular"
#define AACOPY7 "purpose or works at all, unless he or she says so in writing.  Refer"
#define AACOPY8 "to the GSview Free Public Licence (the 'Licence') for full details."
#define AACOPY9 "Every copy of GSview must include a copy of the Licence, normally"
#define AACOPY10 "in a plain ASCII text file named LICENCE.  The Licence grants you"
#define AACOPY11 "the right to copy, modify and redistribute GSview, but only under"
#define AACOPY12 "certain conditions described in the Licence.  Among other things,"
#define AACOPY13 "the Licence requires that the copyright notice and this notice be"
#define AACOPY14 "preserved on all copies."
#define AACOPY15 "pstotxt3.dll autorské práva (C) 1995-1998 Digital Equipment Corporation"
#define AACOPY16 "pstotxt2.dll autorské práva (C) 1995-1998 Digital Equipment Corporation"
#define AACOPY17 "pstotxt1.dll autorské práva (C) 1995-1998 Digital Equipment Corporation"
#define AACOPY18 "and has a different licence.  Pozrite si súbor pstotext.txt pre viac informácií."
#define AACOPY19 "Autor: Russell Lang, Ghostgum Software Pty Ltd"
#define AACOPY20 "Ghostscript DLL prostredie."
#define AACOPY21 "Tento program je časťou programu GSview."
#define AACOPY22 "GSview Win32s/Win16 spooler"

#define AAINPUT "Vstup"

/* DSC error dialog box */
#define AADSC "Dokument Structuring Conventions"
#define AAIGNOREALLDSC "Ignorovať všetky DSC"

/* Info dialog box */
#define AAINFO  "Informácie"
#define AAFILEC "Súbor:"
#define AATYPEC "Typ:"
#define AATITLEC "Názov:"
#define AADATEC "Dátum:"
#define AABOUNDINGBOXC "BoundingBox:"
#define AAORIENTATIONC "Orientácia:"
#define AADEFAULTMEDIAC "Štandardné médium:"
#define AAPAGEORDERC "Page Order:"
#define AAPAGESC "Počet strán:"
#define AAPAGEC "Aktuálna strana:"
#define AABITMAPC "Bitová mapa:"

/* Sounds dialog box */
#define AASOUNDS "Zvuky"
#define AAEVENTC "Udalosť:"
#define AASOUNDC "Zvuk:"

/* Select port, printer, page dialog boxes */
#define AASELECTPORT "Vybrať port tlačiarne"
#define AASELECTPRINTER "Vybrať tlačiareň"
#define AAPRINTING "Tlačí sa"
#define AASELECTPAGE "Vybrať starnu"
#define AASELECTPAGES "Vybrať strany"

/* Convert and Printer Setup dialog box */
#define AAPRINTERSETUP "Nastavenie tlače"
#define AADEVICEC "Zariadenie:"
#define AARESOLUTIONC "Rozlíšenie:"
#define AAQUEUEC "Rad:"
#define AAPRINTTOFILE "Tlačiť do súboru"
#define AAPSPRINTER "PostScript tlačiareň"
#define AAOPTIONSC "Možnosti:"
#define AAFIXEDMEDIA "Fixed media"
#define AACONVERT "Konvertovať"
#define AAWINPRINTERSETTINGS "Windows nastvenia tlačiarne"
#define AACOLOURS "Farba"
#define AABANDW "Čiernobielo"
#define AAGREY "Sivá"
#define AAFULLCOLOUR "Farebne"
#define AASELECTGSDEVICE "Vybrať Ghostscript zariadenie"

#define AAODDEVEN "Nepárne/Párne"
#define AAIGNOREDSC "Ignorovať DSC"
#define AAPAGES "Strany"
#define AAFROMC "Od:"
#define AATOC "do:"

#define AAADVANCED "Rozšírené"
#define AAADVANCEDPSOPT "Rozšírené možnosti PostScript"
#define AASENDCTRLDBEFORE "Send CTRL+D before job"
#define AASENDCTRLDAFTER "Send CTRL+D after job"
#define AAPROLOGFILE "Prolog súbor"
#define AAEPILOGFILE "Epilog súbor"
#define AABROWSE "Prehľadávať"

/* PS2EPS dialog box */
#define AAPSTOEPS "PS do EPS"
#define AAPSTOEPSREAD "Chcete zobraziť Pomocníka o 'PS to EPS'?"
#define AAPSTOEPSAUTO "Automaticky rátať Bounding Box"

#ifdef UNIX
#define AAAPROPERTIES "Vlastnosti"
#define AAALLPAGES "Všetky strany"
#define AAODDPAGES "Nepárne strany"
#define AAEVENPAGES "Párne strany"
#define AAALL "Všetky"
#define AAODD "Nepárne"
#define AAEVEN "Párne"
#define AAREVERSE "Reverse"
#else
#ifdef _Windows
#define AAAPROPERTIES "&Vlastnosti"
#define AAALLPAGES "&Všetky strany"
#define AAODDPAGES "&Nepárne strany"
#define AAEVENPAGES "&Párne strany"
#define AAALL "&Všetky"
#define AAODD "&Nepárne"
#define AAEVEN "&Párne"
#define AAREVERSE "&Reverse"
#else
#define AAAPROPERTIES "~Vlastnosti"
#define AAALLPAGES "~Všetky strany"
#define AAODDPAGES "~Nepárne strany"
#define AAEVENPAGES "~Párne strany"
#define AAALL "~Všetky"
#define AAODD "~Nepárne"
#define AAEVEN "~Párne"
#define AAREVERSE "~Reverse"
#endif
#endif


/* Properties dialog box */
#define AAPROPERTIES "Vlastnosti"
#define AAPROPERTYC "Vlastnosť:"
#define AAVALUEC "Hodnota:"
#define AAPAGEOFFSETPT "Page Offset (pt)"
#define AAXC "X:"
#define AAYC "Y:"

/* Edit Properties dialog box */
#define AAEDITPROPERTIES "Upraviť vlastnosti"
#define AAPROPERTYTYPEC "Typ vlastnosti:"
#define AANUMBER "Číslo"
#define AASTRING "Reťazec"
#define AANAMEC "Názov: e.g.  BitsPerPixel"
#define AAVALUESC "Hodnoty:  e.g.  1,3,8,16,24"
#define AADELETE "Odstrániť"

#define AABOUNDINGBOX "BoundingBox"

/* Advanced Configure dialog box */
#define AACONFIGUREGS "Rozšírené nastavenia"
#define AAGHOSTSCRIPTDLLC "Ghostscript DLL:"
#define AAGHOSTSCRIPTSOC "Ghostscript zdielaný objekt:"
#define AAGHOSTSCRIPTEXEC "Program Ghostscript:"
#define AAGHOSTSCRIPTINCC "Ghostscript Include Path:"
#define AAGHOSTSCRIPTOTHERC "Možnosti Ghostscript:"
#define AACOPYPRINTERDEF "Copy Printer Defaults"
#define AAASSOCIATEPS "Asociovať .ps súbory s programom GSview"
#define AAASSOCIATEPDF "Associovať .pdf súbory s programom GSview"
#define AACREATESTARTMENU "Vytvoriť programovú skupinu v ponuke Štart"

/* Easy Configure dialog box */
#define AAEASYCONFIGURE "Jednoduché nastavenia"
#define AAWHICHGS "Ktorú verziu programu Ghostscript chcete používať?"

/* Download Ghostscript dialog box */
#define AADOWNLOADGS "Preberanie Ghostscript"
#define AADOWNLOADNOW "Prevziať"
#define AAGSNOTFOUND "Ghostscript sa nenašiel na Vašom počítači.  Musíte prevziať program AFPL Ghostscript a potom ho naištalovať"

#define AAGSMESSWIN "GSview pre Windows - Ghostscript správy"
#define AAGSMESSPM "PM GSview - Ghostscript správy"
#define AAGSMESSX11 "X11 GSview - Ghostscript správy"
#define AACOPY "Kopírovať"

/* Display Settings dialog box */
#define AADISPLAYSETTINGS "Nastavenia zobrazenia"
#define AARESOLUTION "Rozlíženie"
#define AAZOOMRESOLUTION "Zoom Resolution"
#define AADEPTH "Hĺbka"
#define AATEXTALPHA "Textový znak"
#define AAGRAPHICSALPHA "Grafický znak"
#define AADRAWMETHOD "Metóda kreslenia"
#define AADPI "dpi"
#define AABPP "bitov/pixel"
#define AABITS "bitov"

/* Measure and custom units dialog box */
#define AAPT "pt"
#define AAMM "mm"
#define AAINCH "inch"
#define AACUSTOM "Custom"
#define AAMEASURE "Measure"
#define AAUNITC	"Unit:"
#define AASTARTC "Start:"
#define AAFINISHC "Finish:"
#define AADELTAC "Delta:"
#define AALENGTHC "Length:"
#define AACHANGE "Change"
#define AACALCXFORM "Prerátavať zmenu"
#define AATRANSLATE "Prerátvať"
#define AAROTATE "Rotovať"
#define AASCALE "Scale"
#define AAINITMATRIX "Initmatrix"
#define AAINVERTMATRIX "Invertmatrix"

/* PS to Edit dialog box */
#define AAPSTOEDIT "PS do Edit"
#define AAFORMATC "Formát:"
#define AADRAWTASP "Kreslyť text ak mnuhouholník"
#define AAMAPTOLATIN1 "Mapovať do ISO-Latin1"
#define AAFLATNESSC "Plochosť:"
#define AADEFAULTFONTC "Štandardné písmo:"
#define AADRIVEROPTIONSC "Možnosti ovládača:"


/* Installation program */
#define AAINSTALL1 "GSview inštalácia"
#define AAINSTALLINTRO1 "Tento Sprievodca Vám pomôže naištalovať"
#define AAINSTALLINTRO2 "If you are not using the self extracting EXE install, make sure you have the files listed in Readme.htm in the current directory."
#define AAINSTALLINTRO3 "Táto inštalácia potrebuje 20 - 30 MB voľného miesta na disku."
#define AAINSTALLCOPYRIGHT1 "Poznámka k autorským právam"
#define AAINSTALLCOPYRIGHT2 "GSview a AFPL Ghostscript sú oddelené programy."
#define AAINSTALLCOPYRIGHT3 "Sú to vlastne oddelené celky."
#define AAINSTALLGSVER1 "Ktoré súčasti chcete inštalovať?"
#define AAINSTALLGSVER2 "Inštaluje sa GSview"
#define AAINSTALLGSVER3 "Inštaluje sa Ghostscript"
#define AAINSTALLGSVER4 "Ktorú verziu programu Ghostscript chcete používať?"
#define AAINSTALLDIR1 "Vyberte priečinok do ktorého chcete inštalovať GSview a Ghostscript."
#define AAINSTALLDIR2 "Within this directory, GSview will be in the subdirectory:"
#define AAINSTALLDIR3 "Ghostscript will be in the subdirectory:"
#define AAINSTALLMKDIR1 "Určený priečinok neexistuje."
#define AAINSTALLMKDIR2 "Nový priečinok bude vytvorený." 
#define AAINSTALLMISSING1 "GSview inštalácia - Chýba súbor ZIP"
#define AAINSTALLMISSING2 "Nedá sa nájsť súbor ZIP.  Vložte disk so súborom."
#define AAINSTALLCONFIG1 "GSview a Ghostscript pužívajú priečinok pre dočasné súbory."
#define AAINSTALLCONFIG2 "Nemáte určený priečinok pre dočasné súbory."
#define AAINSTALLCONFIG3 "Nasledujúci riadok bude pridaný do sboru autoexec.bat\r     SET TEMP=C:\\"
#define AAINSTALLCONFIG4 "Aktualizovať autoexec.bat"
#define AAINSTALLCONFIG5 "Vytvoriť zálohu starého súboru autoexec.bat"
#define AAINSTALLCONFIGPM1 "GSview A Ghostscript pužívajú priečinok pre dočasné súbory."
#define AAINSTALLCONFIGPM2 "Nemáte určený priečinok pre dočasné súbory, alebo musíte nainnštalovať EMX."
#define AAINSTALLCONFIGPM3 "Nasledujúci riadok bude pridaný do sboru config.sys   'SET TEMP=C:\\'  and/or your PATH and LIBPATH will be updated for EMX."
#define AAINSTALLCONFIGPM4 "Aktualizovať config.sys"
#define AAINSTALLCONFIGPM5 "Vytvoriť zálohu starého súboru config.sys"
#define AAINSTALLFINISH "GSview inštalačný program môže začať inštaláciu.  \r\rKliknite na tlačidlo Dokončiť pre pokračovanie."
#define AAINSTALLUNZIP1 "Extrahujú sa súbory..."
#define AADIRNOTEXIST "Priečinok '%s' neexistuje.  má sa vytvoriť?"
#define AAMKDIRFAIL "Nedá sa vytvoriť priečinok"
#define AABETAWARN "Toto je BETA verzia programu GSview.  Bude vypnuté %04d-%02d-%02d."
#define AAINSTALLOK "Inštalácia úspešne dokončená."
#define AAINSTALLFAILED "Nastala chyba počas inštalácie"
#define AAPROGMANGROUP1 "Vytvárajú sa položky v ponuke Štart."
#define AAPROGMANGROUP2 "Create Group / Item"
#define AAPROGMANGROUP3 "Názov"
#define AAPROGMANGROUP4 "GS nástroje"
#define AAPROGMANGROUP5 "Programová skupina s názvom \042%s\042 bola vytvorená."
#define AAPROGMANGROUP6 "Odkaz na pracovnej ploche s názvom \042GSview\042 bol vytvorený."
#define AAPMOBJECT1 "Nedá sa vytvoriť idkaz na pracovnej ploche pre program GSview."
#define AAPMOBJECT2 "Vytvára sa programován skupina programu GSview."
#define AADONEBAK "Starý súbor autoexec.bat bol premenovaný na %s"
#define AACANTLOAD "Nedá sa nahrať %s"
#define AACANTCREATETEMPFILE "Nedá sa vytvoriť dočasný názov súboru"
#define AACANTOPENREAD "Nedá a otvoriť %s pre čítanie"
#define AACANTOPENWRITE "Nedá sa otvoriť %s pre zápis"
#define AAERRORRENAME "Chyba pri premenovávaní %s na %s"
#define AANODDEPROGMAN "Nedá sa otvoriť DDE spoje nie na správacu súborov.  Spustitesystém Windows znovu."
#define AAINSERTDISK "Vložte disk obsahujúci %s"
#define AAZIPNOTFOUND "Súbor Zip sa nedá nájsť"
#define AAUNZIPCANCELLED "Extrahovanie zrušené\n"
#define AACANTALLOCBUF "Nedá sa vyhradiť pamäť pre kopírovanie vyrovnávacej pamäte"
#define AAPROGRAMOBJECTFAILED "Nedá sa vytvoriť odkaz na pracovnej ploche pre program GSview"
#define AAUNINSTALLTITLE "GSview a AFPL Ghostscript"
#define AAUNINSTALLITEM "Odinštalovať GSview a AFPL Ghostscript"

/* setup program */
#define AAINSTALLSPACE "Táto inštalácia potrebuje 5MB na disku"
#define AAGSVIEWREQUIRES "Program GSview tiež potrebuje"
#define AAVIEWREADMEDETAILS "Pozrite si súbor Readme ako získať program Ghostscript."
#define AACOPYRIGHTNOTICE "Autorské práva"
#define AASELECTGSVIEWDIR "Vyberte priečinok do ktorého sa nainštaluje GSview"
#define AABROWSEE "Prehľadávať..."
#define AAGSVIEWADDSTARTMENU "Inštalátor pridá nasledujúce položky do programovej skupiny"
#define AACREATEFOLDER "Vytvára sa priečinok"
#define AAALLUSERS "Všetci používatelia"
#define AASELECTFOLDER "Vybrať priečinok"
#define AAFOLDER "Priečinok"
#define AADESTFOLDER "Cieľový priečinok"
#define AAVIEWREADME "Zobraziť Readme"

/* configure */
#define AAGSVIEWC "GSview:"
#define AAALADDINGSC "Ghostscript:"
#define AACFG1 "GSview nastavenia"
#define AACFG10 "Tento sprievodca Vám pomôže s nastaveniami preprogram GSview."
#define AACFG11 "Ak neporozumiete položkám na nasledujúcek strane,\
 tak ich nemente.  Štandardné sú tiež rozumné."
#define AACFG20 "GSview proterbuje vedieť aku verziu Ghostscript používate\
 ak kde sa nachádza."
#define AACFG21 "Ktorú verziu programu Ghostscript chcete používať?"
#define AACFG22 "V ktorom priečinku je umiestený program Ghostscript?"
#define AACFG23 "Hľadať ďašie typy 1 písma v"
#define AACFG30 "Program GSview si ponechá zoznam Ghostscript tlačiarní\
 a rozlížení v jeho inicializačnom súbore."
#define AACFG31 "The default list of devices and resolutions is taken from the standard\
 distribution version of Ghostscript 5.50 and may not be complete."
#define AACFG32 "Aktualizovať zoznam talčiarní programu GSview" 
#define AACFG33 "Aktualizuje sa zoznam talčiarní programu GSview, zoznam tlačiarní bude nahradený novým." 
#define AACFG40 "Nedá sa vytvoriť asociaácia medzi programom GSview a súbormi PS a\
 PDF."
#define AACFG43 "With these associations, double clicking on a PostScript\
 or PDF file will start GSview.\
  These associations also allow some WWW browsers\
 to use GSview as a viewer for PostScript and PDF files."
#define AACFG41 "Associate PostScript (.ps and .eps) files with GSview."
#define AACFG42 "Associate PDF (.pdf) files with GSview."
#define AACFG50 "GSview can create a Program Manager Group or\
 Start Menu Item for GSview and AFPL Ghostscript."
#define AACFG51 "Create Group / Item"
#define AACFG52 "Názov"
#define AACFG53 "GSview can create a Desktop Program Object for GSview. \
 This will associate PostScript (.ps and .eps) and PDF (.pdf) files with GSview."
#define AACFG54 "Create Program Object"
#define AACFG60 "GSview is now ready to start configuration. \r\rPress Finish to continue."
#define AACFG70 "Configuration successful."
#define AACFG72 "If you need to change the configuration later,\
 run GSview then select \042Options | Easy Configure...\042 or\
 \042Options | Advanced Configure...\042."
#define AACFG73 "Configuration failed."
#define AACFG74 "Configuration cancelled."


/* For gvXgs.rc */
#define AAGSVIEWPRINT "GSview tlačiareň"
#define AAGSVIEWPRINTUSAGE "Usage: %s [/d] dllpath optionfile inputfile\noptionfile and inputfile will be deleted on exit\nIt is intended that gvpgs be called with temporary files\n"
#ifdef _Windows
#define AAMFILE "&Súbor"
#define AAMSAVEAS "Uložiť &ako..."
#define AAMEXIT "&Skončiť"
#define AAMEDIT "Úpr&avy"
#define AAMCOPY "Kopírovať\tCtrl+C"
#define AAMHELP "&Pomocník"
#define AAMABOUT "Č&o je"
#else
#define AAMFILE "~Súbor"
#define AAMSAVEAS "Uložiť ~ako..."
#define AAMEXIT "~Skončiť"
#define AAMEDIT "Úpr~avy"
#define AAMCOPY "Kopírovať\tCtrl+C"
#define AAMHELP "~Pomocník"
#define AAMABOUT "Č~o je"
#endif

#define WINHELPFILE "gsviewsk.chm"
