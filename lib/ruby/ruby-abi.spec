Name:           ruby-abi
Version:        1.8
Release:        1%{?dist}
Summary:        Ruby compatibility package

Group:          Development/Languages

License:        GPL
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildArch:      noarch
BuildRequires:  ruby ruby-devel
Requires:       ruby
Provides:       ruby(abi) = 1.8

%description
Provides ruby(abi), which is included in new Fedora Core ruby packages, but not in the ruby package for RHEL4

%prep

%build
export CFLAGS="$RPM_OPT_FLAGS"


%install
rm -rf $RPM_BUILD_ROOT

 
%check


%clean
rm -rf $RPM_BUILD_ROOT


%files


%changelog
