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

# disable JAR repacking
%define __jar_repack %{nil}

# -----------------------------------------------------------------------------

Summary:        Jasper Reports dependencies for sipXconfig
Name:           sipx-jasperreports-deps
Version:        1.0.0
Release:        2
Group:          Development/Java
License:        LGPL
URL:            http://jasperforge.org/
BuildArch:      noarch
Source0:        %{name}-%{version}-%{release}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root


%description
Libraries required for integrating Jasper Reports into sipXconfig.  Installs the
following jars:
jasperreports-3.0.0.jar
itext_1.5.4.jar
jakarta-poi-3.0.2.jar
jfreechart-1.0.10.jar
jcommon-1.0.13.jar

# -----------------------------------------------------------------------------

%prep
rm -rf $RPM_BUILD_ROOT

%setup -n deps-jars

# -----------------------------------------------------------------------------

%install
rm -rf $RPM_BUILD_ROOT

# jar
install -d $RPM_BUILD_ROOT%{javadir}/%{name}
# install jars to $RPM_BUILD_ROOT%{javadir}/%{name} (as %{name}-%{version}.jar)
install -pm 644 *.jar \
  $RPM_BUILD_ROOT%{_javadir}/%{name}
(cd $RPM_BUILD_ROOT%{javadir}/%{name} && for jar in *.jar; do ln -sf ${jar} `echo $jar| sed -e "s/[_-][.0-9]*\.jar$/.jar/"`; done)
# -----------------------------------------------------------------------------

%clean
rm -rf $RPM_BUILD_ROOT

# -----------------------------------------------------------------------------

%files
%defattr(0644,root,root,0755)
%{javadir}/*


# -----------------------------------------------------------------------------

%changelog
* Tue Oct 28 2008 Damian Krzeminski <kthorley at nortel.com> 0:1.0.0-2
- Fix links to jakarta-poi

* Tue Oct 28 2008 Kevin Thorley <kthorley at nortel.com> 0:1.0.0-1
- Initial build
