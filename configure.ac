AC_PREREQ(2.57)

# Run "make sipx.update-version" to update all the other sipx projects version strings
AC_INIT(sipX, 0.4.4, sipx-dev@list.sipfoundry.org)

# Pass same ./configure switches all sub-projects. Crude but effective.
AC_SUBST(CONFIGURE_OPTIONS, $@)

m4_include([mak/opensuse-build-service.m4])
m4_include([mak/centos-iso.m4])

AC_ARG_ENABLE(sipfoundry-publish, [Enable publishing of RPMs to SIPfoundry],
[
  AC_CONFIG_FILES([mak/25-sipfoundry-publish.mk])
])

AC_CONFIG_FILES([
	Makefile
	mak/00-modules.mk
	mak/05-build.mk
])

AC_OUTPUT

