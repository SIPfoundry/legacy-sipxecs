Summary: Openfire XMPP Server
Name: sipx-openfire
Version: 3.6.4
Release: 3
BuildRoot: %{_builddir}/%{name}-root
Source: openfire_3_6_4.tar.gz
Group: Applications/Communications
BuildArch: noarch
Vendor: Jive Software
Packager: SIPfoundry
License: GPL
AutoReqProv: no
URL: http://www.igniterealtime.org/

%define prefix /opt
%define homedir %{prefix}/openfire

Requires: java-devel
Requires: shadow-utils
Requires: sipxcommserverlib
Obsoletes: openfire

%description
Openfire is a leading Open Source, cross-platform IM server based on the
XMPP (Jabber) protocol. It has great performance, is easy to setup and use,
and delivers an innovative feature set.

%prep
%setup -qn openfire

%build

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p -m 755 $RPM_BUILD_ROOT%{prefix}
cp -r . $RPM_BUILD_ROOT%{homedir}

%clean
rm -rf $RPM_BUILD_ROOT

%pre

%post
# Unpack any packed jar files.
for packed_jar in /opt/openfire/lib/*.jar.pack; do \
	jar=${packed_jar%.pack}; \
	unpack200 -r $packed_jar $jar; \
	chown sipxchange:sipxchange $jar; \
done;

%files
%attr(775,sipxchange,sipxchange) %dir %{homedir}
%attr(-,sipxchange,sipxchange) %{homedir}/*
%attr(-,sipxchange,sipxchange) %{homedir}/.install4j/*
