Name: jasperserver
Version: 5.2.0
Release: 1

Summary: Jasperserver reports war file
License: AGPL
Group: Telcommunications
Vendor: jaspersoft
BuildArch: noarch
Url: http://community.jaspersoft.com/project/jasperreports-server

Source: %name-%version.tar.gz

Prefix: %_prefix
BuildRoot: %{_tmppath}/%name-%version-root

%define homedir %{_datadir}/java/sipXecs/jasperserver/

%description
jasperserver reports, community edition

%prep
%setup -qn jasperserver

%build

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p -m 755 $RPM_BUILD_ROOT%{homedir}
cp -r . $RPM_BUILD_ROOT%{homedir}

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(644,root,root,755)
%{_datadir}/java/sipXecs/jasperserver/*
