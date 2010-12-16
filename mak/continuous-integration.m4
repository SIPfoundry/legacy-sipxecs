dnl Initial Version Copyright (C) 2010 eZuce, Inc., All Rights Reserved.
dnl Licensed to the User under the LGPL license.
dnl
AC_ARG_ENABLE(continuous-integration, [--enable-continuous-integration build setup for building rpms],
[
  AC_ARG_VAR(MIRROR_DIR, [Where to mirror centos and fedora rpms, default is /var/mirror])
  test -n "${MIRROR_DIR}" || MIRROR_DIR=/var/mirror

  AC_CONFIG_FILES([mak/40-continuous-integration.mk])
])
