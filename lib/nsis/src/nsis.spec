Name: nsis
Version: 2.39
Release: 1
URL: http://nsis.sourceforge.net/
Source: http://downloads.sourceforge.net/nsis/%{name}-%{version}-src.tar.bz2
# 64-bit fixes derived from Debian patch
# Andreas Jochens <aj@andaco.de>, Andreas Barth <aba@not.so.argh.org>,
# Steve Langasek <vorlon@debian.org>, Paul Wise <pabs@debian.org>
Patch: nsis.patch
Group: Development/Tools
License: zlib and BSD and CPL
BuildRequires: scons >= 0.98
Requires: nsis-data = %{version}
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Summary: NSIS is an Open Source installer build tool for Windows applications
%description
NSIS (Nullsoft Scriptable Install System) is a professional Open Source system to create Windows installers. It is designed to be as small and flexible as possible and is therefore very suitable for Internet distribution.

%prep
%setup -n nsis-%{version}-src
%patch

%build
scons PREFIX=%{_prefix} PREFIX_DEST=${RPM_BUILD_ROOT} PREFIX_CONF=%{_sysconfdir} SKIPSTUBS=all SKIPPLUGINS=all SKIPUTILS='Library/RegTool,UIs,Makensisw,zip2exe,MakeLangId,NSIS Menu' SKIPMISC=all VERSION=%{version} STRIP=false

%install
# master dir stuff
if [ -d $RPM_BUILD_ROOT ]; then rm -rf $RPM_BUILD_ROOT; fi
mkdir -p $RPM_BUILD_ROOT

# install makensis binary
scons PREFIX=%{_prefix} PREFIX_DEST=${RPM_BUILD_ROOT} PREFIX_CONF=%{_sysconfdir} SKIPSTUBS=all SKIPPLUGINS=all SKIPUTILS='Library/RegTool,UIs,Makensisw,zip2exe,MakeLangId,NSIS Menu' SKIPMISC=all VERSION=%{version} STRIP=false install

# remove files duplicated from nsis-data
rm -rf $RPM_BUILD_ROOT%{_datadir}/nsis
rm -rf $RPM_BUILD_ROOT%{_datadir}/doc/nsis/Docs/StrFunc
rm -rf $RPM_BUILD_ROOT%{_datadir}/doc/nsis/Examples

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)
%{_bindir}/makensis
%{_bindir}/LibraryLocal
%{_bindir}/GenPat
%config(noreplace) %{_sysconfdir}/nsisconf.nsh
%doc %{_datadir}/doc/nsis/COPYING
%doc %{_datadir}/doc/nsis/Docs/*

