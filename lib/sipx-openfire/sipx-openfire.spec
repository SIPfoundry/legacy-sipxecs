Summary: Openfire XMPP Server
Name: sipx-openfire
Version: %{OPENFIRE_VERSION}
Release: %{OPENFIRE_RELEASE}
BuildRoot: %{_builddir}/%{name}-root
Source0: %{OPENFIRE_SOURCE}
Group: Applications/Communications
Vendor: Jive Software
Packager: Nortel
License: GPL
AutoReqProv: no
URL: http://www.igniterealtime.org/

%define prefix /opt
%define homedir %{prefix}/openfire

%description
Openfire is a leading Open Source, cross-platform IM server based on the
XMPP (Jabber) protocol. It has great performance, is easy to setup and use,
and delivers an innovative feature set.

Requires: unpack200
Requires: shadow-utils

Obsoletes: openfire

%prep
%setup -qcn %name-%version

%build

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p -m 755 $RPM_BUILD_ROOT%{homedir}
tar -xzf %{OPENFIRE_BINARY} -C $RPM_BUILD_ROOT%{prefix}

%clean
rm -rf $RPM_BUILD_ROOT

%pre
# If there is no daemon user, create it.
# Red Hat's useradd command adds the group automatically,
# but SUSE does not.
# The -f option means to create the group only if it does not already exist.
/usr/sbin/groupadd -f %{SIPXPBXGROUP}
if ! id -u %{SIPXPBXUSER} > /dev/null 2>&1 ; then
   /usr/sbin/useradd \
       -c "sipX service daemon" \
       -d %{_sysconfdir}/sipxpbx \
       -s /bin/bash \
       -g %{SIPXPBXGROUP} \
       %{SIPXPBXUSER} > /dev/null 2>&1
fi

%post
# Unpack any packed jar files.
for packed_jar in /opt/openfire/lib/*.jar.pack; do \
	jar=${packed_jar%.pack}; \
	unpack200 -r $packed_jar $jar; \
	chown %{SIPXPBXUSER}:%{SIPXPBXGROUP} $jar; \
done;

%files
%attr(775,%{SIPXPBXUSER},%{SIPXPBXGROUP}) %dir %{homedir}
%attr(-,%{SIPXPBXUSER},%{SIPXPBXGROUP}) %{homedir}/*
%attr(-,%{SIPXPBXUSER},%{SIPXPBXGROUP}) %{homedir}/.install4j/*
