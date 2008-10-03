Summary: GNU cgicc is a C++ class library for writing CGI applications
Name: cgicc
Version: 3.2.7
Release: 1
License: LGPL
Group: Internet/WWW/Servers
URL: http://www.cgicc.org/
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: gcc-c++
Patch0: cgicc.m4.patch

%package devel
Requires: %name
Group: Development/Libraries
Vendor: SIPfoundry
Summary: Header files for %name 
URL: http://www.cgicc.org/

%description
GNU cgicc is a C++ class library that greatly simplifies the creation of CGI applications for the World Wide Web. cgicc performs the following functions:

  * Parses both GET and POST form data transparently.
  * Provides string, integer, floating-point and single- and multiple-choice retrieval methods for form data.
  * Provides methods for saving and restoring CGI environments to aid in application debugging.
  * Provides full on-the-fly HTML generation capabilities, with support for cookies.
  * Supports HTTP file upload.
  * Compatible with FastCGI.

%description devel
Header files and documentation for the CGICC C++ class library. 

%prep
%setup -q
%patch0 -p0

%build
%configure --enable-demos=no
make

%install
make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf $RPM_BUILD_ROOT

# Since this package installs a shared library, we need to regenerate links
# to the library from short names.
%post
/sbin/ldconfig
ln -s -f %{_libdir}/libcgicc.so.5.0.2 %{_libdir}/libcgicc.so 

%postun
/sbin/ldconfig
rm -f %{_libdir}/libcgicc.so

%files
%attr(755,root,root) /usr/bin/cgicc-config
%attr(755,root,root) %{_libdir}/libcgicc*

%files devel
%defattr(644,root,root,755) 
%doc /usr/doc/cgicc-%{version}
/usr/include/cgicc
/usr/share/aclocal/cgicc.m4
