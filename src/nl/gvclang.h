/* Copyright (C) 1993-2001, Ghostgum Software Pty Ltd.  All rights reserved.
  
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

/* nl\gvclang.h */
/* Common Dutch language defines */

#define AANEDERLANDS "Nederlands"

#define GSVIEW_COPYRIGHT1 "Copyright (C) 1993-2001 Ghostgum Software Pty Ltd."
#define GSVIEW_COPYRIGHT2 "Alle rechten voorbehouden"
#define GSVIEW_COPYRIGHT3 "Zie het LICENCE bestand voor meer details."

#define GS_COPYRIGHT1 "Copyright (C) 1994-2001 artofcode LLC,"
#define GS_COPYRIGHT2 "Benicia, California, U.S.A.  Alle rechten voorbehouden."
#define GS_COPYRIGHT3 "Zie het bestand PUBLIC voor meer details"

/* Buttons */
#ifdef UNIX
#define AAOK "OK"
#define AACANCEL "Annuleren"
#define AAEDIT "Bewerken"
#define AADEFAULTS "Standaard"
#define AAHELP "Hulp"
#define AANEW "Niew"
#define AATEST "Test"
#define AAYES "Ja"
#define AANO "Neen"
#define AANEXTGT "Volgende >"
#define AABACKLT "< Terug"
#define AAFINISHGT "Afwerken >"
#define AAEXIT "Afsluiten"
#else
#ifdef _Windows
#define AAOK "&Ok"
#define AACANCEL "&Annuleren"
#define AAEDIT "&Bewerken"
#define AADEFAULTS "&Standaard"
#define AAHELP "&Hulp"
#define AANEW "&Niew"
#define AATEST "&Test"
#define AAYES "&Ja"
#define AANO "&Neen"
#define AANEXTGT "&Volgende >"
#define AABACKLT "< &Terug"
#define AAFINISHGT "Afwer&ken >"
#define AAEXIT "Afsl&uiten"
#else
#define AAOK "~Ok"
#define AACANCEL "~Annuleren"
#define AAEDIT "~Bewerken"
#define AADEFAULTS "~Standaard"
#define AAHELP "~Hulp"
#define AANEW "~Niew"
#define AATEST "~Test"
#define AAYES "~Ja"
#define AANO "~Neen"
#define AANEXTGT "~Volgende >"
#define AABACKLT "< ~Terug"
#define AAFINISHGT "Afwer~ken >"
#define AAEXIT "Afsl~uiten"
#endif
#endif

/* About dialog box */
#define AAABOUTWIN "Over GSview voor Windows"
#define AAABOUTPM "Over PM GSview voor OS/2 Warp"
#define AAABOUTX11 "Over X11 GSview"
#define AAABOUTGSV16SPL "Over GSV16SPL"
#define AAABOUTGSVIEWPRINT "Over GSview Print"
#define AACOPY1 "Versie:"
#define AACOPY2 "Ghostscript grafische interface"


#define AACOPY4 "Dit programma werd uitgegeven ZONDER ENIGE GARANTIE welke dan ook."
#define AACOPY5 "Autheurs en verdelers verwerpen elke enkele verantwoordelijkheid"
#define AACOPY6 "wanneer iets fout zou gaan of indien het programma nutteloos blijkt"
#define AACOPY7 "te zijn. Tenzij hier anders en op papier werd beschikt. Meer details"
#define AACOPY8 "vindt u in de GSview Free Public Licence (het bestand \042Licence\042)."
#define AACOPY9 "Elke kopie van GSview moet een kopie van de licentieovereenkomst."
#define AACOPY10 "bevatten. Die bevindt zich doorgaans in een ASCII bestand met de"
#define AACOPY11 "license. Deze verbintenis geeft u het recht GSView te kopiëren,"
#define AACOPY12 "wijzigen enz... echter enkel zoals gestipuleerd werd in het bestand"
#define AACOPY13 "LICENSE. Dit houdt ondermeer in dat de regels inzake autheurs- en"
#define AACOPY14 "kopierechten in elke nieuwe kopie moeten inbegrepen worden. "
#define AACOPY15 "pstotxt3.dll is Copyright (C) 1995-1998 Digital Equipment Corporation"
#define AACOPY16 "pstotxt2.dll is Copyright (C) 1995-1998 Digital Equipment Corporation"
#define AACOPY17 "pstotxt1.dll is Copyright (C) 1995-1998 Digital Equipment Corporation"
#define AACOPY18 "en heeft andere licentievoorwaarden.  Zie pstotext.txt voor de details"
#define AACOPY19 "Autheur: Russell Lang, Ghostgum Software Pty Ltd"
#define AACOPY20 "A Ghostscript DLL interface."
#define AACOPY21 "Dit programma is een onderdeel van GSview."
#define AACOPY22 "GSview Win32s/Win16 spooler"

