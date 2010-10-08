#
# spec file for package asciidoc (Version 8.4.5)
#
# Copyright (c) 2009 SUSE LINUX Products GmbH, Nuernberg, Germany.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

# norootforbuild


Name:           asciidoc
Summary:        Text-Based Document Generation
Version:        8.4.5
Release:        1
License:        GPL v2 or later
Group:          Development/Tools/Doc Generators
Requires:       python >= 2.3
# This is called differently on centos and fedora and rhel
%if 0%{?sles_version} != 0 
Requires:       docbook-xsl-stylesheets
%else
Requires:	docbook-style-xsl
%endif
#Recommends:       dblatex
# a2x needs /usr/bin/xsltproc
Requires:       libxslt
AutoReqProv:    on
Url:            http://www.methods.co.nz/asciidoc/
Source0:        %{name}-%{version}.tar.bz2
Patch0:         asciidoc-vim-fix.diff
Patch1:         asciidoc-8.2.6-no-safe-check.diff
Patch3:         asciidoc-ignore-deprecation.diff
Patch4:         a2x-missing-package-msg.diff
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
BuildArch:      noarch

%description
AsciiDoc is a text document format for writing short documents,
articles, books, and UNIX man pages. AsciiDoc files can be translated
to HTML and DocBook markups using the asciidoc command.



Authors:
--------
    Stuart Rackham <srackham@methods.co.nz>

%package examples
Summary:        Examples and Documents for asciidoc
Group:          Development/Tools/Doc Generators
License:        GPL v2 or later

%description examples
This package contains examples and documetns of asciidoc.



Authors:
--------
    Stuart Rackham <srackham@methods.co.nz>

%prep
%setup -q
%patch0 -p1
%patch1 -p1
%patch3 -p1
%patch4

%build

%install
mkdir -p $RPM_BUILD_ROOT/etc/asciidoc/filters
mkdir -p $RPM_BUILD_ROOT%{_datadir}/asciidoc
mkdir -p $RPM_BUILD_ROOT%{_mandir}/man1
install -m 0644 *.conf $RPM_BUILD_ROOT/etc/asciidoc
install -m 0644 filters/*/*.conf $RPM_BUILD_ROOT/etc/asciidoc/filters/
install -m 0755 filters/*/*.py $RPM_BUILD_ROOT/etc/asciidoc/filters/
install -m 0755 -D asciidoc.py $RPM_BUILD_ROOT%{_bindir}/asciidoc
install -m 0755 -D a2x $RPM_BUILD_ROOT%{_bindir}/a2x
install -m 0644 doc/*.1  $RPM_BUILD_ROOT%{_mandir}/man1/
for i in images stylesheets javascripts docbook-xsl dblatex; do
  cp -av $i $RPM_BUILD_ROOT%{_datadir}/asciidoc/
  ln -s ../../%{_datadir}/asciidoc/$i $RPM_BUILD_ROOT/etc/asciidoc
done
# install vim files
mkdir -p $RPM_BUILD_ROOT%{_datadir}/vim/site/{syntax,ftdetect}
install -m 0644 vim/syntax/* $RPM_BUILD_ROOT%{_datadir}/vim/site/syntax
install -m 0644 vim/ftdetect/* $RPM_BUILD_ROOT%{_datadir}/vim/site/ftdetect

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%config /etc/asciidoc
%{_datadir}/asciidoc
%{_bindir}/*
%{_datadir}/vim
%doc %{_mandir}/man1/*
%doc README BUGS CHANGELOG COPYRIGHT

%files examples
%defattr(-,root,root)
%doc doc examples

%changelog
