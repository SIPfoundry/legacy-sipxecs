Summary:	A system-independent interface for user-level packet capture.
Name:		libpcap
Version:	1.0.0
Release:	1
License:	BSD
Group:		Libraries
Source:		%{name}-%{version}.tar.gz
Patch0:		%{name}-shared.patch
Patch1:		%{name}-any_device.patch
URL:		http://www.tcpdump.org/
BuildRequires:	bison
BuildRequires:	flex
Obsoletes:	libpcap0
BuildRoot:	/tmp/%{name}-buildroot

%description
Packet-capture library LIBPCAP 1.0.0
Now maintained by "The Tcpdump Group"
See http://www.tcpdump.org
Please send inquiries/comments/reports to tcpdump-workers@lists.tcpdump.org

%package devel
Summary:	A pcap library.
Group:		Development/Libraries
Requires:	%{name} = %{version}-%{release}
Obsoletes:	libpcap0-devel

%description devel
Packet-capture library LIBPCAP 1.0.0
Now maintained by "The Tcpdump Group"
See http://www.tcpdump.org
Please send inquiries/comments/reports to tcpdump-workers@lists.tcpdump.org

%prep
%setup -q
%patch0 -p1
%patch1 -p1

%build
%{__autoconf}
%configure --with-pcap=linux
%{__make}

%install
rm -rf $RPM_BUILD_ROOT
install -d $RPM_BUILD_ROOT%{_bindir}

%{__make} install DESTDIR=$RPM_BUILD_ROOT

# some packages want it... but sanitize somehow
# (don't depend on HAVE_{STRLCPY,SNPRINTF,VSNPRINTF} defines)
sed -e '390,396d;399,408d' pcap-int.h > $RPM_BUILD_ROOT%{_includedir}/pcap-int.h

%clean
rm -rf $RPM_BUILD_ROOT

%post   -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files
%defattr(644,root,root,755)
%doc CHANGES CREDITS LICENSE README
%attr(755,root,root) %{_libdir}/libpcap.so.*.*
%attr(755,root,root) %ghost %{_libdir}/libpcap.so.0
%{_mandir}/man5/pcap-savefile.5*
%{_mandir}/man7/pcap-*.7*

%files devel
%defattr(644,root,root,755)
%attr(755,root,root) %{_bindir}/pcap-config
%attr(755,root,root) %{_libdir}/libpcap.so
%attr(755,root,root) %{_libdir}/libpcap.a
%{_includedir}/pcap
%{_includedir}/pcap*.h
%{_mandir}/man1/pcap-config.1*
%{_mandir}/man3/pcap*.3*
