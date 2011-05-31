/* Copyright (C) 1993, 1994, 1995, Russell Lang.  All rights reserved.
  
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

/* gvcen.h */
/* Common English language defines */

#define GSVIEW_BASEDIR "gsview"

#define GSVIEW_COPYRIGHT1 "Copyright (C) 1993-1996 Russell Lang."
#define GSVIEW_COPYRIGHT2 "All rights reserved."
#define GSVIEW_COPYRIGHT3 "See the file LICENCE for more details."

#define GS_COPYRIGHT1 "Copyright (C) 1994-1996 Aladdin Enterprises,"
#define GS_COPYRIGHT2 "Menlo Park, California, U.S.A.  All rights reserved."
#define GS_COPYRIGHT3 "See the file PUBLIC for more details."

/* Buttons */
#ifdef _Windows
#define AAOK "&Ok"
#define AACANCEL "&Cancel"
#define AAEDIT "&Edit"
#define AADEFAULTS "&Defaults"
#define AAHELP "&Help"
#define AANEW "&New"
#define AATEST "&Test"
#define AAYES "&Yes"
#define AANO "&No"
#else
#define AAOK "~Ok"
#define AACANCEL "~Cancel"
#define AADEFAULTS "~Defaults"
#define AAEDIT "~Edit"
#define AAHELP "~Help"
#define AANEW "~New"
#define AATEST "~Test"
#define AAYES "~Yes"
#define AANO "~No"
#endif

/* About dialog box */
#define AAABOUTWIN "About GSview for Windows"
#define AAABOUTPM "About PM GSview"
#define AAABOUTGSV16SPL "About GSV16SPL"
#define AAABOUTGSVIEWPRINT "About GSview Print"
#define AACOPY1 "Version:"
#define AACOPY2 "A Ghostscript graphical interface"
#define AACOPY3 "Portions Copyright (C) 1994, Timothy O. Theisen.  All rights reserved."
#define AACOPY4 "This program is distributed with NO WARRANTY OF ANY KIND."
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
#define AACOPY15 "pstotxt3.dll is Copyright (C) 1995-1996 Digital Equipment Corporation"
#define AACOPY16 "pstotxt2.dll is Copyright (C) 1995-1996 Digital Equipment Corporation"
#define AACOPY17 "pstotxt1.dll is Copyright (C) 1995-1996 Digital Equipment Corporation"
#define AACOPY18 "and has a different licence.  See pstotext.txt for details."
#define AACOPY19 "Author: Russell Lang  (rjl@aladdin.com)"
#define AACOPY20 "A Ghostscript DLL interface."
#define AACOPY21 "This program is part of GSview."
#define AACOPY22 "GSview Win32s/Win16 spooler"

#define AAINPUT "Input"

/* Info dialog box */
#define AAINFO  "Info"
#define AAFILEC "File:"
#define AATYPEC "Type:"
#define AATITLEC "Title:"
#define AADATEC "Date:"
#define AABOUNDINGBOXC "BoundingBox:"
#define AAORIENTATIONC "Orientation:"
#define AADEFAULTMEDIAC "Default Media:"
#define AAPAGEORDERC "Page Order:"
#define AAPAGESC "Pages:"
#define AAPAGEC "Page:"
#define AABITMAPC "Bitmap:"

/* Sounds dialog box */
#define AASOUNDS "Sounds"
#define AAEVENTC "Event:"
#define AASOUNDC "Sound:"

/* Select port, printer, page dialog boxes */
#define AASELECTPORT "Select Printer Port"
#define AASELECTPRINTER "Select Printer"
#define AAPRINTING "Printing"
#define AASELECTPAGE "Select Page"
#define AASELECTPAGES "Select Page"

/* Printer Setup dialog box */
#define AAPRINTERSETUP "Printer Setup"
#define AADEVICEC "Device:"
#define AARESOLUTIONC "Resolution:"
#define AAQUEUEC "Queue:"
#define AAPRINTTOFILE "Print to File"
#define AAPSPRINTER "PostScript Printer"

/* PDF2PS dialog box */
#define AAPDFTOPS	"PDF to PostScript"
#define AABINARYOK	"Binary"
#define AAPSLEVEL1	"PostScript Level 1"
#define AANOPROCSET	"No Prolog"

/* PS2EPS dialog box */
#define AAPSTOEPS "PS to EPS"
#define AAPSTOEPSREAD "Have you read help `PS to EPS` ?"
#define AAPSTOEPSAUTO "Automatically calculate Bounding Box"

#ifdef _Windows
#define AAAPROPERTIES "&Properties"
#define AAALLPAGES "&All Pages"
#define AAODDPAGES "O&dd Pages"
#define AAEVENPAGES "&Even Pages"
#define AAALL "&All"
#define AAODD "O&dd"
#define AAEVEN "&Even"
#else
#define AAAPROPERTIES "~Properties"
#define AAALLPAGES "~All Pages"
#define AAODDPAGES "O~dd Pages"
#define AAEVENPAGES "~Even Pages"
#define AAALL "~All"
#define AAODD "O~dd"
#define AAEVEN "~Even"
#endif


