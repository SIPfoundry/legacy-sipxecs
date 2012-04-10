Summary: A systems administration tool for networks
Name: cfengine
Version: 3.3.0
Release: 1%{?dist}
License: GPLv3
Group: Applications/System
Source0: %{name}-%{version}.tar.gz
Source1: cf-execd
Source2: cf-serverd
Source3: cf-monitord
URL: http://www.cfengine.org/
BuildRequires: db4-devel,openssl-devel,bison,flex,m4,libacl-devel
BuildRequires: libselinux-devel,tetex-dvips,texinfo-tex,pcre-devel
BuildRequires: tokyocabinet-devel
Requires(post): /sbin/chkconfig, /sbin/install-info
Requires(preun): /sbin/chkconfig, /sbin/install-info, /sbin/service
Requires(postun): /sbin/service
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

%description
Cfengine, or the configuration engine is an agent/software robot and a
very high level language for building expert systems to administrate
and configure large computer networks. Cfengine uses the idea of
classes and a primitive form of intelligence to define and automate
the configuration and maintenance of system state, for small to huge
configurations. Cfengine is designed to be a part of a computer immune
system.

%package doc
Summary: Documentation for cfengine
Group: Documentation
Requires: %{name} = %{version}-%{release}
#BuildArch: noarch

%description doc
This package contains the documentation for cfengine.


%prep
%setup -q


%build
%configure BERKELEY_DB_LIB=-ldb \
    --enable-selinux \
    --enable-fhs
make


%install
rm -rf $RPM_BUILD_ROOT

mkdir -p $RPM_BUILD_ROOT%{_sbindir}
mkdir -p $RPM_BUILD_ROOT%{_datadir}/%{name}
make DESTDIR=$RPM_BUILD_ROOT install

# It's ugly, but thats the way Mark wants to have it. :(
# If we don't create this link, cfexecd will not be able to start
# (hardcoded) /var/sbin/cf-agent in scheduled intervals. Other option 
# would be to patch cfengine to use %{_sbindir}/cf-agent
# but upstream won't support this
mkdir -p $RPM_BUILD_ROOT/%{_var}/%{name}/bin
ln -sf %{_sbindir}/cf-agent $RPM_BUILD_ROOT/%{_var}/%{name}/bin/
ln -sf %{_sbindir}/cf-promises $RPM_BUILD_ROOT/%{_var}/%{name}/bin/

# init scripts
mkdir -p $RPM_BUILD_ROOT%{_initrddir}
for i in %{SOURCE1} %{SOURCE2} %{SOURCE3}
do
	install -p -m 0755 $i $RPM_BUILD_ROOT%{_initrddir}/
done

rm -f $RPM_BUILD_ROOT%{_infodir}/dir

# All this stuff is pushed into doc/contrib directories
rm -rf $RPM_BUILD_ROOT%{_datadir}/%{name} 
rm -f $RPM_BUILD_ROOT%{_sbindir}/cfdoc


%post
# cfagent won't run nicely, unless your host has keys.
if [ ! -d /mnt/sysimage -a ! -f %{_var}/%{name}/ppkeys/localhost.priv ]; then
	%{_sbindir}/cf-key >/dev/null || :
fi
/sbin/install-info --info-dir=%{_infodir} %{_infodir}/cfengine*.info* 2> /dev/null || :
# add init files to chkconfig
if [ "$1" = "1" ]; then
	/sbin/chkconfig --add cf-monitord
	/sbin/chkconfig --add cf-execd
	/sbin/chkconfig --add cf-serverd
fi


%preun
if [ "$1" = "0" ]; then
    /sbin/install-info --delete --info-dir=%{_infodir} %{_infodir}/cfengine*.info* 2> /dev/null || :
    /sbin/service cf-monitord stop >/dev/null 2>&1 || :
    /sbin/service cf-execd stop >/dev/null 2>&1 || :
    /sbin/service cf-serverd stop >/dev/null 2>&1 || :
	/sbin/chkconfig --del cf-monitord
	/sbin/chkconfig --del cf-execd
	/sbin/chkconfig --del cf-serverd
