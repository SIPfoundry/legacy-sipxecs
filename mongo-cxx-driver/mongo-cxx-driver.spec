# for better compatibility with SCL spec file
%global pkg_name mongo-cxx-driver

Name:           mongo-cxx-driver
Version:        2.6.7
Release:        1%{?dist}
Summary:        The mongoDb C++ client driver library and its include files
Group:          Development/Libraries
License:        AGPLv3 and zlib and ASL 2.0
URL:          	http://www.mongodb.org
Source0:        https://github.com/mongodb/%{pkg_name}/archive/%{pkg_name}-legacy-0.0-26compat-%{version}.tar.gz

BuildRequires:  scons
BuildRequires:  openssl-devel
BuildRequires:  boost-devel

# Mongodb must run on a little-endian CPU (see bug #630898)
ExcludeArch:    ppc ppc64 %{sparc} s390 s390x

Provides: libmongodb = %{version}-%{release}
Obsoletes: libmongodb < 2.6

%description
This package provides the shared library for the MongoDB legacy C++ Driver.

%package -n %{pkg_name}-devel
Summary:        MongoDB header files
Group:          Development/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

Provides: libmongodb-devel = 2.6.7-%{release}
Obsoletes: libmongodb-devel < 2.6

%description -n %{pkg_name}-devel
This package provides the header files for MongoDB legacy C++ driver.

%prep
%setup -q -n %{name}-legacy-0.0-26compat-%{version}

# CRLF -> LF
sed -i 's/\r//' README.md

# Put lib dir in correct place
# https://jira.mongodb.org/browse/SERVER-10049
sed -i -e "s@\$INSTALL_DIR/lib@\$INSTALL_DIR/%{_lib}@g" src/SConscript.client

%build
scons mongoclient \
        %{?_smp_mflags} \
        --sharedclient \
        --use-system-all \
        --prefix=%{buildroot}%{_prefix} \
        --extrapath=%{_prefix} \
        --usev8 \
        --ssl \
        --full

%install
scons install-mongoclient \
        %{?_smp_mflags} \
        --sharedclient \
        --use-system-all \
        --prefix=%{buildroot}%{_prefix} \
        --extrapath=%{_prefix} \
        --usev8 \
        --ssl \
        --full 
rm -f %{buildroot}%{_libdir}/libmongoclient.a
rm -f %{buildroot}%{_libdir}/../lib/libmongoclient.a


%files
%doc README.md APACHE-2.0.txt
%{_libdir}/libmongoclient.so

%files -n %{pkg_name}-devel
%{_includedir}
%{_libdir}/libmongoclient.so

%changelog
