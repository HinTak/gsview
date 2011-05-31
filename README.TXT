===============================================================
README.TXT         1996-08-11
===============================================================
GSview 2.0 - PostScript previewer.  Needs Ghostscript 4.01.

========
Overview
========
GSview is a graphical interface for Ghostscript under MS-Windows
or OS/2.  Ghostscript is an interpreter for the PostScript page 
description language used by laser printers.
For documents following the Adobe PostScript Document Structuring 
Conventions, GSview allows selected pages to be viewed or printed.
GSview must be used with Ghostscript 4.01.

Features include:
 - view pages in arbitrary order (Next, Previous, Goto).
 - selectable display resolution, depth, alpha.
 - page size is automatically selected from DSC comments
   or can be selected using the menu.
 - orientation is automatically selected from DSC comments
   or can be selected using the menu (Portrait, Landscape).
 - single button zoom
 - print selected pages using Ghostscript.
   page offset can be applied.
 - extract selected pages to another file.
 - copy display bitmap to clipboard.
 - save clipboard bitmap as BMP file.
 - add bitmap or user preview to EPS file 
   (Interchange, TIFF or Windows Metafile)
 - graphically select and show bounding box for EPS file.
 - extract bitmap preview or postscript from DOS EPS file.
 - display PDF files
 - on-line help.
 - OS/2 or Win32 executables
 - installation program for GSview and Ghostscript
 - free (GSview Free Public Licence).
New features in version 2.0:
 - 32-bit only.
 - Uses Ghostscript DLL.  Should now be faster and more robust.
 - Multi-threaded under OS/2, Windows 95, Windows NT.
 - Single-threaded under Win32s (same EXE).
 - Ghostscript command line limit is much longer (1024 vs 127 bytes).
 - Supports long filenames with embedded spaces.
   (except in Start Menu or Program Manager).
 - More network friendly for installation.  Can be run directly
   off a CD-ROM.
 - GSview now owns the image window, so "Options | Show BoundingBox"
   is now available under Windows.  A few other nasties should
   also have been eliminated by this change.
 - Simple PDF viewer works better.
 - Handles documents that change the page size or resolution
   by simply ignoring these requests.
 - Better text display now that Ghostscript has better fonts.
 - Checks Ghostscript version.  It will only operate with
   Aladdin Ghostscript 4.01.
 - pstotext for improved text extraction and searching.
   Highlights found words.  Mark text for copying to the clipboard.
 - Setup program uses DLL version of unzip.
 - Anti-aliasing of text and graphics.
 - See gvcnews.txt in the GSview sources for more detailed change
   notes.

GSview was written by Russell Lang.

============
Installation
============
Create three 1.44M floppies containing the following files, or
put them all in the same directory on your hard disk.

Disk 1:
  gsview.zip
  README.TXT	    (this file)
  LICENCE
  FILE_ID.DIZ
  os2setup.exe      OS/2 installation program
  os2unzip.exe      OS/2 Info-ZIP unzip utility
  setup.exe         MS-Windows installation program
  wizunz32.dll      MS-Windows Info-ZIP unzip utility
  emxrt.zip         EMX Run Time library (OS/2 only).

Disk 2:
  gs401ini.zip      Aladdin Ghostscript library files
    PLUS
  gs401w32.zip      MS-Windows Win32 Aladdin Ghostscript
     OR 
  gs401os2.zip      OS/2 Aladdin Ghostscript

Disk 3:
  gs401fn1.zip      Aladdin Ghostscript fonts.

Disk 1 is the contents of gsview20.zip plus the file emxrt.zip obtained
from ftp://ftp-os2.nmsu.edu/os2/unix/emx09b/emxrt.zip.
Disks 2 and 3 are obtained from ftp://ftp.cs.wisc.edu/ghost/aladdin/

If using the floppy install method, 7Mbytes of hard disk space is required.

To install from OS/2, run 'os2setup.exe' from disk 1.
Make sure that the zip files are transferred in binary mode and
are all present.  The install program will not report bad zip files.

For MS-Windows 3.1, you must have Win32s installed before
installing GSview.  See http://www.microsoft.com/ or
  ftp://ftp.microsoft.com/softlib/mslfiles/PW1118.EXE
To install from MS-Windows, run 'setup.exe' from disk 1.
The installation will overwrite any version of GSview or
Aladdin Ghostscript in the destination directories.  If you
wish to save your Fontmap file, back it up to another directory.
Make sure that the zip files are transferred in binary mode and
are all present.

If you don't like automatic installation programs, it is 
possible to install GSview and Ghostscript manually.  See
the Setup topic below for more details.

If you wish to use install GSview without any prompts, give
a destination directory on the command line.  For example:
  setup c:\gstools
This will not prompt for a destination directory and will
overwrite files without asking.

Configuration of GSview will occur the first time it is run.

===========
Compilation
===========
To compile GSview for OS/2 you need IBM NMAKE.EXE and EMX/GCC 0.9b.
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
  gvpm.exe, gvpgs.exe and gvpm.hlp (OS/2) and/or
  gsview32.exe, gvwgs.exe, gsv16spl.exe and gsview.hlp  (MS-Windows)
    and 
  printer.ini
to the GSview directory.  
Unzip Ghostscript into the gsN.NN directory and the Ghostscript
fonts into gsN.NN\fonts.

Start GSview and answer 'Yes' to all the configuration question.
The configuration DEPENDS on ghostscript being in ../gsN.NN
relative to the GSview directory.

GSview (gvpm.exe) will run under OS/2 2.1 and OS/2 Warp.  
It may run under OS/2 2.0.
PM GSview has been compiled with EMX/GCC 0.9b and requires
the emx 0.9b DLL's.  These are available from
  ftp://hobbes.nmsu.edu/os2/unix/emx09b/emxrt.zip

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
the next update to you on a 3.5" HD disk, provided you supply the 
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

================
Acknowledgements
================
Tim Theisen, who wrote Ghostview for X11 which provided the inspiration
for GSview.  Also for permission to use his ps.c module from Ghostview.

L. Peter Deutsch, for writing Ghostscript and for making changes to 
Ghostscript to support GSview.

Paul McJones and Andrew Birrell of DEC, for making pstotext available 
and making changes to it for use with GSview.

==============================================================