#define AAINPUT "Input"

/* DSC error dialog box */
#define AADSC "Document Structuurgegevens"
#define AAIGNOREALLDSC "Alle DSC negeren"

/* Info dialog box */
#define AAINFO  "Info"
#define AAFILEC "Bestand:"
#define AATYPEC "Type:"
#define AATITLEC "Titel:"
#define AADATEC "Datum:"
#define AABOUNDINGBOXC "Inbinden:"
#define AAORIENTATIONC "Orientatie:"
#define AADEFAULTMEDIAC "Standaard Papierformaat:"
#define AAPAGEORDERC "Pagina Volgorde:"
#define AAPAGESC "Bladzijden:"
#define AAPAGEC "Bladzijde:"
#define AABITMAPC "Bitmap:"

/* Sounds dialog box */
#define AASOUNDS "Geluidjes"
#define AAEVENTC "Gebeurtenis:"
#define AASOUNDC "Geluid:"

/* Select port, printer, page dialog boxes */
#define AASELECTPORT "Seleceer de Printer Poort"
#define AASELECTPRINTER "Printer selecteren"
#define AAPRINTING "Afdrukken bezig"
#define AASELECTPAGE "Selecteer de bladzijde"
#define AASELECTPAGES "Selecteer de bladzijden"

/* Convert and Printer Setup dialog box */
#define AAPRINTERSETUP "Printer Setup"
#define AADEVICEC "Apparaat:"
#define AARESOLUTIONC "Resolutie:"
#define AAQUEUEC "Wachtrij:"
#define AAPRINTTOFILE "Naar bestand afdrukken"
#define AAPSPRINTER "PostScript Printer"
#define AAOPTIONSC "Opties:"
#define AAFIXEDMEDIA "Onveranderlijk formaat"
#define AACONVERT "Omzetten"
#define AAWINPRINTERSETTINGS "Window Printer instellingen"
#define AACOLOURS "Kleuren"
#define AABANDW "Zwart/wit"
#define AAGREY "Grijswaarden"
#define AAFULLCOLOUR "Volledig in kleur"
#define AASELECTGSDEVICE "Selecteer het Ghostscript apparaat"

#define AAODDEVEN "Odd/Even"
#define AAIGNOREDSC "DSC Negeren"
#define AAPAGES "Bladzijden"
#define AAFROMC "Van:"
#define AATOC "Tot:"

#define AAADVANCED "Geavanceerd"
#define AAADVANCEDPSOPT "Geavanceerde PostScript Opties"
#define AASENDCTRLDBEFORE "Stuur vooraf een CTRL+D"
#define AASENDCTRLDAFTER "Stuur een CTRL+D achteraf"
#define AAPROLOGFILE "Proloog Bestand"
#define AAEPILOGFILE "Epiloog Bestand"
#define AABROWSE "Afzoeken"

/* PDF2PS dialog box */
#define AAPDFTOPS       "PDF naar PostScript"
#define AABINARYOK      "Binair"
#define AAPSLEVEL1      "PostScript niveau 1"
#define AANOPROCSET     "Geen voorvoegsel"

/* PS2EPS dialog box */
#define AAPSTOEPS "PS naar EPS"
#define AAPSTOEPSREAD "Reeds de hulp over `PS naar EPS` ingekeken?"
#define AAPSTOEPSAUTO "Het inbinden automatisch laten verlopen"

