AC_CHECK_FILE(/bin/rpm, 
[
  RPMBUILD_TOPDIR="\$(shell rpm --eval '%{_topdir}')"
  AC_SUBST(RPMBUILD_TOPDIR)

  TARGET_ARCH=`rpm --eval '%{_target_cpu}'`
  AC_SUBST(TARGET_ARCH)

  RPM_DIST=`rpm --eval '%{?dist}'`
  AC_SUBST(RPM_DIST)

  dnl NOTE LIMITATION: doesn't account for distros besides centos, redhat and suse or redhat 6
  DISTRO=`rpm --eval '%{?fedora:fedora}%{!?fedora:%{?suse_version:suse}%{!?suse_version:centos}}'`
  AC_SUBST(DISTRO)

  dnl NOTE LIMITATION: doesn't account for distros besides centos, redhat and suse or redhat 6
  DISTRO_VER=`rpm --eval '%{?fedora:%fedora}%{!?fedora:%{?suse_version:%suse_version}%{!?suse_version:5}}'`
  AC_SUBST(DISTRO_VER)
])

AC_ARG_ENABLE(rpm, [--enable-rpm Using mock package to build rpms],
[
  AC_ARG_VAR(DOWNLOAD_LIB_CACHE, [When to cache source files that are downloaded, default ~/libsrc])
  test -n "${DOWNLOAD_LIB_CACHE}" || DOWNLOAD_LIB_CACHE=~/libsrc

  AC_ARG_VAR(DOWNLOAD_LIB_URL, [When to cache source files that are downloaded, default download.sipfoundry.org])
  test -n "${DOWNLOAD_LIB_URL}" || DOWNLOAD_LIB_URL=http://download.sipfoundry.org/pub/sipXecs/libs

  DEFAULT_MOCK_TARGET_PLATFORM="${DISTRO}-${DISTRO_VER}-${TARGET_ARCH}"
  AC_ARG_VAR(MOCK_TARGET_PLATFORM, [Config template in /etc/mock to build for. default ${DEFAULT_MOCK_TARGET_PLATFORM}])
  test -n "${MOCK_TARGET_PLATFORM}" || MOCK_TARGET_PLATFORM="${DEFAULT_MOCK_TARGET_PLATFORM}"

  AC_CONFIG_FILES([mak/10-rpm.mk])
])
