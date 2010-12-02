# This makes things much simpler
%if 0%{?suse_version} && ! 0%{?sles_version}
%define opensuse_version      %suse_version
%endif


Name:                         boost  
Version:                      1.39.0  
Release:                      1
Summary:                      Boost C++ Libraries  
License:                      BSD 3-Clause  
Group:                        Development/Libraries/C and C++  
Url:                          http://www.boost.org  
Buildroot:                    %{_tmppath}/%{name}-%{version}-build  
Source0:                      boost_1_39_0.tar.bz2

Buildrequires:                boost-jam = 3.1.17


Buildrequires:                gcc-c++
Buildrequires:                python-devel 
Buildrequires:                zlib-devel
Buildrequires:                libicu-devel

%if %{_vendor} == redhat
Buildrequires:                bzip2-devel
Buildrequires:                libX11-devel
%endif

# MANDRIVA
%if 0%{?mandriva_version}
Buildrequires:                bzip2
Buildrequires:                libbzip2-devel
BuildRequires:                XFree86-devel
%endif
  
%if 0%{?sles_version} == 9
Buildrequires:                bzip2
Buildrequires:                XFree86-devel
%endif

%if 0%{?sles_version} == 10
Buildrequires:                bzip2
%endif

%if 0%{?sles_version} >= 10
Buildrequires:                xorg-x11-devel
%endif

%if 0%{?sles_version} >= 11
Buildrequires:                libbz2-devel
%endif

# OPENSUSE
%if 0%{?opensuse_version}
Buildrequires:                libbz2-devel
Buildrequires:                xorg-x11-libX11-devel
%endif

  
%description  
Boost provides free peer-reviewed portable C++ source libraries. The  
emphasis is on libraries that work well with the C++ Standard Library.  
One goal is to establish "existing practice" and provide reference  
implementations so that the Boost libraries are suitable for eventual  
standardization. Some of the libraries have already been proposed for  
inclusion in the C++ Standards Committee's upcoming C++ Standard  
Library Technical Report.  
  
Although Boost was begun by members of the C++ Standards Committee  
Library Working Group, membership has expanded to include nearly two  
thousand members of the C++ community at large.  
  
This package contains the dynamic libraries. For development using  
Boost, you also need the boost-devel package. For documentation, see  
the boost-doc package.  
  
  
%package devel  
Summary:                      Development package for Boost C++  
Group:                        Development/Libraries/C and C++  
Requires:                     %{name} = %{version}  
  
%description devel  
This package contains all that is needed to develop/compile  
applications that use the Boost C++ libraries. For documentation see  
the package boost-doc.  
  
  
%prep  
%setup -q -n boost_1_39_0  
  
  
%build  
bjam --toolset=gcc stage  
  
%install  
rm -rf $RPM_BUILD_ROOT  
  
DESTDIR="%{?buildroot:%{buildroot}}"  
  
mkdir -p "$DESTDIR/%{_libdir}/"  
  
# libboost_regex-gcc42-mt-1_35.so.1.35.0  
cp -r stage/lib/libboost*.%{version} "$DESTDIR/%{_libdir}/"  
  
rm -f filelist.lib filelist.devel  
for x in $( find "$DESTDIR/%{_libdir}/" -not -type d )  
do  
    nname="$(basename "$x")"      # libboost_regex-gcc42-mt-1_35.so.1.35.0  
    sname="${nname%%.%version}"   # libboost_regex-gcc42-mt-1_35.so  
    lname="${nname%%%%-*}.so"     # libboost_regex.so  
  
    #echo "$nname -> $sname -> $lname"  
  
    echo "%{_libdir}/$nname" >> filelist.lib  
    echo "%{_libdir}/$sname" >> filelist.devel  
    echo "%{_libdir}/$lname" >> filelist.devel  
  
    ln -s "$nname" "$DESTDIR/%{_libdir}/$sname"  
    ln -s "$nname" "$DESTDIR/%{_libdir}/$lname"  
done  
  
  
mkdir -p "$DESTDIR/%{_includedir}"  
cp -r boost "$DESTDIR/%{_includedir}/"  
  
mkdir -p .backup  
cp -r index.html .backup  
cp -r doc .backup
cp -r more .backup  
find .backup -type f -exec chmod u=rw,go=r "{}" \;  
find .backup -type d -exec chmod u=rwx,go=rx "{}" \;  
  
%clean  
rm -rf $RPM_BUILD_ROOT  
  
%post  
/sbin/ldconfig  
  
%postun  
/sbin/ldconfig  
  
%files -f filelist.lib  
%defattr(-,root,root)  
%doc LICENSE_1_0.txt  
  
%files devel -f filelist.devel  
%defattr(-,root,root)  
%{_includedir}/boost  
%doc .backup/*  
  
%changelog
