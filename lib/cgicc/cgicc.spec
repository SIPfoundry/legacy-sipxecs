Summary: GNU cgicc is a C++ class library for writing CGI applications
Name: cgicc
Version: 3.2.3
Release: 2
License: LGPL
Group: Internet/WWW/Servers
URL: http://www.cgicc.org/
Source0: %{name}-%{version}.tar.gz
Requires: libstdc++
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

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

%build
%configure
make

%install
make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf $RPM_BUILD_ROOT

# Since this package installs a shared library, we need to regenerate links
# to the library from short names.
%post
/sbin/ldconfig
ln -s -f /usr/lib/libcgicc.so.5.0.1 /usr/lib/libcgicc.so 

%postun
/sbin/ldconfig
rm -f /usr/lib/libcgicc.so

%files
%attr(755,root,root) /usr/bin/cgicc-config
%attr(755,root,root) /usr/lib/libcgicc.a
%attr(755,root,root) /usr/lib/libcgicc.la
%attr(755,root,root) /usr/lib/libcgicc.so
%attr(755,root,root) /usr/lib/libcgicc.so.5
%attr(755,root,root) /usr/lib/libcgicc.so.5.0.1

%files devel
%defattr(644,root,root,755) 
%doc /usr/doc/cgicc-3.2.3
/usr/include/cgicc

%changelog