#ifdef UNIX
#define AAAPROPERTIES "Instellingen"
#define AAALLPAGES "Alle Pagina's"
#define AAODDPAGES "Onpare bladzijden"
#define AAEVENPAGES "Pare bladzijden"
#define AAALL "Alle"
#define AAODD "Onpare"
#define AAEVEN "Pare"
#define AAREVERSE "Achterste-voren"
#else
#ifdef _Windows
#define AAAPROPERTIES "&Instellingen"
#define AAALLPAGES "&Alle Pagina's"
#define AAODDPAGES "&Onpare bladzijden"
#define AAEVENPAGES "&Pare bladzijden"
#define AAALL "&Alle"
#define AAODD "&Onpare"
#define AAEVEN "&Pare"
#define AAREVERSE "Achterste-vore&n"
#else
#define AAAPROPERTIES "~Instellingen"
#define AAALLPAGES "~Alle Pagina's"
#define AAODDPAGES "~Onpare bladzijden"
#define AAEVENPAGES "~Pare bladzijden"
#define AAALL "~Alle"
#define AAODD "~Onpare"
#define AAEVEN "~Pare"
#define AAREVERSE "Achterste-vore~n"
#endif
#endif
/* Properties dialog box */
#define AAPROPERTIES "Instellingen"
#define AAPROPERTYC "Instelling:"
#define AAVALUEC "Waarde:"
#define AAPAGEOFFSETPT "Pagina Offset (pt)"
#define AAXC "X:"
#define AAYC "Y:"

/* Edit Properties dialog box */
#define AAEDITPROPERTIES "Instellingen bewerken"
#define AAPROPERTYTYPEC "Type Instelling:"
#define AANUMBER "Nummer"
#define AASTRING "Reeks"
#define AANAMEC "Naam: bij voorbeeld 'BitsPerPixel'"
#define AAVALUESC "Waarden: bij voorbeeld 1,3,8,16,24"
#define AADELETE "Wissen"

#define AABOUNDINGBOX "Inbinden"

/* Advanced Configure dialog box */
#define AACONFIGUREGS "Geavanceerde instellingen"
#define AAGHOSTSCRIPTDLLC "Ghostscript DLL:"
#define AAGHOSTSCRIPTSOC "Ghostscript Shared Object:" /* ENGLISH */
#define AAGHOSTSCRIPTEXEC "Ghostscript Program:" /* ENGLISH */
#define AAGHOSTSCRIPTINCC "Ghostscript Pad:"
#define AAGHOSTSCRIPTOTHERC "Ghostscript Opties:"
#define AACOPYPRINTERDEF "Naar Printer afdrukken: standaardinstellingen"
#define AAASSOCIATEPS "*.ps bestanden koppelen met GSview"
#define AAASSOCIATEPDF "*.pdf bestanden koppelen met GSview"
#define AACREATESTARTMENU "Opstartmenu items creëren"

/* Easy Configure dialog box */
#define AAEASYCONFIGURE "Eenvoudige Configuratie"
#define AAWHICHGS "Welke Ghostscript versie wilt u gebruiken?"

/* Download Ghostscript dialog box */
#define AADOWNLOADGS "Ghostscript Afhalen"
#define AADOWNLOADNOW "Nu afhalen"
#define AAGSNOTFOUND "Ghostscript bevindt zich niet op uw systeem.  U moet Ghostscript eerst afhalen en installeren. "

#define AAGSMESSWIN "GSview voor Windows - Ghostscript Berichten"
#define AAGSMESSPM "PM GSview voor OS/2 Warp - Ghostscript Berichten"
#define AAGSMESSX11 "X11 GSview - Ghostscript Berichten"
#define AACOPY "Kopiëren"

/* Display Settings dialog box */
#define AADISPLAYSETTINGS "Instellingen tonen"
#define AARESOLUTION "Resolutie"
#define AAZOOMRESOLUTION "Zoom Resolutie"
#define AADEPTH "Diepte"
#define AATEXTALPHA "Tekst Alpha"
#define AAGRAPHICSALPHA "Grafisch Alpha"
#define AADRAWMETHOD "Tekenmethode"
#define AADPI "dpi"
#define AABPP "bits/pixel"
#define AABITS "bits"

/* Measure and custom units dialog box */
#define AAPT "pt"
#define AAMM "mm"
#define AAINCH "duim"
#define AACUSTOM "Aangepast"
#define AAMEASURE "Maat"
#define AAUNITC "Meetenheden:"
#define AASTARTC "Begin:"
#define AAFINISHC "Einde:"
#define AADELTAC "Delta:"
#define AALENGTHC "Lengte:"
#define AACHANGE "Veranderen"
#define AACALCXFORM "Transformatie berekenen"
#define AATRANSLATE "Vertalen"
#define AAROTATE "Roteren"
#define AASCALE "Verschalen"
#define AAINITMATRIX "initmatrix"
#define AAINVERTMATRIX "invertmatrix"

