Summary:	A network traffic monitoring tool.
Name:		tcpdump
Version:	4.0.0
Release:	1
License:	BSD
Group:		Applications/Networking
Source:		%{name}-%{version}.tar.gz
URL:		http://www.tcpdump.org/
BuildRequires:	automake
BuildRequires:	libpcap-devel >= 1.0.0
BuildRequires:	openssl-devel >= 0.9.7d
Requires:	libpcap >= 1.0.0
BuildRoot:	/tmp/%{name}-buildroot

%description
A network traffic monitoring tool.
Now maintained by "The Tcpdump Group"
See http://www.tcpdump.org
Please send inquiries/comments/reports to tcpdump-workers@lists.tcpdump.org

%prep
%setup -q

%build
%configure
%{__make}

%install
rm -rf $RPM_BUILD_ROOT
%{__make} install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(644,root,root,755)
%doc CHANGES CREDITS LICENSE README
%attr(755,root,root) %{_sbindir}/tcpdump*
%{_mandir}/man1/*
