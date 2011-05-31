===============================================================
README.TXT         1997-06-27
===============================================================
GSview 2.2 - PostScript previewer.  Needs Ghostscript 5.01.

========
Overview
========
GSview is a graphical interface for Ghostscript under MS-Windows
or OS/2.  Ghostscript is an interpreter for the PostScript page 
description language used by laser printers.
For documents following the Adobe PostScript Document Structuring 
Conventions, GSview allows selected pages to be viewed or printed.
GSview requires Ghostscript 4.03 - 5.99.

Features include:
 - View pages in arbitrary order (Next, Previous, Goto).
 - Selectable display resolution, depth, alpha.
 - Page size is automatically selected from DSC comments
   or can be selected using the menu.
 - Orientation can be automatically selected from DSC comments
   or can be selected using the menu (Portrait, Landscape).
 - Single button zoom.
 - Print selected pages using Ghostscript.
   A page offset can be applied.
 - Extract selected pages to another file.
 - Copy display bitmap to clipboard.
 - Save clipboard bitmap as BMP file.
 - Add bitmap or user preview to EPS file 
   (Interchange, TIFF or Windows Metafile)
 - Graphically select and show bounding box for EPS file.
 - Extract bitmap preview or postscript from DOS EPS file.
 - Extract text or search for text.
 - Display PDF files.
 - On-line help.
 - OS/2, Win16 and Win32 executables.
 - Can be run directly off a CD-ROM.
 - German translation (Deutsch, siehe unten).
 - Installation program for GSview and Ghostscript.
 - Free (GSview Free Public Licence).
New features in version 2.2:
 - Works with Aladdin Ghostscript 4.03 - 5.99 (GSview checks the
   Ghostscript version number).
 - New install program.
 - Can read gzip compressed PostScript and PDF files.
 - Honours %%PageOrientation.
 - See gvcnews.txt in the GSview sources for more detailed change
   notes.

GSview was written by Russell Lang.

============
Installation
============
Create three 1.44M floppies containing the following files, or
put them all in the same directory on your hard disk.

Disk 1:
  For Win32, unzip gsv22w32.zip
      README.TXT    (this file)
      LICENCE
      FILE_ID.DIZ
      README32.TXT
      win32.zip
      setup.exe     Installation program
      wizunz32.dll  Info-ZIP unzip utility

  For Win16, unzip gsv22w16.zip
      README.TXT   (this file)
      LICENCE
      FILE_ID.DIZ
      README16.TXT
      win16.zip
      setup16.exe   Installation program
      wizunz16.dll  Info-ZIP unzip utility

  For OS/2, unzip gsv22os2.zip
      README.TXT    (this file)
      LICENCE
      FILE_ID.DIZ
      README2.TXT
      os2.zip
      os2setup.exe  Installation program
      unzip2.dll    Info-ZIP unzip utility

Disk 2:
  gs501ini.zip      Aladdin Ghostscript library files
    PLUS
  gs501w32.zip      MS-Windows Win32 Aladdin Ghostscript
     OR 
  gs501os2.zip      OS/2 Aladdin Ghostscript

  (for Windows 3.1/Win16 use gs403ini.zip plus gs403win.zip)

Disk 3:
  gs501fn1.zip      Aladdin Ghostscript fonts.

Disk 1 is the contents of gsv22w32.zip or gsv22w16.zip or gsv22os2.zip
(all of which are contained in gsview22.zip).
Disks 2 and 3 are obtained from ftp://ftp.cs.wisc.edu/ghost/aladdin/

If using the floppy install method, 10 Mbytes of hard disk space is 
required.

The installation will overwrite any version of GSview or
Aladdin Ghostscript in the destination directories.  If you
wish to save your Fontmap file, back it up to another directory.
Make sure that the zip files are transferred in binary mode and
are all present.

To install from OS/2, run 'os2setup.exe' from disk 1.
If you do not already have EMX isntalled on your system, you will 
also need a Disk 4:
  emxrt.zip         EMX Run Time library (OS/2 only).
from ftp://ftp.cdrom.com/pub/os2/emx09c/emxrt.zip
or   ftp://ftp-os2.nmsu.edu/pub/os2/dev/emx/v0.9c/emxrt.zip

