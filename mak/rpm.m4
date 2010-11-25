AC_ARG_ENABLE(rpm, [--enable-rpm Using mock package to build rpms],
[
  RPMBUILD_TOPDIR="\$(shell rpm --eval '%{_topdir}')"
  AC_SUBST(RPMBUILD_TOPDIR)
  RPM_TARGET_ARCH="\$(shell rpm --eval '%{_target_cpu}')"
  AC_SUBST(RPM_TARGET_ARCH)

  AC_CONFIG_FILES([
     mak/10-rpm.mk])
])