/* PS to Edit dialog box */
#define AAPSTOEDIT "PS naar Bewerken"
#define AAFORMATC "Formaat:"
#define AADRAWTASP "Tekst als veelhoek tekenen"
#define AAMAPTOLATIN1 "Overzetten naar ISO-Latin1"
#define AAFLATNESSC "Schaduweffect:"
#define AADEFAULTFONTC "Standaardfont:"
#define AADRIVEROPTIONSC "Opties voor stuurbestanden:"

/* Registration */
#define AAREGTOC "Geregistreerd aan:"
#define AANUMBERC "Nummer:"
#define AAGSVIEWREG "GSview Registratie"
#define AAREGISTERNOW "Nu Registreren"
#define AAONLINEREG "Online Registreren"
#define AAREG1 "Tik uw naam en registratie nummmer in"
#define AAREG2 ""
#define AANAG1 "GSview is Copyright 2001 Ghostgum Software Pty Ltd."
#define AANAG2 "Ondersteun de verdere ontwikkeling van GSview door voor dit programma een registratie aan te vragen."
#define AANAG3 ""
#define AANAG4 "GSview kan online worden geregistreerd bij:"
/* Borland Resource Compiler has a bug - it can't handle "//" inside a string */
#define AANAG5 "    http:/\057www.ghostgum.com.au/"


