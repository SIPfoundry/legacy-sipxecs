# Initial Version Copyright (C) 2010 eZuce, Inc., All Rights Reserved.
# Licensed to the User under the LGPL license.
# 
AC_PREREQ(2.57)

# Run "make sipx.update-version" to update all the other sipx projects version strings
AC_INIT(sipX, 
  m4_esyscmd([config/git-version-gen .tarball-version | sed 's/\(.*\)\.\([0-9]\+\)-\([0-9a-z\-]\+\)/\1/g']),
  sipx-dev@list.sipfoundry.org)

# Pass same ./configure switches all sub-projects. Crude but effective.
AC_SUBST(CONFIGURE_OPTIONS, $@)

m4_include([config/sipXlib2.m4])
m4_include([mak/opensuse-build-service.m4])
m4_include([mak/centos-iso.m4])
m4_include([mak/rpm.m4])
m4_include([mak/continuous-integration.m4])

AC_ARG_ENABLE(sipfoundry-publish, [--enable-sipfoundry-publish Enable publishing of RPMs to SIPfoundry],
[
  AC_CONFIG_FILES([mak/25-sipfoundry-publish.mk])
])

AC_CONFIG_FILES([
	Makefile
	mak/00-modules.mk
	mak/05-build.mk
	mak/20-list-dependencies.mk
])

AC_OUTPUT

