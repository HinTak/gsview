/* Copyright (C) 1993-1998, Ghostgum Software Pty Ltd.  All rights reserved.

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

/* de\gvclang.h */
/* Common German language defines */

#define GSVIEW_COPYRIGHT1 "Copyright (C) 1993-1998 Ghostgum Software Pty Ltd."
#define GSVIEW_COPYRIGHT2 "Alle Rechte vorbehalten."
#define GSVIEW_COPYRIGHT3 "Siehe Datei LICENCE für mehr Details."

#define GS_COPYRIGHT1 "Copyright (C) 1994-1998 Aladdin Enterprises,"
#define GS_COPYRIGHT2 "Menlo Park, California, U.S.A.  Alle Rechte vorbehalten."
#define GS_COPYRIGHT3 "Siehe Datei PUBLIC für mehr Details."

/* Buttons */
#ifdef _Windows
#define AAOK "&Ok"
#define AACANCEL "&Abbruch"
#define AADEFAULTS "&Defaults"
#define AAEDIT "&Bearbeiten"
#define AAHELP "&Hilfe"
#define AANEW "&Neu"
#define AATEST "&Test"
#define AAYES "&Ja"
#define AANO "&Nein"
#define AANEXTGT "&Weiter >"
#define AABACKLT "< &Zurück"
#define AAFINISHGT "&Fertig>"
#define AAEXIT "&Ende"
#else
#define AAOK "~Ok"
#define AACANCEL "~Abbruch"
#define AADEFAULTS "~Defaults"
#define AAEDIT "~Bearbeiten"
#define AAHELP "~Hilfe"
#define AANEW "~Neu"
#define AATEST "~Test"
#define AAYES "~Ja"
#define AANO "~Nein"
#define AANEXTGT "~Weiter >"
#define AABACKLT "< ~Zurück"
#define AAFINISHGT "~Fertig>"
#define AAEXIT "~Ende"
#endif

/* About dialog box */
#define AAABOUTWIN "Über GSview für Windows"
#define AAABOUTPM "Über PM GSview"
#define AAABOUTGSV16SPL "Über GSV16SPL"
#define AAABOUTGSVIEWPRINT "Über  GSview Print"
#define AACOPY1 "Version:"
#define AACOPY2 "Grafische Bedienung von Ghostscript "
#define AACOPY3 "Partielles Copyright (C) 1994, Timothy O. Theisen.  Alle Rechte vorbehalten."
#define AACOPY4 "Dieses Programm wird ohne jede Gewährleistung vertrieben."
#define AACOPY5 "Keiner der Autoren oder Distributoren ist verantwortlich"
#define AACOPY6 "für die Folgen des Gebrauchs, weder für die Fehlerfreiheit einer"
#define AACOPY7 "einzelnen Funktion, noch für die des gesamten Programms.  Nähere"
#define AACOPY8 "Details finden sich in der GSview Free Public Licence (der 'Lizenz')."
#define AACOPY9 "Jede Kopie von GSview muß eine Kopie der Lizenz in Form einer ASCII"
#define AACOPY10 "Datei namens 'LICENCE' enthalten.  Diese Lizenz gibt Ihnen das Recht"
#define AACOPY11 "GSview zu kopieren, zu ändern und zu verbreiten, jedoch nur unter"
#define AACOPY12 "den Bedingungen, die in der Lizenz beschrieben sind.  Unter anderem"
#define AACOPY13 "fordert die Lizenz, daß das Copyright und dieser Hinweis in allen"
#define AACOPY14 "Kopien enthalten ist."
#define AACOPY15 "pstotxt3.dll, Copyright (C) 1995-1998 Digital Equipment Corporation,"
#define AACOPY16 "pstotxt2.dll, Copyright (C) 1995-1998 Digital Equipment Corporation,"
#define AACOPY17 "pstotxt1.dll, Copyright (C) 1995-1998 Digital Equipment Corporation,"
#define AACOPY18 "hat eine separate Lizenz.  Siehe Datei pstotext.txt."
#define AACOPY19 "Autor: Russell Lang, Ghostgum Software Pty Ltd"
#define AACOPY20 "Eine Ghostscript DLL Schnittstelle."
#define AACOPY21 "Dies Programm ist Teil von GSview."
#define AACOPY22 "GSview Win32s/Win16 Druck-Manager"

