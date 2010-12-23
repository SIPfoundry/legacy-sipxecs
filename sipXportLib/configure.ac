AC_PREREQ(2.57)
AC_INIT(sipXportLib,
  m4_esyscmd([config/git-version-gen .tarball-version | sed 's/\(.*\)\.\([0-9]\+\)-\([0-9a-z\-]\+\)/\1/g']),
  sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_AUTOMAKE_VERSION([1.6])
AC_CONFIG_SRCDIR([src/os/OsConfigDb.cpp])
AM_CONFIG_HEADER([src/config.h])
AC_PROG_LIBTOOL
AC_LTDL_SHLIBEXT
AC_PATH_PROG([BASH], [bash])
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC
CHECK_XARGS_REPLACE
CHECK_MSG_NOSIGNAL
CHECK_LIBRT
AC_SEARCH_LIBS([dlopen], [dl])
CHECK_CPPUNIT
CHECK_PCRE
CHECK_SSL
SFAC_STRICT_COMPILE_NO_WARNINGS_ALLOWED
SFAC_INIT_FLAGS
SFAC_FEATURE_SIP_TLS
AC_MSG_CHECKING(for system type for process iterator ($host))
case $host in
    *-*-freebsd*)
    PROCESS_ITERATOR=src/os/freebsd/OsProcessIteratorFreeBSD.cpp
    LIBS="$(LIBS) -lkvm"
    ;;
    *-*-hpux*)
    PROCESS_ITERATOR=src/os/hpux/OsProcessIteratorHpux.cpp
    ;;
    *-*-linux*)
    PROCESS_ITERATOR=src/os/linux/OsProcessIteratorLinux.cpp
    ;;
esac
AC_MSG_RESULT($PROCESS_ITERATOR)
AC_CONFIG_LINKS(src/os/OsProcessIterator.cpp:$PROCESS_ITERATOR)
AC_CHECK_FUNCS(setenv)
AC_CONFIG_FILES([
  Makefile
  include/Makefile
  bin/Makefile
  src/Makefile
  src/test/Makefile
  src/test/sipxunit/Makefile
  src/test/testplugin/Makefile
  syslogviewer/Makefile
])

AC_OUTPUT
