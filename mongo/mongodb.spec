# Make sure initddir is defined on el5 and possibly other distros
%{!?_initddir: %define _initddir %{_initrddir}}

%global         daemon mongod
Name:           mongodb
Version:        1.6.4
Release:        3%{?dist}
Summary:        High-performance, schema-free document-oriented database
Group:          Applications/Databases
License:        AGPLv3 and zlib and ASL 2.0
# util/md5 is under the zlib license
# manpages and bson are under ASL 2.0
# everything else is AGPLv3
URL:            http://www.mongodb.org
Obsoletes:	mongo
Obsoletes:	mongo-debuginfo

Source0:        http://fastdl.mongodb.org/src/%{name}-src-r%{version}.tar.gz
Source1:        %{name}.init
Source2:        %{name}.logrotate
Source3:        %{name}.conf
Patch0:         %{name}-cppflags.patch
Patch1:         %{name}-client-ldflags.patch
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  gcc-c++
BuildRequires:  python-devel
BuildRequires:  scons
BuildRequires:  boost-devel
BuildRequires:  pcre-devel
BuildRequires:  js-devel
BuildRequires:  readline-devel
BuildRequires:  ncurses-devel
BuildRequires:  libpcap-devel

Requires(post): chkconfig
Requires(preun): chkconfig

Requires(pre):  shadow-utils

# This is for /sbin/service
Requires(postun): initscripts

# Mongodb must run on a little-endian CPU (see bug #630898)
ExcludeArch:    ppc ppc64 %{sparc} s390 s390x

%description
Mongo (from "humongous") is a high-performance, open source, schema-free
document-oriented database. MongoDB is written in C++ and offers the following
features:
    * Collection oriented storage: easy storage of object/JSON-style data
    * Dynamic queries
    * Full index support, including on inner objects and embedded arrays
    * Query profiling
    * Replication and fail-over support
    * Efficient storage of binary data including large objects (e.g. photos
    and videos)
    * Auto-sharding for cloud-level scalability (currently in early alpha)
    * Commercial Support Available

A key goal of MongoDB is to bridge the gap between key/value stores (which are
fast and highly scalable) and traditional RDBMS systems (which are deep in
functionality).

%package devel
Summary:        MongoDB header files
Group:          Development/Libraries
Requires:       %{name} = %{version}-%{release}
Provides:       %{name}-static = %{version}-%{release}
Obsoletes:	mongo-devel

%description devel
This package provides the header files and C++ driver for MongoDB. MongoDB is
a high-performance, open source, schema-free document-oriented database.

%package server
Summary:        MongoDB server, sharding server and support scripts
Group:          Applications/Databases
Requires:       %{name} = %{version}-%{release}
Obsoletes:	mongo-server

%description server
This package provides the mongo server software, mongo sharding server
software, default configuration files, and init scripts.


%prep
%setup -q -n mongodb-src-r%{version}

# allow cppflags overriding
%patch0 -p1 -b .cppflags
%if %{_vendor} == redhat && 0%{?fedora} <= 6
%patch1 -p0 -b .ldflags
%endif

# spurious permissions
chmod -x README
chmod -x db/repl/rs_exception.h
chmod -x db/resource.h

# wrong end-of-file encoding
sed -i 's/\r//' db/repl/rs_exception.h
sed -i 's/\r//' db/resource.h
sed -i 's/\r//' README

%build
scons %{?_smp_mflags} --cppflags="%{optflags} -fno-strict-aliasing -fPIC" --sharedclient --extralib=ncurses .


%install
rm -rf %{buildroot}
scons install . --cppflags="%{optflags} -fno-strict-aliasing -fPIC" --sharedclient --prefix=%{buildroot}%{_prefix} --nostrip --full
rm -f %{buildroot}%{_libdir}/libmongoclient.a

mkdir -p %{buildroot}%{_sharedstatedir}/%{name}

mkdir -p %{buildroot}%{_localstatedir}/log/%{name}
install -p -D -m 755 %{SOURCE1} %{buildroot}%{_initddir}/%{daemon}
install -p -D -m 644 %{SOURCE2} %{buildroot}%{_sysconfdir}/logrotate.d/%{name}
install -p -D -m 644 %{SOURCE3} %{buildroot}%{_sysconfdir}/mongodb.conf

