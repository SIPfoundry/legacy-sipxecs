%define real_version 1.7.0

Summary:	JavaScript interpreter and libraries
Name:	js
Version:	1.70
Release:	8%{?dist}
# The sources are triple licensed, but when we link against readline which is
# GPL, the result can only be GPL.
%if 0%{?_without_readline:1}
License:	GPLv2+ or LGPLv2+ or MPLv1.1
%else
License:	GPLv2+
%endif
Group:	Development/Languages
URL:		http://www.mozilla.org/js/
Source:	http://ftp.mozilla.org/pub/mozilla.org/js/js-%{real_version}.tar.gz
Patch0:	js-1.7.0-make.patch
Patch1:	js-shlib.patch
Patch2:	js-1.5-va_copy.patch
Patch3:	js-ldflags.patch
Patch4:	js-1.7.0-threadsafe.patch
Patch5:	js-1.60-ncurses.patch
Provides:	libjs = %{version}-%{release}
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root

%if 0%{?suse_version}
Buildrequires: mozilla-nspr-devel
%else
Buildrequires:	nspr-devel
%endif

Buildrequires:	readline-devel, ncurses-devel


%description
JavaScript is the Netscape-developed object scripting language used in millions
of web pages and server applications worldwide. Netscape's JavaScript is a
superset of the ECMA-262 Edition 3 (ECMAScript) standard scripting language,
with only mild differences from the published standard.


%package devel
Summary: Header files, libraries and development documentation for %{name}
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}
Requires: pkgconfig
Provides: libjs-devel = %{version}-%{release}

%description devel
This package contains the header files, static libraries and development
documentation for %{name}. If you like to develop programs using %{name},
you will need to install %{name}-devel.


%prep
%setup -q -n %{name}
%patch0 -p1 -b .make
%patch1 -p0 -b .shlib
%patch2 -p1 -b .vacopy
%patch3 -p0 -b .ldflags
%patch4 -p1 -b .threadsafe
%patch5 -p1 -b .ncurses

# Create pkgconfig file
%{__cat} > libjs.pc << 'EOF'
prefix=%{_prefix}
exec_prefix=%{_prefix}
libdir=%{_libdir}
includedir=%{_includedir}

Name: libjs
Description: JS library
Requires: nspr
Version: %{real_version}
Libs: -L${libdir} -ljs
Cflags: -DXP_UNIX=1 -DJS_THREADSAFE=1 -I${includedir}
EOF


%build
export BUILD_OPT=1
%{__make} %{?_smp_mflags} -C src -f Makefile.ref \
	JS_THREADSAFE="1" \
	XCFLAGS="%{optflags} -fPIC" \
	BUILD_OPT="1" \
%if 0%{!?_without_readline:1}
	JS_READLINE="1"
%endif


%install
%{__rm} -rf %{buildroot}
%{__mkdir_p} %{buildroot}%{_bindir} \
	%{buildroot}%{_libdir}/pkgconfig \
	%{buildroot}%{_includedir}
%{__install} -m 0755 src/Linux_All_OPT.OBJ/{js,jscpucfg} \
	%{buildroot}%{_bindir}/
%{__install} -m 0644 src/Linux_All_OPT.OBJ/libjs.a \
	%{buildroot}%{_libdir}/
%{__install} -m 0755 src/Linux_All_OPT.OBJ/libjs.so \
	%{buildroot}%{_libdir}/
