AC_ARG_ENABLE(rpm, [--enable-rpm Using mock package to build rpms],
[
  RPMBUILD_TOPDIR="\$(shell rpm --eval '%{_topdir}')"
  AC_SUBST(RPMBUILD_TOPDIR)
  RPM_TARGET_ARCH="\$(shell rpm --eval '%{_target_cpu}')"
  AC_SUBST(RPM_TARGET_ARCH)

  RPM_DIST="\$(shell rpm --eval '%{dist}')"
  AC_SUBST(RPM_DIST)

  AC_ARG_VAR(DOWNLOAD_LIB_CACHE, [When to cache source files that are downloaded, default ~/libsrc])
  test -n "${DOWNLOAD_LIB_CACHE}" || DOWNLOAD_LIB_CACHE=~/libsrc

  AC_ARG_VAR(DOWNLOAD_LIB_URL, [When to cache source files that are downloaded, default download.sipfoundry.org])
  test -n "${DOWNLOAD_LIB_URL}" || DOWNLOAD_LIB_URL=http://download.sipfoundry.org/pub/sipXecs/libs

  AC_CONFIG_FILES([
     mak/10-rpm.mk])
])
