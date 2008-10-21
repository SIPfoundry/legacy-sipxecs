#
# spec file for package rubygems (Version 1.3.0-1)
#

# norootforbuild
# neededforbuild  ruby ruby-devel

Name:           rubygems
Version:        1.3.0
Release:        1
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
License:        Other uncritical OpenSource License
Group:          Development/Languages/Ruby
BuildRequires:  ruby-devel
Requires:       ruby-devel
Provides:       rubygems_with_buildroot_patch
#
URL:            http://rubyforge.org/projects/rubygems/
Summary:        The Ruby standard for publishing and managing third party libraries.
BuildArch: 	noarch
Source:         rubygems-%{version}.tgz
Patch:          rubygems_buildroot.patch
Obsoletes:	ruby-gems ruby-gems-devel ruby-gems-debuginfo


%description
RubyGems is the Ruby standard for publishing and managing third party libraries.

%prep
%setup -n %{name}-%{version}
%patch
find test -type f -perm 0755 -print0 | xargs chmod -v a-x

%build

%install
GEM_HOME=%{buildroot}%{_libdir}/ruby/gems/%{rb_ver}/ \
    ruby -rvendor-specific setup.rb --destdir=%{buildroot}
%{__install} -D -m 0755 %{S:1} %{buildroot}%{_bindir}/gem_build_cleanup

%clean
%{__rm} -rf %{buildroot};

%files
%defattr(-,root,root)
%doc ChangeLog GPL.txt LICENSE.txt README TODO
%doc test/ pkgs/
%{_bindir}/gem_build_cleanup
%{_bindir}/gem
%dir %{_libdir}/ruby/vendor_ruby/%{rb_ver}/rbconfig/
%{_libdir}/ruby/vendor_ruby/%{rb_ver}/rbconfig/datadir.rb
%{_libdir}/ruby/vendor_ruby/%{rb_ver}/*ubygems.rb
%{_libdir}/ruby/vendor_ruby/%{rb_ver}/rubygems/
%dir %{_libdir}/ruby/gems/
%dir %{_libdir}/ruby/gems/%{rb_ver}
%dir %{_libdir}/ruby/gems/%{rb_ver}/cache/
%dir %{_libdir}/ruby/gems/%{rb_ver}/doc/
%dir %{_libdir}/ruby/gems/%{rb_ver}/gems/
%dir %{_libdir}/ruby/gems/%{rb_ver}/specifications/
%{_libdir}/ruby/gems/%{rb_ver}/doc/rubygems-%{version}

