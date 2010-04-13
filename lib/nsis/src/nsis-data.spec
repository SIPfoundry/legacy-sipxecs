Name: nsis-data
Version: 2.39
Release: 1
URL: http://nsis.sourceforge.net/
Source: http://downloads.sourceforge.net/nsis/nsis-%{version}.zip
Group: Development/Tools
License: zlib and BSD and CPL
BuildRequires: unzip
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildArch: noarch
Summary: Data and target files for the NSIS Windows installer build tool
%description
NSIS (Nullsoft Scriptable Install System) is a professional Open Source system to create Windows installers. It is designed to be as small and flexible as possible and is therefore very suitable for Internet distribution. This package contains data and target files for NSIS.

%prep
%setup -n nsis-%{version}

%build
# remove Win32-only files used at build time
rm -f Bin/LibraryLocal.exe Bin/GenPat.exe Bin/zip2exe.exe Bin/MakeLangId.exe

%install
# master dir stuff
if [ -d $RPM_BUILD_ROOT ]; then rm -rf $RPM_BUILD_ROOT; fi
mkdir -p $RPM_BUILD_ROOT

# install makensis data
mkdir -p ${RPM_BUILD_ROOT}%{_datadir}/doc/nsis
mkdir -p ${RPM_BUILD_ROOT}%{_datadir}/nsis
cp -fr Docs/ Examples/ ${RPM_BUILD_ROOT}%{_datadir}/doc/nsis
cp -fr Bin/ Contrib/ Include/ Menu/ Plugins/ Stubs/ ${RPM_BUILD_ROOT}%{_datadir}/nsis

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)
%doc COPYING
%{_datadir}/nsis/
%doc %{_datadir}/doc/nsis/