/* Installation program */
#define AAINSTALL1 "GSview Installatie"
#define AAINSTALLINTRO1 "U wordt hierbij geholpen door de installatie weetal"
#define AAINSTALLINTRO2 "Voor wanneer het zelfuitpakkend -.exe bestand niet wordt gebruikt: kijk er op uit dat alle bestanden vermeld in de Readme.htm in de huidige directory aanwezig zijn."
#define AAINSTALLINTRO3 "Het volledige programma neemt circa 20 @ 30 MB op de harde schijf in beslag."
#define AAINSTALLCOPYRIGHT1 "Copyright opmerkingen"
#define AAINSTALLCOPYRIGHT2 "GSview en AFPL Ghostscript zijn totaal afzonderlijke programma's."
#define AAINSTALLCOPYRIGHT3 "En beide programma's hebben verschillende eigenaars."
#define AAINSTALLGSVER1 "Welke onderdelen wenst u te installeren?"
#define AAINSTALLGSVER2 "GSview installeren"
#define AAINSTALLGSVER3 "Ghostscript installeren"
#define AAINSTALLGSVER4 "Welke versie van Ghostscript wil u gebruiken?"
#define AAINSTALLDIR1 "Selecteer een directory waarin GSview en Ghostscript moeten geïnstalleerd worden."
#define AAINSTALLDIR2 "Vanuit deze directory zal GSview geïnstalleerd worden in de subdirectory:"
#define AAINSTALLDIR3 "Ghostscript zal zich bvinden in de subdirectory:"
#define AAINSTALLMKDIR1 "De directory door u aangegeven bestaat niet."
#define AAINSTALLMKDIR2 "Er zal een nieuwe directory worden gecreëerd."
#define AAINSTALLMISSING1 "GSview Setup - Het *.zip bestand ontbreekt."
#define AAINSTALLMISSING2 "Het *.zip bestand bestaat niet op uw systeem.  Plaats de diskette waarop zich onderstaand bestand bevindt in het station."
#define AAINSTALLCONFIG1 "GSview en Ghostscript plaatsen de tijdelijke bestanden in de directory TEMP, die als zodaning dient vernoemd te zijn in de omgevingsvariabele set temp= ."
#define AAINSTALLCONFIG2 "U hebt geen variable TEMP gedefiniëerd."
#define AAINSTALLCONFIG3 "Volgende regel zal worden toegevoegd aan het autoexec.bat\r     SET TEMP=C:\\"
#define AAINSTALLCONFIG4 "Autoexec.bat wordt bijgewerkt"
#define AAINSTALLCONFIG5 "Reservekopie van het vorige autoexec.bat"
#define AAINSTALLCONFIGPM1 "Voor het plaatsen van de tijdelijke bestanden maken GSview en Ghostscript gebruik van de omgevingsvariabele set TEMP= ."
#define AAINSTALLCONFIGPM2 "Ofwel hebt u geen TEMP variabele, ofwel werd  EMX niet geïnstalleerd."
#define AAINSTALLCONFIGPM3 "Volgende regel zal worden toegevoegd aan het 'SET TEMP=C:\\'  en/of uw PAD en LIBPATH zullen worden aangevuld met een verwijzing naar."
#define AAINSTALLCONFIGPM4 "Config.sys wordt bijgewerkt"
#define AAINSTALLCONFIGPM5 "Reservekopie van het vorige config.sys"
#define AAINSTALLFINISH "GSview is nu klaar om geïnstalleerd te worden.  \r\rTik op Afwerken om verder te gaan."
#define AAINSTALLUNZIP1 "Bestanden worden uitgepakt..."
#define AADIRNOTEXIST "Directory '%s' bestaat niet.  Deze directory aanmaken?"
#define AAMKDIRFAIL "Onmogelijk de directory aan te maken."
#define AABETAWARN "Dit is een BETA testversie van GSview.  De werking vervalt op %04d-%02d-%02d."
#define AAINSTALLOK "Installatie met succes afgewerkt."
#define AAINSTALLFAILED "Installatie mislukt!"
#define AAPROGMANGROUP1 "GSview Setup kan een Programmagroep of Start Menu Item aanmaken."
#define AAPROGMANGROUP2 "Aanmaken van Groep / Item"
#define AAPROGMANGROUP3 "Naam"
#define AAPROGMANGROUP4 "GS Tuigjes"
#define AAPROGMANGROUP5 "Een Programmagroep met de naam \042%s\042 werd aangemaakt."
#define AAPROGMANGROUP6 "Een programmaobject met de naam \042GSview\042 werd op de werkplek geplaatst."
#define AAPMOBJECT1 "De GSview Setup kan een programmaobject aanmaken voor GSview."
#define AAPMOBJECT2 "GSview programmaobject wordt aangemaakt."
#define AADONEBAK "Het vorige autoexec.bat werd hernoemd tot %s"
#define AACANTLOAD "%s kan niet ingeladen worden"
#define AACANTCREATETEMPFILE "Het tijdelijke bestand kan niet gecreëerd worden."
#define AACANTOPENREAD "%s kan niet ingelezen worden."
#define AACANTOPENWRITE "%s kan niet naar de schijf worden geschreven."
#define AAERRORRENAME "Fout bij het hernoemen van %s tot %s."
#define AANODDEPROGMAN "DDE verbinding met de Program Manager niet mogelijk.  Windows moet opnieuw worden opgestart."
#define AAINSERTDISK "Plaats de diskette met %s erop in het station."
#define AAZIPNOTFOUND "Zipbestand bevindt zich niet op uw systeem."
#define AAUNZIPCANCELLED "Het uitpakken werd afgelast\n"
#define AACANTALLOCBUF "Kopiëren niet mogelijk wegens probleem met het geheugen."
#define AAPROGRAMOBJECTFAILED "Programmaobject kon niet op de Werkplek worden gezet."
#define AAUNINSTALLTITLE "GSview en AFPL Ghostscript"
#define AAUNINSTALLITEM "GSview en AFPL Ghostscript de-installeren."

/* setup program */
#define AAINSTALLSPACE "Voor deze installatie is minimum 5 MB op de harde schijf vereist."
#define AAGSVIEWREQUIRES "GSview heeft ook nodig"
#define AAVIEWREADMEDETAILS "Lees het Readme bestand om te vernemen hoe u zich Ghostscript kan aanschaffen."
#define AACOPYRIGHTNOTICE "Copyright Notaatje"
#define AASELECTGSVIEWDIR "Selecteer de directory waarin GSview moet worden geïnstalleerd."
#define AABROWSEE "Afzoeken..."
#define AAGSVIEWADDSTARTMENU "GSview Setup zal enige objecten toevoegen aan de Opstarten map."
#define AACREATEFOLDER "Map wordt aangemaakt."
#define AAALLUSERS "Alle Gebruikers."
#define AASELECTFOLDER "Selecteer de map"
#define AAFOLDER "Map"
#define AADESTFOLDER "Doelmap"
#define AAVIEWREADME "Readme lezen"

