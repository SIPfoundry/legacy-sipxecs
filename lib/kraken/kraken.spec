Summary: Kraken XMPP IM Gateway
Name: kraken
Version: %{KRAKEN_VERSION}
Release: %{KRAKEN_RELEASE}
BuildRoot: %{_builddir}/%{name}-root
Source0: %{KRAKEN_SOURCE}
Group: Applications/Communications
Vendor: Daniel Henninger
Packager: Nortel
License: GPL
AutoReqProv: no
URL: http://kraken.blathersource.org/

%define prefix /opt
%define homedir %{prefix}/openfire

%description
Kraken is a java based XMPP gateway. It provides a mechanism for communicating, via XMPP,
with a large number of "legacy services", such as AIM, ICQ, MSN, etc. The single gateway
implementation houses a number of transport implementations to individual services.

%prep
%setup -qcn %name-%version

%build

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p -m 755 $RPM_BUILD_ROOT%{homedir}/plugins
cp %{KRAKEN_JAR} $RPM_BUILD_ROOT%{homedir}/plugins

%clean
rm -rf $RPM_BUILD_ROOT

%files
%attr(-,root,root) %{homedir}/plugins/*
