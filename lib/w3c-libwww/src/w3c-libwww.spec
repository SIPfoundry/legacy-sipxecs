Name:          w3c-libwww
Version:       5.4.1
Release:       0
Summary:       HTTP library of common code

Group:         System Environment/Libraries
License:       W3C (see: http://www.w3.org/Consortium/Legal/copyright-software.html)
URL:           http://www.w3.org/Library
Source:        w3c-libwww-5.4.1-0.tar.gz
BuildRoot:     %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires: libtool autoconf automake zlib-devel openssl-devel
Obsoletes:     w3c-libwww < $(VERSION) w3c-libwww > $(VERSION) w3c-libwww-apps < $(VERSION) w3c-libwww-apps > $(VERSION) w3c-libwww-debuginfo < $(VERSION) w3c-libwww-debuginfo > $(VERSION)

%description
Libwww is a general-purpose Web API written in C for Unix and Windows (Win32).
With a highly extensible and layered API, it can accommodate many different
types of applications including clients, robots, etc. The purpose of libwww
is to provide a highly optimized HTTP sample implementation as well as other
Internet protocols and to serve as a testbed for protocol experiments.

%package devel
Summary: Header files for programs that use libwww
Group: Development/Libraries
Requires: w3c-libwww = %{version}-%{release}
Requires: zlib-devel openssl-devel

%description devel
Header files for libwww, which are available as public libraries.

%package apps
Summary: Applications built using Libwww web library
Group: Applications/Internet
Requires: w3c-libwww = %{version}-%{release}

%description apps

Web applications built using Libwww: Robot, Command line tool, 
line mode browser.  The Robot can crawl web sites faster, and
with lower load, than any other web walker that we know of, 
due to its extensive pipelining and use of HTTP/1.1.

The command line tool (w3c) is very useful for manipulation of 
Web sites that implement more than just HTTP GET (e.g. PUT, 
 POST, etc.).

The line mode browser is a minimal line mode web browser; 
often useful to convert to ascii text.

%prep
%setup -q -n %{name}-%{version}-%{release}

# we don't want the libwww versions
rm -fr modules/expat modules/idn

#perl config/winConfigure.pl
libtoolize -c -f
aclocal
autoheader
automake --add-missing
autoconf
echo timestamp > stamp-h.in

%build
%configure --enable-shared --disable-static --with-gnu-ld --with-regex --with-zlib --with-ssl
export tagname=CC
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT

export tagname=CC
make DESTDIR=$RPM_BUILD_ROOT install

pushd $RPM_BUILD_ROOT
  chmod +x .%{_libdir}/lib{www*,md5}.so.0.*
popd

install -p -m644 wwwconf.h ${RPM_BUILD_ROOT}/%{_includedir}/w3c-libwww/

rm -f ${RPM_BUILD_ROOT}/%{_libdir}/*.la

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%doc COPYRIGHT.html LICENSE.html PATCHES.html README.html
%{_libdir}/libwww*.so.*
%{_libdir}/libmd5.so.*
%{_libdir}/libpics.so.*
%{_datadir}/w3c-libwww

%files apps
%defattr(-,root,root,-)
%{_bindir}/webbot
%{_bindir}/w3c
%{_bindir}/www

%files devel
%defattr(-,root,root,-)
%{_bindir}/libwww-config
%{_libdir}/lib*.so

%{_includedir}/w3c-libwww

%changelog
* Fri Sep 15 2006 Andreas Bierfert <andreas.bierfert[AT]lowlatency.de>
5.4.1-0.4.20060206cvs
- FE6 rebuild

* Sat Apr 22 2006 Andreas Bierfert <andreas.bierfert[AT]lowlatency.de>
5.4.1-0.3.20060206cvs
- fix md5 (#187895)

* Tue Feb 28 2006 Andreas Bierfert <andreas.bierfert[AT]lowlatency.de>
5.4.1-0.2.20060206cvs
- readd wwwconfig.h to fix internal header errors

* Wed Feb 08 2006 Andreas Bierfert <andreas.bierfert[AT]lowlatency.de>
5.4.1-0.1.20060206cvs
- base of cvs snapshot which should have been 5.4.1

* Thu Feb 02 2006 Andreas Bierfert <andreas.bierfert[AT]lowlatency.de>
5.4.0-17
- fixed description... www _is_ included now...

* Sun Jan 22 2006 Andreas Bierfert <andreas.bierfert[AT]lowlatency.de>
5.4.0-16
- include suggestions from Patrice Dumas

* Thu Jan 19 2006 Andreas Bierfert <andreas.bierfert[AT]lowlatency.de>
5.4.0-15
- revisit for fe inclusion
- include www

* Fri Dec 09 2005 Jesse Keating <jkeating@redhat.com>
- rebuilt

* Fri Sep 30 2005 Harald Hoyer <harald@faro.stuttgart.redhat.com> - 14
- fix for libwww's handling of multipart/byteranges content and possible
  stack overflow (bug #159597)

* Thu Mar 03 2005 Harald Hoyer <harald@redhat.com> 
- rebuilt

* Thu Feb 24 2005 Harald Hoyer <harald@redhat.com> - 5.4.0-12
- built with ssl

* Wed Feb 09 2005 Harald Hoyer <harald@redhat.com>
- rebuilt

* Tue Jun 15 2004 Elliot Lee <sopwith@redhat.com>
- rebuilt

* Tue Mar 02 2004 Elliot Lee <sopwith@redhat.com>
- rebuilt

* Fri Feb 13 2004 Elliot Lee <sopwith@redhat.com>
- rebuilt

* Thu Jan 29 2004 Harald Hoyer <harald@faro.stuttgart.redhat.com> - 5.4.0-8
- added zlib-devel dependency

* Thu Aug  7 2003 Elliot Lee <sopwith@redhat.com> 5.4.0-7
- Fix libtool, auto*

* Wed Jun 04 2003 Elliot Lee <sopwith@redhat.com>
- rebuilt

* Mon Feb 17 2003 Elliot Lee <sopwith@redhat.com> 5.4.0-5
- ppc64 fixes

* Wed Jan 29 2003 Harald Hoyer <harald@redhat.de> 5.4.0-4
- rebuilt

* Wed Jan 22 2003 Tim Powers <timp@redhat.com>
- rebuilt

* Mon Nov 18 2002 Tim Powers <timp@redhat.com>
- rebuild on all arches
- lib64'ize
- fix broken %%doc file list

* Tue Jul 23 2002 Harald Hoyer <harald@redhat.de> 5.4.0-1
- removed prestripping

* Fri Jun 21 2002 Tim Powers <timp@redhat.com>
- automated rebuild

* Thu May 23 2002 Tim Powers <timp@redhat.com>
- automated rebuild

* Wed Feb 20 2002 Harald Hoyer <harald@redhat.de>
- fixed --cflags (#59503)

* Wed Jan 23 2002 Harald Hoyer <harald@redhat.de> 5.3.2-4
- moved wwwconf.h in w3c-libwww subdir #58060
- added libpics.so.* #58433

* Mon Jan 07 2002 Harald Hoyer <harald@redhat.com> 5.3.2-2
- added wwwconf.h (#58060)

* Thu Dec 13 2001 Harald Hoyer <harald@redhat.com>
- fix for #55526

* Tue Jul 10 2001 Tim Powers <timp@redhat.com>
- make devel package version dependant

* Sun Jun 24 2001 Elliot Lee <sopwith@redhat.com>
- Bump release + rebuild for 7.2.

* Thu Jul 13 2000 Prospector <bugzilla@redhat.com>
- automatic rebuild

* Sun Jun 18 2000 Matt Wilson <msw@redhat.com>
- rebuilt for next release
- added patch to toplevel Makefile.am to honor DESTDIR
- use "make DESTDIR=$RPM_BUILD_ROOT install"

* Thu Aug 12 1999 Jeff Johnson <jbj@redhat.com>
- rebuild for 6.1.

* Mon Apr 12 1999 Jeff Johnson <jbj@redhat.com>
- repackage for Red Hat 6.0.
