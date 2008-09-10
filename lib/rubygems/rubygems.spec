#
# spec file for package rubygems (Version 0.8.11-2)
#

# norootforbuild
# neededforbuild  ruby ruby-devel

Name:           rubygems
Version:        0.8.11
Release:        2
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
License:        Other uncritical OpenSource License
Group:          Development/Languages/Ruby
Provides:       rubygems_with_buildroot_patch
#
URL:            http://rubyforge.org/projects/rubygems/
Summary:        The Ruby standard for publishing and managing third party libraries.
BuildArch: 	noarch
Obsoletes:	ruby-gems ruby-gems-devel ruby-gems-debuginfo


%description
RubyGems is the Ruby standard for publishing and managing third party
libraries.

%install
[ "$RPM_BUILD_ROOT" = "/" ] && printf "Error: RPM_BUILD_ROOT is bogus\n" && exit 99
rm -rf "$RPM_BUILD_ROOT" 
cd /var/tmp/ruby-gems_raw && find . -print0 | cpio -pvud0 "$RPM_BUILD_ROOT"
set -xv
if test "%{_arch}" = "x86_64" ; then
    mv "$RPM_BUILD_ROOT"/usr/lib "$RPM_BUILD_ROOT"%{_libdir}
fi

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && [ -d "$RPM_BUILD_ROOT" ] && rm -rf $RPM_BUILD_ROOT;

%files
%defattr(-,root,root)
%{_bindir}/*
%{_libdir}/*
%{_datadir}/*