%{__mv} %{buildroot}%{_libdir}/libjs.so %{buildroot}%{_libdir}/libjs.so.1
%{__ln_s} -nf libjs.so.1 %{buildroot}%{_libdir}/libjs.so
%{__install} -m 0644 src/js*.h src/js.msg src/*.tbl \
	src/Linux_All_OPT.OBJ/jsautocfg.h \
	%{buildroot}%{_includedir}/
%{__install} -m 0644 libjs.pc \
	%{buildroot}%{_libdir}/pkgconfig/


%clean
%{__rm} -rf %{buildroot}


%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%defattr(-,root,root,-)
%doc src/README.html
%{_bindir}/js
%{_libdir}/*.so.*

%files devel
%defattr(-,root,root,-)
%{_bindir}/jscpucfg
%{_libdir}/pkgconfig/*.pc
%{_libdir}/*.so
%{_libdir}/*.a
%{_includedir}/js*.h
%{_includedir}/*.tbl
%{_includedir}/js.msg


%changelog
* Sun Aug 2 2009 Pavel Alexeev <Pahan@Hubbitus.info> - 1.70-8
- Reformat spec with tabs.
- By report of Thomas Sondergaard (BZ#511162) Add -DXP_UNIX=1 -DJS_THREADSAFE=1 flags and nspr requires into libjs.pc

* Fri Jul 24 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.70-7
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Fri May 29 2009 Dan Horak <dan[at]danny.cz> 1.70-6
- update the va_copy patch for s390x

* Thu Apr  9 2009 Matthias Saou <http://freshrpms.net/> 1.70-5
- Update description (#487903).

* Wed Feb 25 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org>
- Rebuilt for https://fedoraproject.org/wiki/Fedora_11_Mass_Rebuild

* Wed Jun  4 2008 Jon McCann <jmccann@redhat.com> - 1.70-3
- Add two missing files (#449715)

* Wed Feb 27 2008 Tom "spot" Callaway <tcallawa@redhat.com> - 1.70-2
- Rebuild for perl 5.10 (again)

* Sun Feb  3 2008 Matthias Saou <http://freshrpms.net/> 1.70-1
- Update to 1.7.0, as 1.70 to avoid introducing an epoch for now...
- Remove no longer provided perlconnect parts.

* Thu Jan 24 2008 Tom "spot" Callaway <tcallawa@redhat.com> 1.60-6
- BR: perl(ExtUtils::Embed)

* Sun Jan 20 2008 Tom "spot" Callaway <tcallawa@redhat.com> 1.60-5
- rebuild for new perl

* Wed Aug 22 2007 Matthias Saou <http://freshrpms.net/> 1.60-4
- Rebuild for new BuildID feature.

* Mon Aug  6 2007 Matthias Saou <http://freshrpms.net/> 1.60-3
- Update License field.
- Add perl(ExtUtils::MakeMaker) build requirement to pull in perl-devel.

* Fri Feb  2 2007 Matthias Saou <http://freshrpms.net/> 1.60-2
- Include jsopcode.tbl and js.msg in devel (#235481).
- Install static lib mode 644 instead of 755.

* Fri Feb  2 2007 Matthias Saou <http://freshrpms.net/> 1.60-1
- Update to 1.60.
- Rebuild in order to link against ncurses instead of termcap (#226773).
- Add ncurses-devel build requirement and patch s/termcap/ncurses/ in.
- Change mode of perl library from 555 to 755 (#224603).

* Mon Aug 28 2006 Matthias Saou <http://freshrpms.net/> 1.5-6
- Fix pkgconfig file (#204232 & dupe #204236).

* Mon Jul 24 2006 Matthias Saou <http://freshrpms.net/> 1.5-5
- FC6 rebuild.
- Enable JS_THREADSAFE in the build (#199696), add patch and nspr build req.

* Mon Mar  6 2006 Matthias Saou <http://freshrpms.net/> 1.5-4
- FC5 rebuild.

* Thu Feb  9 2006 Matthias Saou <http://freshrpms.net/> 1.5-3
- Rebuild for new gcc/glibc.

* Mon Jan 30 2006 Matthias Saou <http://freshrpms.net/> 1.5-2
- Fix .pc file.

* Thu Jan 26 2006 Matthias Saou <http://freshrpms.net/> 1.5-1
- Update to 1.5.0 final.
- Spec file cleanups.
- Move docs from devel to main, since we need the license there.
- Remove no longer needed js-perlconnect.patch.
- Update js-1.5-va_copy.patch.
- Include a pkgconfig file (#178993).

* Tue Apr 19 2005 Ville Skyttä <ville.skytta at iki.fi> - 1.5-0.rc6a.6
- Link shared lib with libperl.

* Fri Apr  7 2005 Michael Schwendt <mschwendt[AT]users.sf.net>
- rebuilt

* Mon Feb 14 2005 David Woodhouse <dwmw2@infradead.org> - 1.5-0.rc6a.4
- Take js-va_copy.patch out of %%ifarch x86_64 so it fixes the PPC build too

* Sun Feb 13 2005 Thorsten Leemhuis <fedora at leemhuis dot info> - 1.5-0.rc6a.3
- Add js-va_copy.patch to fix x86_64; Patch was found in a Mandrake srpm

* Sat Dec 11 2004 Ville Skyttä <ville.skytta at iki.fi> - 1.5-0.rc6a.2
- Include perlconnect.
- Include readline support, rebuild using "--without readline" to disable.
- Add libjs* provides for upstream compatibility.
- Install header files in %%{_includedir} instead of %%{_includedir}/js.

* Tue Jun 15 2004 Matthias Saou <http://freshrpms.net> 1.5-0.rc6a
- Update to 1.5rc6a.

* Tue Mar 02 2004 Dag Wieers <dag@wieers.com> - 1.5-0.rc6
- Initial package. (using DAR)

