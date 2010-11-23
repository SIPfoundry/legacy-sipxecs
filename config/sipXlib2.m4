# simplified version of sipXlib.m4 defining common values

AC_ARG_VAR(SIPX_USER, [The service daemon user name, default is the user compiling this package])
test -z "$SIPX_USER" && SIPX_USER="$USER"

AC_SUBST(SIPX_RUN_DIR, [${localstatedir}/run/sipxpbx])
AC_SUBST(SIPX_LOG_DIR, [${localstatedir}/log/sipxpbx])
AC_SUBST(SIPX_CONF_DIR, [${sysconfdir}/sipxpbx])
AC_SUBST(SIPX_BIN_DIR, [${bindir}])
