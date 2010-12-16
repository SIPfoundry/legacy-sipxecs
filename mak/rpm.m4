dnl Initial Version Copyright (C) 2010 eZuce, Inc., All Rights Reserved.
dnl Licensed to the User under the LGPL license.
dnl

dnl NOTE: To support non-rpm based distos, write equivalent of this that defines DISTRO_* vars
AC_CHECK_FILE(/bin/rpm, 
[
  RPMBUILD_TOPDIR="\$(shell rpm --eval '%{_topdir}')"
  AC_SUBST(RPMBUILD_TOPDIR)

  RPM_DIST=`rpm --eval '%{?dist}'`
  AC_SUBST(RPM_DIST)

  DistroArchDefault=`rpm --eval '%{_target_cpu}'`

  dnl NOTE LIMITATION: default doesn't account for distros besides centos, redhat and suse or redhat 6
  DistroDefault=`rpm --eval '%{?fedora:fedora}%{!?fedora:%{?suse_version:suse}%{!?suse_version:centos}}'`

  dnl NOTE LIMITATION: default doesn't account for distros besides centos, redhat and suse or redhat 6
  DistroVerDefault=`rpm --eval '%{?fedora:%fedora}%{!?fedora:%{?suse_version:%suse_version}%{!?suse_version:5}}'`
])

AC_ARG_VAR(DISTRO_ARCH, [What CPU architecture you are compiling for. Default is ${DistroArchDefault}])
test -n "${DISTRO_ARCH}" || DISTRO_ARCH="${DistroArchDefault:-unknown}"

AC_ARG_VAR(DISTRO, [What operating system you are compiling for. Default is ${DistroDefault}])
test -n "${DISTRO}" || DISTRO="${DistroDefault:-unknown}"

AC_ARG_VAR(DISTRO_VER, [What operating system version you are compiling for. Default is ${DistroVerDefault}])
test -n "${DISTRO_VER}" || DISTRO_VER="${DistroVerDefault:-unknown}"

AC_ARG_ENABLE(rpm, [--enable-rpm Using mock package to build rpms],
[
  AC_ARG_VAR(DOWNLOAD_LIB_CACHE, [When to cache source files that are downloaded, default ~/libsrc])
  test -n "${DOWNLOAD_LIB_CACHE}" || DOWNLOAD_LIB_CACHE=~/libsrc

  AC_ARG_VAR(DOWNLOAD_LIB_URL, [When to cache source files that are downloaded, default download.sipfoundry.org])
  test -n "${DOWNLOAD_LIB_URL}" || DOWNLOAD_LIB_URL=http://download.sipfoundry.org/pub/sipXecs/libs

  AC_ARG_VAR(MOCK_TARGET_PLATFORM, [Config template in /etc/mock to build for. default is 'default'])
  test -n "${MOCK_TARGET_PLATFORM}" || MOCK_TARGET_PLATFORM=default

  AC_ARG_VAR(RPM_DIST_DIR, [Where to assemble final set of RPMs and SRPMs in preparation for publishing to a download server.])
  test -n "${RPM_DIST_DIR}" || RPM_DIST_DIR=repo

  AC_CONFIG_FILES([mak/10-rpm.mk])
])
