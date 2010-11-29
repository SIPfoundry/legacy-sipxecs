Name:                         boost-jam
Version:                      3.1.17
Release:                      3.1
AutoReqProv:    on
Summary:                      An Enhanced Make Replacement
License:                      BSD
Group:                        Development/Tools
Url:                          http://www.boost.org/
Buildroot:                    %{_tmppath}/%{name}-%{version}-build
Source:                       %{name}-%{version}.tgz
Source2:                      test.tgz

%description
Boost Jam is a build tool based on FTJam, which in turn is based on
Perforce Jam. It contains significant improvements made to facilitate
its use in the Boost Build System, but should be backward compatible
with Perforce Jam.



Authors:
--------
    Perforce Jam : Cristopher Seiwald
    FT Jam    : David Turner
    Boost Jam : David Abrahams

%prep
%setup -q
find . -type f|xargs chmod -R u+w
chmod -x images/*.png

%build
export RPM_OPT_FLAGS="$RPM_OPT_FLAGS -Wall -fno-strict-aliasing"
export CFLAGS="$RPM_OPT_FLAGS"
LOCATE_TARGET=bin ./build.sh gcc --symbols
# Trivial test: -- Documented used of bjam -v: Print the version of jam and exit:
bin/bjam -v
ln  -s bin bin.linux
cd ..
tar zxvf %{SOURCE2}
ln -s %{name}-%{version} src
cd test
sh test.sh || if [ $? -gt 5 ]; then sh test.sh;fi

%install
mkdir -p %{buildroot}%{_bindir}
mkdir -p $RPM_BUILD_ROOT%{_docdir}/%{name}-%{version}
install -m 755 bin/bjam %{buildroot}%{_bindir}/bjam-%{version}
ln -sf bjam-%{version} %{buildroot}%{_bindir}/bjam
ln -sf bjam-%{version} %{buildroot}%{_bindir}/jam

%files
%defattr(-,root,root)
%attr(755,root,root) %{_bindir}/*
%doc *.html images

%clean
rm -rf %{buildroot}
%changelog