#define AAINPUT "Eingabe"

/* Info dialog box */
#define AAINFO  "Info"
#define AAFILEC "Datei:"
#define AATYPEC "Typ:"
#define AATITLEC "Titel:"
#define AADATEC "Datum:"
#define AABOUNDINGBOXC "BoundingBox:"
#define AAORIENTATIONC "Ausrichtung:"
#define AADEFAULTMEDIAC "Papierformat:"
#define AAPAGEORDERC "Seitenordnung:"
#define AAPAGESC "Seiten:"
#define AAPAGEC "Seite:"
#define AABITMAPC "Bitmap:"

/* Sounds dialog box */
#define AASOUNDS "Klänge"
#define AAEVENTC "Ereignis:"
#define AASOUNDC "Klang:"

/* Select port, printer, page dialog boxes */
#define AASELECTPORT "Auswahl Druckeranschluß"
#define AASELECTPRINTER "Auswahl Drucker"
#define AAPRINTING "Drucke"
#define AASELECTPAGE "Auswahl Seite"
#define AASELECTPAGES "Auswahl Seiten"

/* Printer Setup dialog box */
#define AAPRINTERSETUP "Drucker Einstellung"
#define AADEVICEC "Gerät:"
#define AARESOLUTIONC "Auflösung:"
#define AAQUEUEC "Drucker:"
#define AAPRINTTOFILE "Druck in Datei"
#define AAPSPRINTER "PostScript Drucker"
#define AAOPTIONSC "Optionen:"

#define AAADVANCED "Advanced"
#define AAADVANCEDPSOPT "Advanced PostScript Options"
#define AASENDCTRLDBEFORE "Send CTRL+D before job"
#define AASENDCTRLDAFTER "Send CTRL+D after job"
#define AAPROLOGFILE "Prolog File"
#define AAEPILOGFILE "Epilog File"
#define AABROWSE "Browse"

/* PDF2PS dialog box */
#define AAPDFTOPS	"PDF zu PostScript"
#define AABINARYOK	"Binary"
#define AAPSLEVEL1	"PostScript Level 1"
#define AANOPROCSET	"Kein Prolog"

/* PS2EPS dialog box */
#define AAPSTOEPS "PS zu EPS"
#define	AAPSTOEPSREAD  "Haben Sie die Hilfe `PS zu EPS` gelesen ?"
#define AAPSTOEPSAUTO "Automatische Berechnung der Bounding Box"

#ifdef _Windows
#define AAAPROPERTIES "&Einstellung"
#define AAALLPAGES "Alle &Seiten"
#define AAODDPAGES "&Ungerade"
#define AAEVENPAGES "&Gerade"
#define AAALL "Alle &Seiten"
#define AAODD "&Ungerade"
#define AAEVEN "&Gerade"
#define AAREVERSE "&Absteigend"
#else
#define AAAPROPERTIES "~Einstellung"
#define AAALLPAGES "Alle ~Seiten"
#define AAODDPAGES "~Ungerade"
#define AAEVENPAGES "~Gerade"
#define AAALL "Alle ~Seiten"
#define AAODD "~Ungerade"
#define AAEVEN "~Gerade"
#define AAREVERSE "~Absteigend"
#endif

/* Properties dialog box */
#define AAPROPERTIES "Einstellung"
#define AAPROPERTYC "Einstellung:"
#define AAVALUEC "Wert:"
#define AAPAGEOFFSETPT "Seiten Offset (pts)"
#define AAXC "X:"
#define AAYC "Y:"

