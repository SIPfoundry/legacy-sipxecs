%global realname gen_server_mock
%global upstream sipxopenacd
%global debug_package %{nil}


Name:		erlang-%{realname}
Version:	0.0.5
Release:	1%{?dist}.g%{gitref}
Summary:	gen_server mocking library
Group:		Development/Libraries
License:	Freely redistributable without restriction
URL:		https://github.com/dannaaduna/gen_server_mock
Source0:	%{upstream}-%{realname}-%{version}-g%{gitref}.tar.gz
BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)
BuildRequires:	erlang-rebar
BuildRequires:	erlang-gen_leader
Requires:	erlang-erts >= R14B
Requires:	erlang-kernel >= R14B
Requires:	erlang-stdlib >= R14B
Requires:	erlang-gen_leader

%description
gen_server mocking library


%prep
%setup -q -n %{upstream}-%{realname}-%{gitref}

%build
REBAR_SHARED_DEPS=1 rebar compile skip_deps=true

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%{_libdir}/erlang/lib/%{realname}-%{version}/ebin
install -m 644 ebin/%{realname}.app %{buildroot}%{_libdir}/erlang/lib/%{realname}-%{version}/ebin
install -m 644 ebin/*.beam %{buildroot}%{_libdir}/erlang/lib/%{realname}-%{version}/ebin

%clean
rm -rf %{buildroot}


%files
%defattr(-,root,root,-)
%dir %{_libdir}/erlang/lib/%{realname}-%{version}
%dir %{_libdir}/erlang/lib/%{realname}-%{version}/ebin
%{_libdir}/erlang/lib/%{realname}-%{version}/ebin/%{realname}.app
%{_libdir}/erlang/lib/%{realname}-%{version}/ebin/*.beam

%changelog
* Mon Jan 21 2012 Jan Vincent Liwanag <jvliwanag@gmail.com>
- Initial release
