AC_PREREQ(2.57)
AC_INIT(sipXtackLib,
  m4_esyscmd([config/git-version-gen .tarball-version | sed 's/\(.*\)\.\([0-9]\+\)-\([0-9a-z\-]\+\)/\1/g']),
  sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_AUTOMAKE_VERSION([1.6])
AC_CONFIG_SRCDIR([src/net/SipMessage.cpp])
AM_CONFIG_HEADER([src/config.h])
AC_PROG_LIBTOOL
AC_PATH_PROG([BASH], [bash])
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC
CHECK_XARGS_REPLACE
AC_MSG_CHECKING(for res_query)
AC_TRY_LINK_FUNC(res_query, [NEED_RESOLV=0; AC_MSG_RESULT(yes)],
        [NEED_RESOLV=1])
if test $NEED_RESOLV = "1"; then
   saved_LIBS="$LIBS"
   LIBS="$LIBS -lresolv"
   AC_MSG_CHECKING(for res_query in -lresolv)
   AC_LINK_IFELSE( [AC_LANG_PROGRAM([ #include <resolv.h>],
                                    [res_query (0, 0, 0, 0, 0);])
                   ],
      [AC_MSG_RESULT(yes)], [LIBS="$saved_LIBS"; AC_MSG_RESULT(no)])
fi

CHECK_CPPUNIT
CHECK_SSL
SFAC_STRICT_COMPILE_NO_WARNINGS_ALLOWED
SFAC_INIT_FLAGS
SFAC_LIB_PORT
SFAC_FEATURE_SIP_TLS
AC_ARG_ENABLE(stack-examples,
  [  --enable-stack-examples       Builds examples directory (disabled by default) ]
  , enable_stack_examples=yes)
AM_CONDITIONAL(EXAMPLES, test x$enable_stack_examples = xyes)

# named is used in some sipXtackLib tests
CHECK_NAMED

AC_CONFIG_FILES([
  Makefile
  include/Makefile
  src/Makefile
  meta/Makefile
  src/test/Makefile
  examples/Makefile
  examples/im/Makefile
  examples/xmlrpc/Makefile
  siptest/Makefile
  siptest/src/Makefile
  siptest/include/Makefile
  siptest/src/siptest/Makefile
  dialogwatch/Makefile
])

AC_OUTPUT