/* Edit Properties dialog box */
#define AAEDITPROPERTIES "Einstellung bearbeiten"
#define AAPROPERTYTYPEC "Einstellungstyp:"
#define AANUMBER "Nummer"
#define AASTRING "Text"
#define AANAMEC "Name: z.B.  BitsPerPixel"
#define AAVALUESC "Werte:  z.B.  1,3,8,16,24"
#define AADELETE "Löschen"

#define AABOUNDINGBOX "BoundingBox"

/* Configure Ghostscript dialog box */
#define AACONFIGUREGS "Konfiguriere Ghostscript"
#define AAGHOSTSCRIPTDLLC "Ghostscript DLL:"
#define AAGHOSTSCRIPTINCC "Ghostscript Suchpfad:"
#define AAGHOSTSCRIPTOTHERC "Ghostscript Optionen:"

#define AAGSMESSWIN "GSview für Windows - Ghostscript Meldungen"
#define AAGSMESSPM "PM GSview - Ghostscript Meldungen"
#define AACOPY "Kopiere"

/* Display Settings dialog box */
#define AADISPLAYSETTINGS "Anzeige Einstellungen"
#define AARESOLUTION "Auflösung"
#define AAZOOMRESOLUTION "Zoom Auflösung"
#define AADEPTH "Farbtiefe"
#define AATEXTALPHA "Text Alpha"
#define AAGRAPHICSALPHA "Graphik Alpha"
#define AADRAWMETHOD "Zeichen Methode"
#define AADPI "dpi"
#define AABPP "bits/pixel"
#define AABITS "bits"

/* Measure and custom units dialog box */
/* ENGLISH */
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
#define AACALCXFORM "Calculate Transformation"
#define AATRANSLATE "Translate"
#define AAROTATE "Rotate"
#define AASCALE "Scale"
#define AAINITMATRIX "initmatrix"
#define AAINVERTMATRIX "invertmatrix"


