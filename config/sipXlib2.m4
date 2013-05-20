# Initial Version Copyright (C) 2011 eZuce, Inc., All Rights Reserved.
# Licensed to the User under the LGPL license.
# 
# common directories and variables used in sipxecs
#
AC_PREFIX_DEFAULT([/usr/local/sipx])

# This is a "common fix" to a "known issue" with using ${prefix} variable
test "x$prefix" = xNONE && prefix=$ac_default_prefix
test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'

AC_SUBST(SIPX_INCDIR, [${includedir}])
AC_SUBST(SIPX_LIBDIR, [${libdir}])
AC_SUBST(SIPX_LIBEXECDIR, [${libexecdir}/sipXecs])
AC_SUBST(SIPX_BINDIR,  [${bindir}])
AC_SUBST(SIPX_SNAPSHOTBINDIR,  [${bindir}/snapshot])
AC_SUBST(SIPX_BACKUPBINDIR,  [${bindir}/backup])
AC_SUBST(SIPX_CONFDIR, [${sysconfdir}/sipxpbx])
AC_SUBST(SIPX_DATADIR, [${datadir}/sipxecs])
AC_SUBST(SIPX_CFINPUTS, [${datadir}/sipxecs/cfinputs])
AC_SUBST(SIPX_CFMODULES, [${datadir}/sipxecs/cfmodules])
AC_SUBST(SIPX_CFDATA, [${localstatedir}/sipxdata/cfdata])
AC_SUBST(SIPX_INTEROP, [${datadir}/sipxecs/interop])
AC_SUBST(SIPX_DOCDIR,  [${datadir}/doc/sipxecs])
AC_SUBST(SIPX_JAVADIR, [${datadir}/java/sipXecs])
AC_SUBST(SIPX_VARDIR,  [${localstatedir}/sipxdata])
AC_SUBST(SIPX_IVRDATADIR,  [${localstatedir}/sipxdata/mediaserver/data])
AC_SUBST(SIPX_TMPDIR,  [${localstatedir}/sipxdata/tmp])
AC_SUBST(SIPX_DBDIR,   [${localstatedir}/sipxdata/sipdb])
AC_SUBST(SIPX_LOGDIR,  [${localstatedir}/log/sipxpbx])
AC_SUBST(SIPX_RUNDIR,  [${localstatedir}/run/sipxpbx])
AC_SUBST(SIPX_VARLIB,  [${localstatedir}/lib/sipxpbx])
AC_SUBST(SIPX_SERVICEDIR, [${sysconfdir}/init.d])
# sipx RPMs should be hardcoded to use sipxchange user for their sipx user, not the buildbot user
AC_SUBST(SIPX_RPM_CONFIGURE_OPTIONS,  [SIPXPBXUSER=sipx])

# Get the user to run sipX under.
AC_ARG_VAR(SIPXPBXUSER, [The sipX service daemon user name, default is ${USER}])
test -n "$SIPXPBXUSER" || SIPXPBXUSER=$USER

# Get the group to run sipX under.
AC_ARG_VAR(SIPXPBXGROUP, [The sipX service daemon group name, default is value of SIPXPBXUSER])
test -n "$SIPXPBXGROUP" || SIPXPBXGROUP=$SIPXPBXUSER

AC_ARG_VAR(PACKAGE_REVISION, [Package revision number, default is pulled from git])
test -n "$PACKAGE_REVISION" || PACKAGE_REVISION=`cd ${srcdir} && ./config/revision-gen ${PACKAGE_VERSION}`
AC_DEFINE_UNQUOTED([PACKAGE_REVISION], "${PACKAGE_REVISION}", [Revion number including git SHA])

# automake eats straight "if.." in makefiles as autoconf conditions. this avoids that
# http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=34051
AC_SUBST(IF, [if])
AC_SUBST(IFDEF, [ifdef])
AC_SUBST(IFNDEF, [ifndef])
AC_SUBST(ENDIF, [endif])
