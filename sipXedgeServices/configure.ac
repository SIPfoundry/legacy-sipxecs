AC_PREREQ(2.57)
AC_INIT(sipXedgeServices, 14.04, https://github.com/SIPFoundry)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR([applications/webrtc])
AC_PATH_PROG([BASH], [bash])
AC_PROG_CXX
AC_PROG_CC

m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

AM_INIT_AUTOMAKE(foreign tar-pax)
AM_CONFIG_HEADER([config.h])
LT_INIT

AM_CONDITIONAL(IS_RPMBUILD, test x$enable_rpmbuild = xyes)
SFAC_INIT_FLAGS
SFAC_LIB_PORT
SFAC_LIB_COMMSERVER

AC_ARG_ENABLE(dep-check,
  AS_HELP_STRING([--disable-dep-check],
  [Do not check for dependencies other then those required for dist target]))

AC_DEFUN([SF_MISSING_DEP],[
  if test "x$enable_dep_check" != "xno"; then
    AC_MSG_ERROR([$1])
  fi
])

AC_CHECK_LIB(boost_thread, main, [BOOST_LIBS="-lboost_date_time -lboost_filesystem -lboost_system -lboost_thread -lboost_program_options -lboost_iostreams -lboost_random -lboost_regex"],
    [AC_CHECK_LIB(boost_thread-mt, main,
    [BOOST_LIBS="-lboost_date_time-mt -lboost_filesystem-mt -lboost_system-mt -lboost_thread-mt -lboost_program_options-mt -lboost_iostreams-mt -lboost_regex-mt"],
    [AC_MSG_ERROR("no boost thread found")])])
AC_SUBST(BOOST_LIBS)


LIBVERSION_CURRENT="1"
LIBVERSION_REVISION="0"
LIBVERSION_AGE="0"
LIBVERSION_FULL="$LIBVERSION_CURRENT.$LIBVERSION_REVISION.$LIBVERSION_AGE"
LIBVERSION_INFO="$LIBVERSION_CURRENT:$LIBVERSION_REVISION:$LIBVERSION_AGE"
AC_SUBST(LIBVERSION_CURRENT)
AC_SUBST(LIBVERSION_REVISION)
AC_SUBST(LIBVERSION_AGE)
AC_SUBST(LIBVERSION_FULL)
AC_SUBST(LIBVERSION_INFO)

AC_CONFIG_FILES([
  applications/Makefile
  Makefile
])

AC_OUTPUT