/* Installation program */
#define AAINSTALL1 "GSview Installation"
#define AAINSTALLINTRO1 "Dies Programm soll Ihnen bei der Installation helfen "
#define AAINSTALLINTRO2 "Bevor Sie mit der Installation fortfahren, stellen Sie bitte sicher, daß sich die Dateien aus  README.TXT im aktuellen Verzeichnis oder auf drei Disketten befinden. "
#define AAINSTALLINTRO3 "Die Installation beansprucht ca. 10 MB Platz auf der Festplatte."
#define AAINSTALLCOPYRIGHT1 "Copyright Anmerkungen"
#define AAINSTALLCOPYRIGHT2 "GSview und Aladdin Ghostscript sind separate Programme."
#define AAINSTALLCOPYRIGHT3 "Sie gehören unterschiedlichen Personen."
#define AAINSTALLGSVER1 "Welche Komponenten möchten Sie installieren?"
#define AAINSTALLGSVER2 "Installiere GSview"
#define AAINSTALLGSVER3 "Installiere Ghostscript"
#define AAINSTALLGSVER4 "Welche Version von Ghostscript möchten Sie benutzen?"
#define AAINSTALLDIR1 "Wählen Sie bitte ein Verzeichnis für Gsview und Ghostscript aus."
#define AAINSTALLDIR2 "Innerhalb dieses Verzeichnisses wird sich Gsview in folgendem Unterverzeichnis befinden:"
#define AAINSTALLDIR3 "Ghostscript wird in dem Unterverzeichnis installiert:"
#define AAINSTALLMKDIR1 "Das angegebene Verzeichnis existiert nicht."
#define AAINSTALLMKDIR2 "Ein neues Verzeichnis wird angelegt."
#define AAINSTALLMISSING1 "GSview Installation - Fehlende ZIP Datei"
#define AAINSTALLMISSING2 "Kann ZIP Datei nicht finden. Bitte die Diskette mit folgender Datei einlegen."
#define AAINSTALLCONFIG1 "GSview und Ghostscript benutzen die Umgebungsvariable TEMP um ein Verzeichnis für temporäre Dateien zu bestimmen."
#define AAINSTALLCONFIG2 "Bisher haben Sie kein TEMP definiert."
#define AAINSTALLCONFIG3 "Folgende Zeile wird in Ihre autoexec.bat eingefügt \r     SET TEMP=C:\\"
#define AAINSTALLCONFIG4 "Update autoexec.bat"
#define AAINSTALLCONFIG5 "Backup der alten autoexec.bat behalten "
#define AAINSTALLCONFIGPM1 "GSview und Ghostscript benutzen die Umgebungsvariable TEMP um ein Verzeichnis für temporäre Dateien zu bestimmen. "
#define AAINSTALLCONFIGPM2 "Bisher haben Sie kein TEMP definiert, oder Sie benötigen EMX."
#define AAINSTALLCONFIGPM3 "Folgende Zeile wird Ihrer config.sys zugefügt 'SET TEMP=C:\\'  und/oder Ihr PATH und LIBPATH wird um EMX erweitert. "
#define AAINSTALLCONFIGPM4 "Aktualisieren config.sys"
#define AAINSTALLCONFIGPM5 "Backup der alten config.sys behalten"
#define AAINSTALLFINISH "GSview Setup ist jetzt fertig zum Start der Installation.  \r\rDrücken Sie Fertig um fortzufahren."
#define AAINSTALLUNZIP1 "Entpacke Dateien..."
#define AADIRNOTEXIST "Verzeichnis '%s' existiert nicht. Soll es angelegt werden?"
#define AAMKDIRFAIL "Verzeichnis kann nicht angelegt werden"
#define AABETAWARN "Dies ist eine BETA Test Version von GSview. Sie wird deaktiviert am %04d-%02d-%02d."
#define AAINSTALLOK "Installation erfolgreich."
#define AAINSTALLFAILED "Installation fehlgeschlagen"
#define AAPROGMANGROUP1 "GSview kann eine Programm Manager Gruppe oder ein Start Menü anlegen."
#define AAPROGMANGROUP2 "Lege Gruppe / Symbol an"
#define AAPROGMANGROUP3 "Name"
#define AAPROGMANGROUP4 "GS Tools"
#define AAPROGMANGROUP5 "Eine Programm Manager Gruppe namens \042%s\042 wurde angelegt."
#define AAPROGMANGROUP6 "Ein Desktop Objekt namens \042GSview\042 wurde angelegt."
#define AAPMOBJECT1 "GSview Setup kann ein Desktop Objekt für Gsview anlegen."
#define AAPMOBJECT2 "Lege GSview Programm Object an."
#define AADONEBAK "Die alte autoexec.bat wurde umbenannt in %s"
#define AACANTLOAD "Nicht ladbar %s"
#define AACANTCREATETEMPFILE "Kann temporäre Datei nicht anlegen "
#define AACANTOPENREAD "Kann Datei %s nicht zum lesen öffnen "
#define AACANTOPENWRITE "Kann Datei %s nicht zum schreiben öffnen "
#define AAERRORRENAME "Fehler beim umbenennen von %s zu %s"
#define AANODDEPROGMAN "Kann DDE Verbindung zum Programm Manager nicht öffnen.  Starten Sie bitte Windows erneut."
#define AAINSERTDISK "Disk einlegen, die enthält %s"
#define AAZIPNOTFOUND "Zip Datei nicht gefunden"
#define AAUNZIPCANCELLED "Entpacken abgebrochen \n"
#define AACANTALLOCBUF "Kann keinen Speicher für den Kopierpuffer bekommen "
#define AAPROGRAMOBJECTFAILED  "Kann Desktop Objekt nicht anlegen"
#define AAUNINSTALLTITLE "GSview und Aladdin Ghostscript "
#define AAUNINSTALLITEM "GSview und Aladdin Ghostscript löschen"

/* configure */
#define AAGSVIEWC "GSview:"
#define AAALADDINGSC "Aladdin Ghostscript:"
#define AACFG1 "GSview Konfigurieren"
#define AACFG10 "Dieser Wizard hilft Ihnen Gsview zu konfigurieren."
#define AACFG11 "Wenn Sie einen Eintrag der folgenden Seiten nicht verstehen,\
 dann ändern Sie ihn nicht.  Die Defaults sollten sinnvoll sein."