For Windows NT or Windows 95, run 'setup.exe' from disk 1.
If you get a "Can't open DDE connection to Program Manager"
message, your system is corrupted.  Reboot Windows then try again.

For MS-Windows 3.1, it is recommended that you use the
Win32 version.  You must have Win32s installed before
installing GSview.  See http://www.microsoft.com/ or
  ftp://ftp.microsoft.com/softlib/mslfiles/PW1118.EXE
GSview and Ghostscript will not work with some versions
of Win32s.  If it doesn't work with your version, try 
updating to a later version of Win32s.  Some computers
will work with Win32s v1.15, others will not.
To install from MS-Windows, run 'setup.exe' from disk 1.
If you wish to use the slower 16-bit version, install
using 'setup16.exe' from gsv22w16.zip.

If you don't like automatic installation programs, it is 
possible to install GSview and Ghostscript manually.  See
the Setup topic below for more details.

If you wish to use install GSview without any prompts, give
a destination directory on the command line.  For example:
  setup c:\gstools
This will not prompt for a destination directory and will
overwrite files without asking.

Configuration of GSview will occur the first time it is run.

===============
German Language
===============
English:
Brief instructions for German readers.
Obtain the files listed in the section above titled "Installation".
Run setup.exe (MS-Windows) or os2setup.exe (OS/2).
Press the "Next>" button on several pages, then the "Finish>" button
and finally the "Exit" button.
Run GSview.
If running a non-English version of MS-Windows, you will be prompted
to select "English" or "Deutsch".
Otherwise, configure GSview (press the "Next>", "Finish>" and "Exit"
buttons) then select "Options | Language | German"


Deutsch:                                     ISO 8859-1 (Latin-1)
Kurze Beschreibung für deutsche Leser.
Besorgen Sie sich bitte die Dateien,  die in dem obigen Abschnitt 
"Installation" aufgeführt sind.
Starten Sie setup.exe (MS-Windows) oder os2setup.exe (OS/2).
Drücken Sie den "Next>" Knopf mehrfach, dann den "Finish>" Knopf und schließlich "Exit".
Starten Sie GSview.
Wenn Sie eine Version von Windows haben, die nicht Englisch ist, 
so werden Sie dazu aufgefordert zwischen "English" und "Deutsch" 
zu wählen.
Werden Sie dazu nicht aufgefordert, dann konfigurieren Sie Gsview (drücken 
der "Next>", "Finish>" und "Exit" Knöpfe) und wählen dann
"Optionen | Sprache | Deutsch"

===========
Compilation
===========
To compile GSview for OS/2 you need IBM NMAKE.EXE and EMX/GCC 0.9c.
To create the GSview online help you need the IBM toolkit.
Copy gvpm.mak to Makefile and edit COMPBASE and EMXPATH as required.
Type 'nmake'

To compile GSview for Windows you need Borland C++ 4.5.
Copy gvwin.mak to Makefile then edit COMPBASE as required.
Type 'make'.

=====
Setup
=====
These are the manual setup instructions.
It is recommended that you use the installation program
as described in the Installation section above.

Create the directories gsview and gsN.NN (where N.NN is the
Ghostscript version number).

Copy 
  gvpm.exe, gvpgs.exe, gvpmen.dll, gvpmde.dll, gvpmen.hlp
  and gvpmde.hlp (OS/2)
     and/or
  gsview32.exe, gvwgs32.exe, gsv16spl.exe, gsvw32en.dll, gsvw32de.dll,
  gsviewen.hlp and gsviewde.hlp (MS-Windows)
    and 
  printer.ini
to the GSview directory.  
Unzip Ghostscript into the gsN.NN directory and the Ghostscript
fonts into gsN.NN\fonts.

Start GSview and follow the configurations wizard instructions.

GSview (gvpm.exe) will run under OS/2 2.1 and OS/2 Warp. 
It may run under OS/2 2.0.
PM GSview has been compiled with EMX/GCC 0.9c and requires
the emx 0.9c DLL's.  These are available from
  ftp://ftp.cdrom.com/pub/os2/emx09c/emxrt.zip
  ftp://ftp-os2.nmsu.edu/pub/os2/dev/emx/v0.9c/emxrt.zip

