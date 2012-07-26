Name: resiprocate
Version: 1.8.4
Release: 1
Summary: Resiprocate SIP Stack
License: Vovida Software License http://opensource.org/licenses/vovidapl.php
Group: Productivity/Telephony/SIP/Servers
Vendor: resiprocate.org
Packager: Daniel Pocock <daniel@pocock.com.au>
Url: http://www.resiprocate.org
Source: %name-%version.tar.gz
BuildRequires: openssl-devel >= 0.9.7
BuildRequires: popt
BuildRequires: boost-devel
Requires: openssl >= 0.9.7
Requires: chkconfig
Prefix: %_prefix
BuildRoot: %{_tmppath}/%name-%version-root

%description
The reSIProcate components, particularly the SIP stack, are in use in both
commercial and open-source products. The project is dedicated to maintaining
a complete, correct, and commercially usable implementation of SIP and a few
related protocols.

%package devel
Summary: Resiprocate development files
Group: Development/Libraries
Requires: %{name} = %{version}

%description devel
Resiprocate SIP Stack development files.

%prep
%setup -q

%build
%configure
make

%install
make DESTDIR=%buildroot install

%clean
[ ${RPM_BUILD_ROOT} != "/" ] && rm -rf ${RPM_BUILD_ROOT}

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files
%defattr(644,root,root,755)
%{_sysconfdir}/repro.conf
%{_libdir}/*.so
%{_sbindir}/*

%files devel
%defattr(644,root,root,755)
%{_includedir}/rutil/*
%{_includedir}/repro/*
%{_includedir}/resip/*
%{_libdir}/*.a
%{_libdir}/*.la
%{_mandir}/man3/*.gz
%{_mandir}/man8/*.gz

%changelog
* Wed Jul  25 2012 Douglas Hubler <douglas@hubler.us> -
- Update for new files in 1.8.4 release
* Sun Aug  7 2011 Daniel Pocock <daniel@pocock.com.au> -
- Initial build based on autotools