#define AACFG20 "GSview muß die Version von Ghostscript kennen\
 und wo es installiert ist."
#define AACFG21 "Welche Version von Ghostscript wollen Sie einsetzen?"
#define AACFG22 "In welchem Verzeichnis ist Ghostscript installiert?"
#define AACFG23 "Zusätzliche Typ 1 Zeichensätze sind in"
#define AACFG30 "GSview hält eine Liste von Ghostscript Druckern und\
 Auflösungen in seiner INI Datei vor."
#define AACFG31 "Die Default Liste der Geräte und Auflösungen wurde der Standarddistribution \
 von Ghostscript 5.01 entnommen und könnte unvollständig sein."
#define AACFG32 "Aktualisiere GSview Druckerliste"
#define AACFG33 "Aktualisierung der GSview Druckerliste überschreibt vorhandene Einträge"
#define AACFG40 "GSview kann Dateiverknüfungen für PostScript und\
 Portable Document Format (PDF) Dateien anlegen."
#define AACFG43 "Durch diese Verknüpfungen führt ein Doppelklick auf jene Dateien \
 zum start von GSview.  Außerdem bewirken sie bei einigen WWW browsern,\
 daß GSview als ein Viewer für PostScript und PDF Dateien benutzt wird."
#define AACFG41 "Verknüpfung von PostScript (.ps and .eps) Dateien mit GSview."
#define AACFG42 "Verknüpfung von PDF (.pdf) Dateien mit GSview."
#define AACFG50 "GSview kann eine Programm Manager Gruppe oder\
 Start Menü Einträge für GSview und Aladdin Ghostscript anlegen."
#define AACFG51 "Lege Gruppe / Eintrag an"
#define AACFG52 "Name"
#define AACFG60 "GSview ist jetzt bereit die Konfiguration zu starten.\r\r\
Drücken Sie Fertig um weiter zu machen."
#define AACFG53 "GSview kann ein Desktop Programm Objekt für Gsview anlegen. \
 Dies verknüpft PostScript (.ps und .eps) und PDF (.pdf) Dateien mit GSview."
#define AACFG54 "Lege Programm Objekt an"
#define AACFG70 "Konfiguration erfolgreich."
#define AACFG72 "Um die Konfiguration später zu verändern,\
 starten Sie GSview und wählen \042Optionen | Konfiguriere...\042 oder\
 \042Optionen | Konfiguriere Ghostscript...\042."
#define AACFG73 "Konfiguration fehlgeschlagen."
#define AACFG74 "Konfiguration abgebrochen."

/* Menu items for gvXgs.rc */
#define AAGSVIEWPRINT "GSview Druck"
#define AAGSVIEWPRINTUSAGE "Aufruf: %s [/d] dllpfad optionsdatei eingabedatei\noptionsdatei und eingabedatei werden am ende gelöscht\nEs ist beabsichtigt gvpgs mit temporären Dateien aufzurufen\n"
#ifdef _Windows
#define AAMFILE "&Datei"
#define AAMSAVEAS "Speichen &Unter..."
#define AAMEXIT "E&nde"
#define AAMEDIT "&Bearbeiten"
#define AAMCOPY "Kopieren\tCtrl+K"
#define AAMHELP "&Hilfe"
#define AAMABOUT "&Über..."
#else
#define AAMFILE "~Datei"
#define AAMSAVEAS "Speichern ~Unter..."
#define AAMEXIT "E~nde"
#define AAMEDIT "~Bearbeiten"
#define AAMCOPY "Kopieren\tCtrl+K"
#define AAMHELP "~Hilfe"
#define AAMABOUT "~Über..."
#endif


/* Language changing */
#define AASELECTLANGUAGE "Select Language"   /* Don't translate this */
#define AAENGLISH "English"
#define AADEUTSCH "Deutsch"
#define AAFRANCAIS "Français"
#define AAITALIANO "Italiano"