fi

%postun
if [ $1 -ge 1 ]; then
    /sbin/service cf-monitord condrestart >/dev/null 2>&1 || :
    /sbin/service cf-execd condrestart >/dev/null 2>&1 || :
    /sbin/service cf-serverd condrestart >/dev/null 2>&1 || :
fi


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc AUTHORS ChangeLog README
%{_sbindir}/*
/usr/libexec/cfengine/libpromises*
%{_mandir}/man8/*
%{_initrddir}/cf-monitord
%{_initrddir}/cf-execd
%{_initrddir}/cf-serverd
%{_var}/%{name}

%files doc
%defattr(-,root,root,-)
%doc %{_datadir}/doc/cfengine/README
%doc %{_datadir}/doc/cfengine/ChangeLog
%doc %{_datadir}/doc/cfengine/example_config/*.cf
%doc %{_datadir}/doc/cfengine/examples/*.cf
%doc %{_datadir}/doc/cfengine-%{version}/AUTHORS
%doc %{_datadir}/doc/cfengine-%{version}/ChangeLog
%doc %{_datadir}/doc/cfengine-%{version}/README

%changelog
* Tue Apr 10 2012 Douglas Hubler <douglas@hubler.us> - 3.3.0-1
- Update to 3.3.0

* Tue Jun 21 2011 Douglas Hubler <douglas@hubler.us> - 3.1.5-2
- Removed --with-doc no longer nec
- Added --target to get to compile.
  See https://bugzilla.redhat.com/show_bug.cgi?id=204177

* Thu Sep 30 2010 Jeff Sheltren <jeff@osuosl.org> - 3.0.5p1-1
- Update for cfengine 3
- Now buildrequires pcre-devel

* Tue Feb 23 2010 Jeff Sheltren <jeff@osuosl.org> - 2.2.10-6
- Rebuild for new db4

* Tue Dec 29 2009 Jeff Sheltren <jeff@osuosl.org> - 2.2.10-5
- Move docs into a -doc subpackage (#523538)

* Sat Dec 12 2009 Jeff Sheltren <jeff@osuosl.org> - 2.2.10-4
- Patch for class definitions using shellcommands (#530458)

* Fri Aug 21 2009 Tomas Mraz <tmraz@redhat.com> - 2.2.10-3
- rebuilt with new openssl

* Fri Jul 24 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.2.10-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Tue May 19 2009 Jeff Sheltren <jeff@osuosl.org> 2.2.10-1
- Update to upstream 2.2.10
- Remove db-4.7 patch as it is now included upstream

* Mon Feb 23 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.2.8-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_11_Mass_Rebuild

* Thu Jan 15 2009 Tomas Mraz <tmraz@redhat.com> 2.2.8-3
- rebuild with new openssl

* Sat Sep 27 2008 Jeff Sheltren <jeff@osuosl.org> 2.2.8-2
- Patch configure to detect db-4.7 (#461942)

* Fri Aug  8 2008 Jeff Sheltren <jeff@osuosl.org> 2.2.8-1
- Update to upstream 2.2.8
- Release now includes full documentation again
- Add buildrequires for tetex-dvips and texinfo-tex

* Tue Jun 17 2008 Jeff Sheltren <jeff@osuosl.org> 2.2.7-1
- Update to upstream 2.2.7

* Tue Apr 22 2008 Jeff Sheltren <jeff@osuosl.org> 2.2.6-1
- Update to upstream 2.2.6
- Redirect cfkey output to /dev/null
- Manpages now included (again) in upstream package, remove unneeded patch

* Sat Mar 22 2008 Jeff Sheltren <jeff@osuosl.org> 2.2.5-1
- Update to upstream 2.2.5
- Remove variable expansion patch
- Add patch for manpages which are missing from this release
- Remove documentation files which are no longer included (no more info files)
- Buildreqs for texinfo, tetex, tetex-dvips no longer needed

* Sat Feb 23 2008 Jeff Sheltren <jeff@osuosl.org> 2.2.3-5
- Patch for buffer overflow during variable expantion (SVN r526)

* Tue Feb 19 2008 Fedora Release Engineering <rel-eng@fedoraproject.org> - 2.2.3-4
- Autorebuild for GCC 4.3

* Wed Dec  5 2007 Jeff Sheltren <jeff@osuosl.org> 2.2.3-3
- Rebuild in devel for new openssl

* Sun Dec  2 2007 Jeff Sheltren <jeff@osuosl.org> 2.2.3-2
- fix libdir regex in files section, don't include debug files (#407881)

* Sat Dec  1 2007 Jeff Sheltren <jeff@osuosl.org> 2.2.3-1
- Update to upstream 2.2.3
- Remove unneeded patches (hostrange and glibc open)

* Sat Aug 25 2007 Jeff Sheltren <sheltren@cs.ucsb.edu> 2.2.1-4
- Patch for bug when using open with newer glibc
- Update license tag in spec

* Tue Jun 26 2007 Jeff Sheltren <sheltren@cs.ucsb.edu> 2.2.1-3
- Update hostrange patch

* Mon Jun 25 2007 Jeff Sheltren <sheltren@cs.ucsb.edu> 2.2.1-2
- Fix hostrange bug (patch from SVN r397)

* Wed May 30 2007 Jeff Sheltren <sheltren@cs.ucsb.edu> 2.2.1-1
- Update to upstream 2.2.1
- Remove SELinux patch (included upstream)

* Fri May 11 2007 Jeff Sheltren <sheltren@cs.ucsb.edu> 2.2.0-1
- Update to upstream 2.2.0
- No longer need automake,autoconf for build

* Wed Apr 25 2007 Jeff Sheltren <sheltren@cs.ucsb.edu> 2.1.22-4
- Update SELinux patch

* Fri Apr 13 2007 Jeff Sheltren <sheltren@cs.ucsb.edu> 2.1.22-3
- Patch for OS detection for newer Fedora/RedHat releases (#235922)
- Patch for updated autotools
- Add service condrestart commands to postun
- Add service stop commands to preun

* Sun Feb 25 2007 Jeff Sheltren <sheltren@cs.ucsb.edu> 2.1.22-2
- Patch for selinux support (#187120)
- init scripts no longer marked as config files

* Mon Jan 29 2007 Jeff Sheltren <sheltren@cs.ucsb.edu> 2.1.22-1
- update to upstream 2.2.22

* Fri Nov 10 2006 Jeff Sheltren <sheltren@cs.ucsb.edu> 2.1.21-3
- rebuild for db4 update

* Thu Oct 05 2006 Christian Iseli <Christian.Iseli@licr.org> 2.1.21-2
 - rebuilt for unwind info generation, broken in gcc-4.1.1-21

* Wed Sep 20 2006 Jeff Sheltren <sheltren@cs.ucsb.edu> - 2.1.21-1
- update to upstream 2.1.21
- remove unneeded ipv6 overflow patch (fixed in current version)

* Sat Sep  9 2006 Jeff Sheltren <sheltren@cs.ucsb.edu> - 2.1.20-5
- another build system release bump

* Sat Sep  9 2006 Jeff Sheltren <sheltren@cs.ucsb.edu> - 2.1.20-4
- Bump release for FC6 rebuild

* Mon May  8 2006 Jeff Sheltren <sheltren@cs.ucsb.edu> - 2.1.20-3
- Patch for buffer overflow when using ipv6 addresses (#190822)

* Fri Mar 31 2006 Jeff Sheltren <sheltren@cs.ucsb.edu> - 2.1.20-1
- Update to upstream 2.1.20

* Thu Mar  2 2006 Jeff Sheltren <sheltren@cs.ucsb.edu> - 2.1.19p1-1
- Update to upstream 2.1.19p1

* Fri Feb 17 2006 Jeff Sheltren <sheltren@cs.ucsb.edu> - 2.1.18-2
- Rebuild for Fedora Extras 5

* Fri Dec 30 2005 Jeff Sheltren <sheltren@cs.ucsb.edu> - 2.1.18-1
- Update to upstream 2.1.18

* Mon Oct 17 2005 Jeff Sheltren <sheltren@cs.ucsb.edu> - 2.1.16-2
- Patch insecure temp file, CAN-2005-2960 (#170896)

* Sun Oct  2 2005 Jeff Sheltren <sheltren@cs.ucsb.edu> - 2.1.16-1
- Update to upstream 2.1.16

* Mon Jun 20 2005 Jeff Sheltren <sheltren@cs.ucsb.edu> - 2.1.15-2
- Remove cfdoc from sbin and make contrib/cfdoc non-executable
  in order to get rid of perl dependency
- Add dist tag to release

* Thu Jun 16 2005 Jeff Sheltren <sheltren@cs.ucsb.edu> - 2.1.15-1
- Update to upstream 2.1.15

* Thu Apr 14 2005 Jeff Sheltren <sheltren@cs.ucsb.edu> - 2.1.14-2
- Bump release for FC4/devel package

* Sat Apr  9 2005 Jeff Sheltren <sheltren@cs.ucsb.edu> - 2.1.14-1
- Update to upstream 2.1.14

* Mon Mar 14 2005 Jeff Sheltren <sheltren@cs.ucsb.edu> - 2.1.13-4
- add buildrequires: tetex

* Wed Mar  9 2005 Jeff Sheltren <sheltren@cs.ucsb.edu> - 2.1.13-3
- change _localstatdir macros to _var
- change group to Applications/System
- add buildrequires: libacl-devel
- add requires(post,preun) for chkconfig and install-info

* Wed Mar  9 2005 Jeff Sheltren <sheltren@cs.ucsb.edu> - 2.1.13-2
- Remove unnecessary 'chkconfig <init_script> off' from post section

* Mon Mar  7 2005 Jeff Sheltren <sheltren@cs.ucsb.edu> - 2.1.13-1
- Various spec file changes: change summary, line separators, defattr
- Remove epoch 0

* Mon Feb 27 2005 David Dorgan <davidd at micro-gravity.com> - 0:2.1.13-0
- Updated to version 2.1.13

* Sun Aug 15 2004 Juha Ylitalo <jylitalo@iki.fi> - 0:2.1.9-2
- nowdays we need --with-docs to get man pages included
- texinfo added into buildrequires list
- tetex replaced with tetex-dvips

* Sun Aug 15 2004 Juha Ylitalo <jylitalo@iki.fi> - 0:2.1.9-0.fdr.1
- mainstream version update
- fixes http://www.coresecurity.com/common/showdoc.php?idx=387&idxseccion=10
- fixes #1975 in bugzilla.fedora.us

* Sun Nov 30 2003 Juha Ylitalo <jylitalo@iki.fi> - 0.2.1.0-0.fdr.2.p1
- FC1 requires m4 into BuildRequires.

* Tue Nov 11 2003 Juha Ylitalo <jylitalo@iki.fi> - 0.2.1.0-0.fdr.1.p1
- new upstream version

* Sat Sep 27 2003 Juha Ylitalo <jylitalo@iki.fi> - 0:2.0.8-0.fdr.2.p1
- changed init.d to follow Fedora template
- got rid of duplicate example files
- moved example files into input directory
- fixed permissions on example files (removed execute permissions on them)

* Thu Sep 25 2003 Juha Ylitalo <jylitalo@iki.fi> - 0:2.0.8-0.fdr.1.p1
- new upstream version

* Tue Sep 02 2003 Juha Ylitalo <jylitalo@iki.fi> - 0:2.0.8-0.fdr.1
- new upstream version
- delete NEWS file if its zero bytes long.

* Tue Aug 12 2003 Juha Ylitalo <jylitalo@iki.fi> - 0:2.0.7-0.fdr.4.p3
- chmod 644 for source0
- added missing signature to SRPM that comes out

* Mon Jul 21 2003 Juha Ylitalo <jylitalo@iki.fi> - 0:2.0.7-0.fdr.3.p3
- fixed License from "GNU GPL" to GPL
- init.d scripts changed to be "config(noreplace)"

* Fri Jul 04 2003 Juha Ylitalo <jylitalo@iki.fi> - 0:2.0.7-0.fdr.2.p3
- changes to version and release to apply with Fedora's Naming Guidelines
- dropped gcc from BuildRequires
- "2345" initlevels replaced with "-" in init.d scripts chkconfig part.
- added chkconfig into %post and %pre for all init.d scripts

* Wed Jun 25 2003 Juha Ylitalo <jylitalo@iki.fi> - 0:2.0.7p3-0.fdr.1
- new upstream version.

* Sun May 04 2003 Juha Ylitalo <jylitalo@iki.fi> - 0:2.0.6-0.fdr.4
- added init.d script for cfenvd.
- sanity checks for cfservd and cfexecd init.d script
- added outputs and ppkeys directories into %files list.

* Sun Apr 20 2003 Juha Ylitalo <jylitalo@iki.fi> - 0:2.0.6-0.fdr.3
- set Group to "System Environment/Daemons", which is valid in RPM 4.2
  (old value was System/Utilities)

* Wed Apr 09 2003 Juha Ylitalo <jylitalo@iki.fi> - 0:2.0.6-0.fdr.2
- fixed URL
- removed Requires as redundant
- remove %{_infodir}/dir from package, if it would exist 
  (which is not the case in RH8)

* Wed Apr 09 2003 Juha Ylitalo <jylitalo@iki.fi> - 0:2.0.6-0.fdr.1
- upgrade from 2.0.5 to 2.0.6
- added install-info to %post and %preun
- added Epoch
- changed all paths that are not hard coded into %{_sbindir}, etc.
- changed ./configure to %configure
- changed URL to something you can cut&paste
- simplified %files list and yet added more stuff into %doc part.

* Wed Mar 26 2003 Juha Ylitalo <jylitalo@iki.fi> - 0:2.0.5-0.fdr.1
- upgrade from 2.0.4 to 2.0.5
- fedora related changes to spec file.

* Thu Sep 05 2002 Juha Ylitalo <juha.ylitalo@iki.fi> - 2.0.4-1
- new upstream version.

* Thu Jun 20 2002 Juha Ylitalo <juha.ylitalo@iki.fi> - 2.0.2-4
- added check that if we are doing initial install 
  (meaning that /mnt/sysimage is mounted), there is no point at
  running cfkey until system boots up with correct root directory

* Thu Jun 13 2002 Juha Ylitalo <juha.ylitalo@iki.fi> - 2.0.2-3
- fixed revision numberings to go on in sensible way
- added missing man page
- changed cfexecd and cfservd to start later 
  (assuming your using chkconfig for setting things)
- additional check in cfexecd start() for making sure that local cfkeys
  have been created.

* Wed May 29 2002 Juha Ylitalo <juha.ylitalo@iki.fi> - 2.0.2-1
- version upgrade
- support for clients using dynamic addresses (=DHCP)
- support for ip ranges
- bug fixes

* Thu May  9 2002 Juha Ylitalo <juha.ylitalo@iki.fi> - 2.0.1-2
- fixed first line in init.d scripts
- added chkconfig line into init.d scripts

* Wed May  1 2002 Juha Ylitalo <juha.ylitalo@iki.fi> - 2.0.1-1
- first public RPM...

