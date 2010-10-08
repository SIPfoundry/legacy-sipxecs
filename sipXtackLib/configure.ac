#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXtackLib, 4.3.2, sipx-dev@list.sipfoundry.org) 

AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

SFAC_SIPX_INSTALL_PREFIX
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])

AC_CONFIG_SRCDIR([src/net/SipMessage.cpp])
AM_CONFIG_HEADER([config.h])
AC_PROG_LIBTOOL

# Checks for programs.
AC_PATH_PROG([BASH], [bash])
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC
CHECK_XARGS_REPLACE

# Checks for libraries.
#AC_SEARCH_LIBS([res_init], [resolv])
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

# Checks for header files.
# Checks for typedefs, structures, and compiler characteristics.
# Checks for library functions.

CHECK_CPPUNIT
CHECK_SSL
# CHECK_RPM_DEBUG

# Note that SFAC_STRICT_COMPILE_NO_WARNINGS_ALLOWED must be called before
# SFAC_INIT_FLAGS, but also that SFAC_LIB_PORT will call SFAC_INIT_FLAGS also.
SFAC_STRICT_COMPILE_NO_WARNINGS_ALLOWED
SFAC_INIT_FLAGS

# sipX dependancies

SFAC_LIB_PORT

SFAC_FEATURE_SIP_TLS

ENABLE_DOC
ENABLE_DOXYGEN
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)
AM_CONDITIONAL(GENERATE_DOXYGEN, test x$enable_doxygen = xyes)

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
  bin/Makefile
  meta/Makefile
  src/test/Makefile
  examples/Makefile
  examples/im/Makefile
  examples/xmlrpc/Makefile
  siptest/Makefile
  siptest/src/Makefile
  siptest/include/Makefile
  siptest/src/siptest/Makefile
  doc/Makefile
  dialogwatch/Makefile
])

AC_OUTPUT