/* Properties dialog box */
#define AAPROPERTIES "Properties"
#define AAPROPERTYC "Property:"
#define AAVALUEC "Value:"
#define AAPAGEOFFSETPT "Page Offset (pts)"
#define AAXC "X:"
#define AAYC "Y:"

/* Edit Properties dialog box */
#define AAEDITPROPERTIES "Edit Properties"
#define AAPROPERTYTYPEC "Property Type:"
#define AANUMBER "Number"
#define AASTRING "String"
#define AANAMEC "Name: e.g.  BitsPerPixel"
#define AAVALUESC "Values:  e.g.  1,3,8,16,24"
#define AADELETE "Delete"

#define AABOUNDINGBOX "BoundingBox"

/* Configure Ghostscript dialog box */
#define AACONFIGUREGS "Configure Ghostscript"
#define AAGHOSTSCRIPTDLLC "Ghostscript DLL:"
#define AAGHOSTSCRIPTINCC "Ghostscript Include Path:"
#define AAGHOSTSCRIPTOTHERC "Ghostscript Options:"

#define AAGSMESSWIN "GSview for Windows - Ghostscript Messages"
#define AAGSMESSPM "PM GSview - Ghostscript Messages"
#define AACOPY "Copy"

/* Display Settings dialog box */
#define AADISPLAYSETTINGS "Display Settings"
#define AARESOLUTION "Resolution"
#define AAZOOMRESOLUTION "Zoom Resolution"
#define AADEPTH "Depth"
#define AATEXTALPHA "Text Alpha"
#define AAGRAPHICSALPHA "Graphics Alpha"
#define AADRAWMETHOD "Draw Method"
#define AADPI "dpi"
#define AABPP "bits/pixel"
#define AABITS "bits"

/* Installation program */
#define AAINSTALL1 "GSview Install"
#define AAINSTALLINTRO1 "GSview Install - Introduction"
#define AAINSTALLINTRO2 "Installation program for:"
#define AAINSTALLINTRO3 "Before continuing installation, make sure you have the 3 disks listed in README.TXT."
#define AAINSTALLCOPYRIGHT1 "GSview Install - Copyright"
#define AAINSTALLCOPYRIGHT2 "GSview and Aladdin Ghostscript are separate programs."
#define AAINSTALLCOPYRIGHT3 "They are owned by separate entities."
#define AAINSTALLDIR1 "GSview Install - Directory"
#define AAINSTALLDIR2 "The base directory for GSview and Ghostscript is:"
#define AAINSTALLDIR3 "Within the base directory, GSview will be installed into the subdirectory:"
#define AAINSTALLDIR4 "Ghostscript will be installed into the subdirectory:"
#define AAINSTALLDIR5 "Change the base directory if you wish."
#define AAINSTALLMKDIR1 "GSview Install - Make Directory"
#define AAINSTALLMKDIR2 "The directory" 
#define AAINSTALLMKDIR3 "does not exist.  Create it?" 
#define AAINSTALLMISSING1 "GSview Install - Missing ZIP file"
#define AAINSTALLMISSING2 "Can't find ZIP file.  Insert disk with the file below."
#define AAINSTALLCONFIG1 "GSview Install - Config"
#define AAINSTALLCONFIG2 "Don't update autoexec.bat"
#define AAINSTALLCONFIG3 "The following line will be added to your autoexec.bat\r     SET TEMP=C:\\\rYour old autoexec.bat will be renamed autoexec.gs"
#define AAINSTALLUNZIP1 "GSview Install - Unzipping"
#define AAINSTALLEA1 "Updating Extended Attributes"
#define AADIRNOTEXIST "Directory '%s' does not exist.  Create it?"
#define AAMKDIRFAIL "Couldn't make directory"
#define AABETAWARN "This is a BETA test version of GSview.  It will disable on %04d-%02d-%02d."
#define AAINSTALLOKWIN "Installation successful.\015A Program Manager group named \042GS Tools\042 has been created."
#define AAINSTALLOKPM "Installation successful.\012A GSview program object has been created on the desktop" 
#define AAINSTALLABORT "Installation aborted\012%s"
#define AASKIPGSINSTALL "Ghostscript %s appears to be already installed.  Skip installation of Ghostscript?"


/* For gvXgs.rc */
#define AAGSVIEWPRINT "GSview Print"
#define AAGSVIEWPRINTUSAGE "Usage: %s [/d] dllpath optionfile inputfile\noptionfile and inputfile will be deleted on exit\nIt is intended that gvpgs be called with temporary files\n"
#ifdef _Windows
#define AAMFILE "&File"
#define AAMSAVEAS "Save &As..."
#define AAMEXIT "E&xit"
#define AAMEDIT "&Edit"
#define AAMCOPY "Copy\tCtrl+C"
#define AAMHELP "&Help"
#define AAMABOUT "&About..."
#else
#define AAMFILE "~File"
#define AAMSAVEAS "Save ~As..."
#define AAMEXIT "E~xit"
#define AAMEDIT "~Edit"
#define AAMCOPY "Copy\tCtrl+C"
#define AAMHELP "~Help"
#define AAMABOUT "~About..."
#endif

/* Language changing */
#define AASELECTLANGUAGE "Select Language"
#define AAENGLISH "English"
#define AADEUTSCH "Deutsch"
