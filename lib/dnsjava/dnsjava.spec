# Copyright (c) 2000-2007, JPackage Project
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the
#    distribution.
# 3. Neither the name of the JPackage Project nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

%define javadir         %{_datadir}/java
%define javadocdir      %{_datadir}/javadoc
%define section         free

# -----------------------------------------------------------------------------

Summary:        dnsjava reference implementation
Name:           dnsjava
Version:        2.0.6
Release:        1
Group:          Development/Java
License:        BSD License
URL:            http://www.dnsjava.org
BuildArch:      noarch
Source0:        http://www.dnsjava.org/download/dnsjava-2.0.6.tar.gz
Patch0:         0001-Remove-building-SUN-JVM-DNS-provider.patch
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root

BuildRequires:  jpackage-utils >= 0:1.7.3
BuildRequires:  java-devel >= 0:1.5.0
BuildRequires:  ant >= 0:1.6.5
Requires:       log4j >= 0:1.2

%description
dnsjava is an implementation of DNS in Java.  It supports all defined record
types (including the DNSSEC types), and unknown types.  It can be used for
queries, zone transfers, and dynamic updates.  It includes a cache which can be
used by clients, and an authoritative only server.  It supports TSIG
authenticated messages, partial DNSSEC verification, and EDNS0.  It is fully
thread safe.  It can be used to replace the native DNS support in Java.

# -----------------------------------------------------------------------------

%package javadoc
Group:          Documentation
Summary:        Javadoc for %{name}

%description javadoc
Javadoc for %{name}.

# -----------------------------------------------------------------------------

%prep
rm -rf $RPM_BUILD_ROOT
%setup -q
%patch -p1

# -----------------------------------------------------------------------------

%build
ant -Dbuild.sysclasspath=only jar docs

# -----------------------------------------------------------------------------

%install
rm -rf $RPM_BUILD_ROOT

# jar
install -d $RPM_BUILD_ROOT%{javadir}/%{name}
# install jars to $RPM_BUILD_ROOT%{javadir}/%{name} (as %{name}-%{version}.jar)
install -pm 644 %{name}-%{version}.jar \
  $RPM_BUILD_ROOT%{_javadir}/%{name}/%{name}-%{version}.jar
(cd $RPM_BUILD_ROOT%{javadir}/%{name} && for jar in *-%{version}.jar; do ln -sf ${jar} `echo $jar| sed  "s|-%{version}||g"`; done)

# javadoc
install -d $RPM_BUILD_ROOT%{javadocdir}/%{name}-%{version}/
cp -pr doc/* $RPM_BUILD_ROOT%{javadocdir}/%{name}-%{version}/
(cd $RPM_BUILD_ROOT%{javadocdir} && ln -sf %{name}-%{version} %{name})

# -----------------------------------------------------------------------------

%clean
rm -rf $RPM_BUILD_ROOT

# -----------------------------------------------------------------------------

%files
%defattr(0644,root,root,0755)
%{javadir}/*

%files javadoc
%defattr(0644,root,root,0755)
%{javadocdir}/%{name}-%{version}
%{javadocdir}/%{name}

# -----------------------------------------------------------------------------

%changelog
* Thu May 29 2008 Damian Krzeminski <damian at pingtel.com> 0:2.0.6-1jpp
- Initial build