GSview (gsview32.exe) will run under Windows NT, Windows 95 and
Windows 3.1/Win32s.  It will not run under Windows 3.0.

See the on-line help for more details.
The 'Common Problems' topic is worth reading if GSview won't work.

====
Bugs
====
Send bug reports to Russell Lang <rjl@aladdin.com>.
(This is an alias provided by Aladdin Enterprises.  Russell Lang is not 
an employee or agent of Aladdin Enterprises).
When reporting bugs, please include the version number and date of 
GSview (from the Help | About box), the version number of Ghostscript, 
and the exact wording of any error messages.
Do not send me large files (>100k bytes) unless requested.
If you get a SYS3175 or GPF, please send the address of the GPF - I can 
use this to help locate the bug.
Please distinguish between bugs in GSview and bugs in Ghostscript.
Don't expect a fast response - the work I'm paid to do has a higher
priority.

=======
Updates
=======
The latest version of GSview should be available from
  ftp://ftp.cs.wisc.edu/ghost/rjl/gsview*.zip
or by mail (see next section).

A World Wide Web home page for Ghostscript, Ghostview and GSview is at
  http://www.cs.wisc.edu/~ghost/

==================================
Copyright, Licensing and Donations
==================================
GSview is copyright by Russell Lang.
GSview is distributed with the GSview Free Public Licence.
This licence is contained in the file LICENCE.
The GSview Free Public Licence does not require any payment to the author;
however the author would welcome any donations to cover costs and time
involved in developing and maintaining GSview.  These may be sent to
    Russell Lang
    12 Princetown Road
    MOUNT WAVERLEY  VIC  3149
    AUSTRALIA
Donations are preferred in Australian dollars because it costs AUS$5
to bank an overseas currency cheque.
If the donation is over AUS$40 or US$40 then the author will mail 
the next update to you on 3.5" HD disks, provided you supply the 
information below.  Updates are not likely to be frequent.
Please accompany donations with the following information:
    Your name.
    Your postal address.
    Your email address (if you have one).
    Version number of GSview.
    Where you obtained or heard about GSview.
    Version number of Ghostscript.
    Donation amount and currency.
    Do you want updates to be mailed to you on disk?

The ps.c module is copyright by Tim Theisen and is covered by the
licence in the file PUBLIC.

GSview uses Ghostscript to display the contents of the PostScript
files.  Ghostscript is a separate program from GSview; it is
written and owned by Aladdin Enterprises, not by Russell Lang.

Aladdin Ghostscript comes with a licence that is more restrictive than 
the GNU Licence; in particular, it restricts the distribution of
Aladdin Ghostscript in commercial contexts.  Please see the file PUBLIC
that accompanies Aladdin Ghostscript for more details.  

GSview uses pstotext in an external DLL. pstotext was written by 
Andrew Birrell and Paul McJones.  It is
  Copyright (C) 1995-1996, Digital Equipment Corporation.
See the licence in pstotext.txt or pstotext.zip for more details.
If you do not agree to the pstotext licence, delete pstotext.zip,
pstotxt2.dll and pstotxt3.dll.

GSview uses zlib 1.0.4 in an external DLL.  zlib is
  Copyright (C) 1995-1996 Jean-loup Gailly and Mark Adler.
  See http://www.cdrom.com/pub/infozip/zlib/

The installation programs use an unzip decompression library (DLL)
by the Info-ZIP group.  The original sources are freely available
from ftp://ftp.cdrom.com/pub/infozip/ on the Internet.

================
Acknowledgements
================
Tim Theisen, who wrote Ghostview for X11 which provided the inspiration
for GSview.  Also for permission to use his ps.c module from Ghostview.

L. Peter Deutsch, for writing Ghostscript and for making changes to 
Ghostscript to support GSview.

Paul McJones and Andrew Birrell of DEC, for making pstotext available 
and making changes to it for use with GSview.

Thomas Pfeiffer and Mrs. Stefanie Kolweyh for translating GSview into 
German/Deutsch.

==============================================================
