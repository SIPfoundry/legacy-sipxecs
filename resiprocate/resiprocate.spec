Name: resiprocate
Version: 1.9.0~rc2
Release: 1%{?dist}
Summary: SIP and TURN stacks, with SIP proxy and TURN server implementations
License: VSL
Url: http://www.resiprocate.org
Source: https://www.resiprocate.org/files/pub/reSIProcate/releases/%name-%version.tar.gz
BuildRequires: libtool automake autoconf
BuildRequires: boost-devel
%if 0%{?fedora} >= 18
BuildRequires: db4-cxx-devel
%endif
BuildRequires: db4-devel
BuildRequires: openssl-devel >= 0.9.8
Requires: openssl >= 0.9.8
Requires: chkconfig
%if 0%{?fedora} < 17
Requires(preun): initscripts
%endif

%description
The reSIProcate components, particularly the SIP stack, are in use in both
commercial and open-source products. The project is dedicated to maintaining
a complete, correct, and commercially usable implementation of SIP and a few
related protocols.

%package devel
Summary: reSIProcate development files
Requires: %{name} = %{version}-%{release}

%description devel
Resiprocate SIP Stack development files.

%prep
%setup -q

%build
export LDFLAGS="${LDFLAGS} -L%{_libdir}/libdb4"
CXXFLAGS="%{optflags} -I%{_includedir}/libdb4" %configure
sed -i 's|^hardcode_libdir_flag_spec=.*|hardcode_libdir_flag_spec=""|g' libtool
sed -i 's|^runpath_var=LD_RUN_PATH|runpath_var=DIE_RPATH_DIE|g' libtool
make %{?_smp_mflags}

%install
make DESTDIR=%{buildroot} install

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig


%files
%defattr(644,root,root,755)
%{_libdir}/*.so

%files devel
%defattr(644,root,root,755)
%{_includedir}/rutil/*
%{_includedir}/resip/*
%{_libdir}/*.a
%{_libdir}/*.la
%{_mandir}/man3/*.gz


%changelog
* Sat Feb  8 2014 Ionut Slaveanu islaveanu@ezuce.com -
- Update for new files in 1.9.0~rc2-1 release
* Sat Nov 24 2012 Daniel Pocock <daniel@pocock.com.au> - 1.9.0~rc2-1
- Produce multiple packages for stack/libs, daemons, sipdialer
- Initial build based on autotools

