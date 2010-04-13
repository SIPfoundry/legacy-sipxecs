#
# spec file for package rubygems (Version 1.2.0-1)
#

%define gem_dir %(ruby -rrbconfig -e 'puts File::join(Config::CONFIG["libdir"],"/ruby/gems")')
%define rb_ver %(ruby -rrbconfig -e 'puts Config::CONFIG["ruby_version"]')
%define gem_home %{gem_dir}/%{rb_ver}
%define ruby_sitelib %(ruby -rrbconfig -e 'puts Config::CONFIG["sitelibdir"]')

Name:           rubygems
Version:        1.2.0
Release:        2
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
License:        Ruby or GPL+
Group:          Development/Languages/Ruby
BuildRequires:  ruby ruby-devel ruby-rdoc
Requires:       ruby(abi) = 1.8 ruby-rdoc
Provides:       ruby(rubygems) = %{version}
#
URL:            http://rubyforge.org/projects/rubygems/
Summary:        The Ruby standard for publishing and managing third party libraries.
BuildArch: 	noarch
Source:         rubygems-%{version}.tgz
Obsoletes:	ruby-gems ruby-gems-devel ruby-gems-debuginfo


%description
RubyGems is the Ruby standard for publishing and managing third party libraries.

%prep
%setup -q

# Some of the library files start with #! which rpmlint doesn't like
# and doesn't make much sense
for f in `find lib -name \*.rb` ; do
  head -1 $f | grep -q '^#!/usr/bin/env ruby' && sed -i -e '1d' $f
done

%build
# Nothing

%install
rm -rf $RPM_BUILD_ROOT
GEM_HOME=$RPM_BUILD_ROOT%{gem_home} ruby setup.rb --destdir=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root, -)
%doc README TODO ChangeLog
%doc GPL.txt LICENSE.txt
%{_bindir}/gem

%{gem_dir}

%{ruby_sitelib}/ubygems.rb
%{ruby_sitelib}/rubygems.rb
%{ruby_sitelib}/rubygems
%{ruby_sitelib}/rbconfig