mkdir -p %{buildroot}%{_mandir}/man1
cp -p debian/*.1 %{buildroot}%{_mandir}/man1/

mkdir -p %{buildroot}%{_localstatedir}/run/%{name}
mkdir -p %{buildroot}%{_localstatedir}/lib/%{name}

%clean
rm -rf %{buildroot}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%pre server
getent group %{name} >/dev/null || groupadd -r %{name}
getent passwd %{name} >/dev/null || \
useradd -r -g %{name} -d %{_sharedstatedir}/%{name} -s /sbin/nologin \
-c "MongoDB Database Server" %{name}
exit 0

%post server
/sbin/chkconfig --add %{daemon}


%preun server
if [ $1 = 0 ] ; then
    /sbin/service  stop >/dev/null 2>&1
    /sbin/chkconfig --del %{daemon}
fi


%postun server
if [ "$1" -ge "1" ] ; then
    /sbin/service %{daemon} condrestart >/dev/null 2>&1 || :
fi


%files
%defattr(-,root,root,-)
%doc README GNU-AGPL-3.0.txt APACHE-2.0.txt
%{_bindir}/mongo
%{_bindir}/mongodump
%{_bindir}/mongoexport
%{_bindir}/mongofiles
%{_bindir}/mongoimport
%{_bindir}/mongorestore
%{_bindir}/mongostat
%{_bindir}/mongosniff
%{_bindir}/bsondump
%{_libdir}/libmongoclient.so

%{_mandir}/man1/mongo.1*
%{_mandir}/man1/mongod.1*
%{_mandir}/man1/mongodump.1*
%{_mandir}/man1/mongoexport.1*
%{_mandir}/man1/mongofiles.1*
%{_mandir}/man1/mongoimport.1*
%{_mandir}/man1/mongosniff.1*
%{_mandir}/man1/mongostat.1*
%{_mandir}/man1/mongorestore.1*

%files server
%defattr(-,root,root,-)
%{_initddir}/%{daemon}
%{_bindir}/mongod
%{_bindir}/mongos
%{_mandir}/man1/mongod.1*
%{_mandir}/man1/mongos.1*
%dir %attr(0755, %{name}, root) %{_sharedstatedir}/%{name}
%dir %attr(0755, %{name}, root) %{_localstatedir}/log/%{name}
%dir %attr(0755, %{name}, root) %{_localstatedir}/run/%{name}
%dir %attr(0755, %{name}, root) %{_localstatedir}/lib/%{name}
%config(noreplace) %{_sysconfdir}/logrotate.d/%{name}
%config(noreplace) %{_sysconfdir}/mongodb.conf

%files devel
%defattr(-,root,root,-)
%{_includedir}/mongo

%changelog
* Mon Dec 06 2010 Nathaniel McCallum <nathaniel@natemccallum.com> - 1.6.4-3
- Add post/postun ldconfig... oops!

* Mon Dec 06 2010 Nathaniel McCallum <nathaniel@natemccallum.com> - 1.6.4-2
- Enable --sharedclient option, remove static lib

* Sat Dec 04 2010 Nathaniel McCallum <nathaniel@natemccallum.com> - 1.6.4-1
- New upstream release

* Fri Oct 08 2010 Nathaniel McCallum <nathaniel@natemccallum.com> - 1.6.3-4
- Put -fPIC onto both the build and install scons calls

* Fri Oct 08 2010 Nathaniel McCallum <nathaniel@natemccallum.com> - 1.6.3-3
- Define _initddir when it doesn't exist for el5 and others

* Fri Oct 08 2010 Nathaniel McCallum <nathaniel@natemccallum.com> - 1.6.3-2
- Added -fPIC build option which was dropped by accident

* Thu Oct  7 2010 Ionuț C. Arțăriși <mapleoin@fedoraproject.org> - 1.6.3-1
- removed js Requires
- new upstream release
- added more excludearches: sparc s390, s390x and bugzilla pointer

* Tue Sep  7 2010 Ionuț C. Arțăriși <mapleoin@fedoraproject.org> - 1.6.2-2
- added ExcludeArch for ppc

* Fri Sep  3 2010 Ionuț C. Arțăriși <mapleoin@fedoraproject.org> - 1.6.2-1
- new upstream release 1.6.2
- send mongod the USR1 signal when doing logrotate
- use config options when starting the daemon from the initfile
- removed dbpath patch: rely on config
- added pid directory to config file and created the dir in the spec
- made the init script use options from the config file
- changed logpath in mongodb.conf

* Wed Sep  1 2010 Ionuț C. Arțăriși <mapleoin@fedoraproject.org> - 1.6.1-1
- new upstream release 1.6.1
- patched SConstruct to allow setting cppflags
- stopped using sed and chmod macros

* Fri Aug  6 2010 Ionuț C. Arțăriși <mapleoin@fedoraproject.org> - 1.6.0-1
- new upstream release: 1.6.0
- added -server package
- added new license file to %%docs
- fix spurious permissions and EOF encodings on some files

* Tue Jun 15 2010 Ionuț C. Arțăriși <mapleoin@fedoraproject.org> - 1.4.3-2
- added explicit js requirement
- changed some names

* Wed May 26 2010 Ionuț C. Arțăriși <mapleoin@fedoraproject.org> - 1.4.3-1
- updated to 1.4.3
- added zlib license for util/md5
- deleted upstream deb/rpm recipes
- made scons not strip binaries
- made naming more consistent in logfile, lockfiles, init scripts etc.
- included manpages and added corresponding license
- added mongodb.conf to sources

* Fri Oct  2 2009 Ionuț Arțăriși <mapleoin@fedoraproject.org> - 1.0.0-3
- fixed libpath issue for 64bit systems

* Thu Oct  1 2009 Ionuț Arțăriși <mapleoin@fedoraproject.org> - 1.0.0-2
- added virtual -static package

* Mon Aug 31 2009 Ionuț Arțăriși <mapleoin@fedoraproject.org> - 1.0.0-1
- Initial release.
