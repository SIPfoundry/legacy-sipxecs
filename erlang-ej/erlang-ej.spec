%global realname ej
%global upstream seth
%global debug_package %{nil}
%global patchnumber 0


Name:		erlang-%{realname}
Version:	0.0.2
Release:	2%{?dist}.g%{gitref}
Summary:	An Erlang JSON helper library
Group:		Development/Libraries
License:	Freely redistributable without restriction
URL:		https://github.com/seth/ej
Source0:	%{upstream}-%{realname}-%{version}-%{patchnumber}-g%{gitref}.tar.gz
Patch1:		erlang-ej-0001-Replace-git-based-app-vsn.patch
BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)
BuildRequires:	erlang-rebar
Requires:	erlang-erts >= R14B
Requires:	erlang-kernel >= R14B
Requires:	erlang-stdlib >= R14B


%description
The ej module makes it easier to work with Erlang terms representing JSON in the format returned by jiffy, mochijson2, or ejson.


%prep
%setup -q -n %{upstream}-%{realname}-%{gitref}
%patch1 -p1

%build
rebar compile -v


%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%{_libdir}/erlang/lib/%{realname}-%{version}/ebin
mkdir -p %{buildroot}%{_libdir}/erlang/lib/%{realname}-%{version}/include
install -m 644 ebin/%{realname}.app %{buildroot}%{_libdir}/erlang/lib/%{realname}-%{version}/ebin
install -m 644 ebin/*.beam %{buildroot}%{_libdir}/erlang/lib/%{realname}-%{version}/ebin
install -m 644 include/%{realname}.hrl %{buildroot}%{_libdir}/erlang/lib/%{realname}-%{version}/include

%clean
rm -rf %{buildroot}


%files
%defattr(-,root,root,-)
%dir %{_libdir}/erlang/lib/%{realname}-%{version}
%dir %{_libdir}/erlang/lib/%{realname}-%{version}/ebin
%dir %{_libdir}/erlang/lib/%{realname}-%{version}/include
%{_libdir}/erlang/lib/%{realname}-%{version}/ebin/%{realname}.app
%{_libdir}/erlang/lib/%{realname}-%{version}/ebin/*.beam
%{_libdir}/erlang/lib/%{realname}-%{version}/include/*.hrl

%changelog
* Mon Nov 26 2012 Jan Vincent Liwanag <jvliwanag@gmail.com>
- Initial release
