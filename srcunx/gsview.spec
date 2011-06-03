#
# Spec file for GSview 4.91beta
#
# 4.91beta release
#  2011-06-03 Russell Lang <gsview@ghostgum.com.au>
#
Summary: PostScript and PDF previewer
Name: gsview
Version: 4.91
Release: 1
Copyright: Aladdin Free Public License, Ghostgum Software Pty Ltd
Group: Applications/Graphics
Source: ftp://mirror.cs.wisc.edu/pub/mirrors/ghost/ghostgum/gsview-4.9.tar.gz
URL: http://www.cs.wisc.edu/~ghost/gsview/
#Icon: gsview.gif
#Distribution: Unknown
Vendor: Ghostgum Software Pty Ltd
Packager: Russell Lang <gsview@ghostgum.com.au>
Requires: ghostscript >= 7.04
BuildRoot: %{_tmppath}/%{name}-%{version}

%{!?_applnkdir: %define _applnkdir /etc/X11/applnk}
%{!?_icondir: %define _icondir /usr/share/pixmaps}

%description
GSview is a graphical interface for Ghostscript.
Ghostscript is an interpreter for the PostScript page 
description language used by laser printers.
For documents following the Adobe PostScript Document Structuring 
Conventions, GSview allows selected pages to be viewed or printed.
GSview requires Ghostscript 7.04 - 9.99.

%prep
# remove old directory
rm -rf $RPM_BUILD_DIR/gsview-%{version}
mkdir $RPM_BUILD_DIR/gsview-%{version}
#
# unpack main sources
#cd $RPM_BUILD_DIR/gsview-%{version}
%setup -n gsview-%{version}

%build
cd $RPM_BUILD_DIR/gsview-%{version}
make -f srcunx/unx.mak RPM_OPT_FLAGS="$RPM_OPT_FLAGS" \
        GSVIEW_BASE=%{_prefix}          \
        GSVIEW_BINDIR=%{_bindir}        \
        GSVIEW_MANDIR=%{_mandir}        \
        GSVIEW_DOCPATH=%{_docdir}       \
        GSVIEW_ETCPATH=%{_sysconfdir}

%install
rm -rf $RPM_BUILD_ROOT
install -d $RPM_BUILD_ROOT%{_bindir}
install -d $RPM_BUILD_ROOT%{_mandir}
install -d $RPM_BUILD_ROOT%{_docdir}
install -d $RPM_BUILD_ROOT%{_sysconfdir}
install -d $RPM_BUILD_ROOT%{_applnkdir}/Graphics
install -d $RPM_BUILD_ROOT%{_icondir}/

cd $RPM_BUILD_DIR/gsview-%{version}
make -f srcunx/unx.mak install \
        GSVIEW_BASE=$RPM_BUILD_ROOT%{_prefix}           \
        GSVIEW_BINDIR=$RPM_BUILD_ROOT%{_bindir}         \
        GSVIEW_MANDIR=$RPM_BUILD_ROOT%{_mandir}         \
        GSVIEW_DOCPATH=$RPM_BUILD_ROOT%{_docdir}        \
        GSVIEW_ETCPATH=$RPM_BUILD_ROOT%{_sysconfdir}

# desktop/icon files
install srcunx/gvxdesk.txt $RPM_BUILD_ROOT%{_applnkdir}/Graphics/gsview.desktop
install binary/gsview48.png  $RPM_BUILD_ROOT%{_icondir}/gsview.png

%clean
rm -rf $RPM_BUILD_DIR/%{name}-%{version}
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)
%doc %{_docdir}/*
%{_bindir}/*
%dir %{_sysconfdir}/gsview
%config %{_sysconfdir}/gsview/printer.ini
%{_mandir}/man*/*
%{_applnkdir}/Graphics/gsview.desktop
%{_icondir}/gsview.png


%changelog
* Fri Jun 03 2011 Russell Lang <gsview@ghostgum.com.au>
- GSview 4.91beta release
  Minor bug fixes, mostly for Windows 7.
* Sun Nov 18 2007 Russell Lang <gsview@ghostgum.com.au>
- GSview 4.9 release
  Minor bug fixes.
* Sat Feb 25 2006 Russell Lang <gsview@ghostgum.com.au>
- GSview 4.8 release
  Minor bug fixes.
* Fri Apr 25 2005 Russell Lang <gsview@ghostgum.com.au>
- GSview 4.7 release
  Minor bug fixes.
* Sun Jan 11 2004 Russell Lang <gsview@ghostgum.com.au>
- GSview 4.6 release
  Minor bug fixes.
* Sat Oct 18 2003 Russell Lang <gsview@ghostgum.com.au>
- GSview 4.5 release
  Bug fixes. New translations.
* Fri Apr 04 2003 Russell Lang <gsview@ghostgum.com.au>
- GSview 4.4 release
  Bug fixes. New translations.
* Tue Apr 30 2002 Russell Lang <gsview@ghostgum.com.au>
- GSview 4.3 release
  Removed pstotext.  This will be in a separate RPM.
  Sources now in tar.gz.
* Wed Feb 07 2002 Russell Lang <gsview@ghostgum.com.au>
- GSview 4.2 release
* Thu Nov 22 2001 Russell Lang <gsview@ghostgum.com.au>
- GSview 4.1 release
* Sat Oct 20 2001 Russell Lang <gsview@ghostgum.com.au>
- GSview 4.05 beta release
  Use RPM macros as proposed by Rex Dieter.
  Install icon and desktop files.
* Sat Jun 02 2001 Russell Lang <gsview@ghostgum.com.au>
- GSview 4.0 release
  Mark printer.ini an config file and the documentation files
  as such.  Make sure only gsview files are included in the rpm.
* Fri Dec 29 2000 Russell Lang <gsview@ghostgum.com.au>
- GSview 3.6 release
* Sat Dec 16 2000 Russell Lang <gsview@ghostgum.com.au>
- GSview 3.5 release
* Fri Dec 08 2000 Russell Lang <gsview@ghostgum.com.au>
- GSview 3.22-beta release
- Why can't we use normal dates "08 Dec 2000" or "2000-12-08"
  instead of backward "Month Day Year"?