/* configure */
#define AAGSVIEWC "GSview:"
#define AAALADDINGSC "AFPL Ghostscript:"
#define AACFG1 "GSview Configuratie"
#define AACFG10 "Deze weetal zal u bijstaan met het configureren van GSview."
#define AACFG11 "Indien u niets begrijpt van wat volgt,\
 verander er dan ook niets aan.  De standaardwaarden doen het voortreffelijk sowieso."
#define AACFG20 "GSview moet nauwkeurig weten welke versie van Ghostscript het moet gebruiken,\
 en waar deze versie zich precies bevindt."
#define AACFG21 "Welke versie van Ghostscript wenst u te gebruiken?"
#define AACFG22 "In welke directory bevindt Ghostscript zich?"
#define AACFG23 "Er wordt naar supplementaire Type 1 fonts gezocht in"
#define AACFG30 "GSview houdt een lijst van Ghostscript printerapparaten en\
 printerresoluties bij in het printer.ini bestand."
#define AACFG31 "De standaardlijst van apparaten en resoluties komt uit\
 de Ghostscript 5.50 versie en is waarschijnlijk ietwat verouderd."
#define AACFG32 "GSview printerlijst wordt bijgewerkt."
#define AACFG33 "Bij het bijwerken van de GSview printerlijst zullen ALLE bestaande gegevens overschreven worden."
#define AACFG40 "GSview kan koppelingen aanmaken voor PostScript en\
 Portable Document Format (PDF) bestanden."
#define AACFG43 "Door deze koppelingen zal GSview worden opgestart\
 wanneer op het programmaobject van de PDF- of Postscriptbestanden wordt gedubbelklikt.\
  Ook laten deze koppelingen sommige Webbrowsers toe\
  GSview als lezer te gebruiken voor PostScript en PDF bestanden."
#define AACFG41 "Koppel PostScript (*.ps and *.eps) bestanden met GSview."
#define AACFG42 "Koppel PDF (*.pdf) bestanden met GSview."
#define AACFG50 "GSview kan een Programmagroep of\
 Opstartobject Item aanmaken voor GSview en AFPL Ghostscript."
#define AACFG51 "Groep/Item wordt aangemaakt"
#define AACFG52 "Naam"
#define AACFG53 "GSview kan een programmaobject aanmaken op de Werkplek. \
 Hierbij worden dan PostScript (*.ps, *.eps) en PDF (*.pdf) bestanden gekoppeld aan GSview."
#define AACFG54 "Programmaobject wordt aangemaakt."
#define AACFG60 "GSview is now ready to start configuration. \r\rPress Finish to continue."
#define AACFG70 "Configuration successful."
#define AACFG72 "Indien het later nodig zou zijn de configuratie wijzigen,\
 start dan GSview en activeer dan \042Opties | Eenvoudige configuratie...\042 of\
 \042Opties | geavanceerde configuratie...\042."
#define AACFG73 "Het configureren is mislukt."
#define AACFG74 "Het configureren werd afgelast."


/* For gvXgs.rc */
#define AAGSVIEWPRINT "GSview Afdrukken"
#define AAGSVIEWPRINTUSAGE "Syntaxis: %s [/d] dllpath optiesbestand inputbestand\ngeenoptiesbestand en het inputbestand zal gewist worden bij het afsluiten\nHt is de bedoeling dat gvpgs geactiveerd wordt middels tijdelijke bestanden\n"
#ifdef _Windows
#define AAMFILE "&Bestand"
#define AAMSAVEAS "Opslaan &Als..."
#define AAMEXIT "Af&sluiten"
#define AAMEDIT "&Bewerken"
#define AAMCOPY "&Kopiëren\tCtrl+C"
#define AAMHELP "&Hulp"
#define AAMABOUT "&Over GSview..."
#else
#define AAMFILE "~Bestand"
#define AAMSAVEAS "Opslaan ~Als..."
#define AAMEXIT "Af~sluiten"
#define AAMEDIT "~Bewerken"
#define AAMCOPY "~Kopiëren\tCtrl+C"
#define AAMHELP "~Hulp"
#define AAMABOUT "~Over GSview..."
#endif

#define WINHELPFILE "gsviewnl.hlp"
