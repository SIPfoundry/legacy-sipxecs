%global realname mimetypes
%global upstream spawngrid
%global debug_package %{nil}

Name:		erlang-%{realname}
Version:	0.9
Release:	1%{?dist}.g%{gitref}
Summary:	An Erlang library to fetch MIME extension/name mappings
Group:		Development/Libraries
License:	Freely redistributable without restriction
URL:		https://github.com/spawngrid/mimetypes
Source0:	%{upstream}-%{realname}-%{version}-g%{gitref}.tar.gz
Patch1:		erlang-mimetypes-0001-Replace-git-vsn-with-fixed-value.patch
BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)
BuildRequires:	erlang-rebar
BuildRequires:  erlang-parsetools
Requires:	erlang-erts >= R14B
Requires:	erlang-kernel >= R14B
Requires:	erlang-stdlib >= R14B


%description
mimetypes is an Erlang library to fetch MIME extension/name mappings.


%prep
%setup -q -n %{upstream}-%{realname}-%{gitref}
%patch1 -p1

%build
rebar compile -v


%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%{_libdir}/erlang/lib/%{realname}-%{version}/ebin
mkdir -p %{buildroot}%{_libdir}/erlang/lib/%{realname}-%{version}/priv
install -m 644 ebin/%{realname}.app %{buildroot}%{_libdir}/erlang/lib/%{realname}-%{version}/ebin
install -m 644 ebin/*.beam %{buildroot}%{_libdir}/erlang/lib/%{realname}-%{version}/ebin
install -m 644 priv/mime.types %{buildroot}%{_libdir}/erlang/lib/%{realname}-%{version}/priv

%clean
rm -rf %{buildroot}


%files
%defattr(-,root,root,-)
%dir %{_libdir}/erlang/lib/%{realname}-%{version}
%dir %{_libdir}/erlang/lib/%{realname}-%{version}/ebin
%dir %{_libdir}/erlang/lib/%{realname}-%{version}/priv
%{_libdir}/erlang/lib/%{realname}-%{version}/ebin/%{realname}.app
%{_libdir}/erlang/lib/%{realname}-%{version}/ebin/*.beam
%{_libdir}/erlang/lib/%{realname}-%{version}/priv/mime.types

%changelog
* Mon Nov 28 2012 Jan Vincent Liwanag <jvliwanag@gmail.com>
- Initial release
