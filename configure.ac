# Initial Version Copyright (C) 2010 eZuce, Inc., All Rights Reserved.
# Licensed to the User under the LGPL license.
#
# 
AC_PREREQ(2.57)

AC_INIT(sipX, 14.10, sipx-dev@list.sipfoundry.org)

# Pass standard and sipxecs common params to ./configure switches on all sub-projects.
# Doesn't have to handle ./configure vars like SIPX_PBXUSER because those
# can be passed handled 
OPTIONS=
for a in $@; do
  case $a in
    --prefix*)OPTIONS+=" $a" ;;
    --disable-dep-check*)OPTIONS+=" $a" ;;
    --enable-debug*)OPTIONS+=" $a" ;;
    PACKAGE_REVISION*)OPTIONS+=" $a" ;;
    DART*)OPTIONS+=" $a" ;;
  esac
done
AC_SUBST(OPTIONS)

m4_include([config/autoconf-2.66.m4])
m4_include([config/sipXlib2.m4])
m4_include([mak/rpm.m4])
m4_include([mak/centos-iso.m4])
m4_sinclude([custom/custom.m4])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([mak/05-build.mk:mak/build.mk.in])
AC_CONFIG_FILES([mak/20-list-dependencies.mk:mak/list-dependencies.mk.in])
AC_CONFIG_FILES([mak/30-setup.mk:mak/setup.mk.in])

AC_OUTPUT

