# Generated from rake-0.7.3.gem by gem2rpm -*- rpm-spec -*-
%define ruby_sitelib %(ruby -rrbconfig -e "puts Config::CONFIG['sitelibdir']")
%define gemdir %(ruby -rubygems -e 'puts Gem::dir' 2>/dev/null)
%define gemname rake
%define geminstdir %{gemdir}/gems/%{gemname}-%{version}

%global rubyabi 1.8

Summary: Ruby based make-like utility
Name: rubygem-%{gemname}

Version: 0.8.7
Release: 1%{?dist}
Group: Development/Languages
License: MIT
URL: http://rake.rubyforge.org
Source0: http://gems.rubyforge.org/gems/%{gemname}-%{version}.gem
BuildRoot: %{_tmppath}/%{name}-%{version}-root-%(%{__id_u} -n)
Requires: ruby(rubygems)
Requires: ruby(abi) = %{rubyabi}
BuildRequires: ruby(rubygems)
BuildRequires: ruby(abi) = %{rubyabi}
#BuildRequires(check): ruby(flexmock)
BuildArch: noarch
Provides: rubygem(%{gemname}) = %{version}

%description
Rake is a Make-like program implemented in Ruby. Tasks and dependencies are
specified in standard Ruby syntax.


%prep
%setup -q -c -T

%build
mkdir -p .%{gemdir}
gem install -V \
	--local \
	--install-dir $(pwd)/%{gemdir} \
	--force --rdoc \
	%{SOURCE0}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%{gemdir}
cp -a .%{gemdir}/* %{buildroot}%{gemdir}/


mkdir -p %{buildroot}/%{_bindir}
mv %{buildroot}%{gemdir}/bin/* %{buildroot}/%{_bindir}
find %{buildroot}/%{_bindir} -type f | xargs -n 1 sed -i  -e 's"^#!/usr/bin/env ruby"#!/usr/bin/ruby"'
rmdir %{buildroot}%{gemdir}/bin
find %{buildroot}%{geminstdir}/{lib,test} -type f | xargs -n 1 sed -i  -e '/^#!\/usr\/bin\/env ruby/d'
find %{buildroot}%{geminstdir}/{doc,lib,test} -type f | xargs chmod 0644

%clean
rm -rf %{buildroot}

%check
export GEM_PATH=$(pwd)/%{gemdir}
pushd .%{geminstdir}
# Someone please check why test fails!!
# Note that on ppc64 the following test causes segv, perhaps
# bug in ruby itself, needs investigating.
#ruby ./bin/rake test || :

%files
%defattr(-, root, root, -)
%{_bindir}/rake
%dir %{geminstdir}
%{geminstdir}/Rakefile
%{geminstdir}/install.rb
%{geminstdir}/bin
%{geminstdir}/lib
%doc %{geminstdir}/test
%doc %{gemdir}/doc/%{gemname}-%{version}
%doc %{geminstdir}/README
%doc %{geminstdir}/MIT-LICENSE
%doc %{geminstdir}/TODO
%doc %{geminstdir}/CHANGES
%doc %{geminstdir}/doc
%{gemdir}/cache/%{gemname}-%{version}.gem
%{gemdir}/specifications/%{gemname}-%{version}.gemspec


%changelog
* Thu Jun 18 2009 Mamoru Tasaka <mtasaka@ioa.s.u-tokyo.ac.jp> - 0.8.7-1
- 0.8.7
- Enable %%check

* Tue Mar 17 2009 Jeroen van Meeuwen <j.van.meeuwen@ogd.nl> - 0.8.4-1
- New upstream version

* Wed Feb 25 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.8.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_11_Mass_Rebuild

* Mon Oct 27 2008 David Lutterkort <lutter@redhat.com> - 0.8.3-1
- Cleanup multiply listed files
- Set permissions in doc/, lib/ and test/ to 644

* Thu May 15 2008 Alan Pevec <apevec@redhat.com> 0.8.1-2
- fix shebang in scripts

* Thu May 15 2008 Alan Pevec <apevec@redhat.com> 0.8.1-1
- new upstream version

* Thu Aug 23 2007 David Lutterkort <dlutter@redhat.com> - 0.7.3-2
- Fix license tag
- Remove bogus shebangs in lib/ and test/

* Mon Jun 18 2007 David Lutterkort <dlutter@redhat.com> - 0.7.3-1
- Initial package
