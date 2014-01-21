AC_PREREQ(2.57)
AC_INIT(sipxexample, 4.6.0, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE(foreign tar-ustar)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

# java projects - typical macros
m4_include([config/java.m4])
m4_include([config/java2.m4])
m4_include([config/dart.m4])

# cpp projects - typical macros
AC_CONFIG_SRCDIR([cpp-executable/src/ExampleApp.cpp])
AM_CONFIG_HEADER([cpp-executable/config.h])
AM_DISABLE_STATIC
AC_PROG_LIBTOOL
AC_PROG_CXX
AC_PROG_CC
AX_COMPILER_VENDOR
SFAC_INIT_FLAGS
SFAC_LIB_COMMSERVER

# java projects
AC_PROG_JAVA_CC([javac])
AC_PROG_JAVA([java])
PROG_JAVA_DEP

# sipXconfig UI plugins
CHECK_DART_SDK

# projects that interact w/postgres
CHECK_POSTGRES

# ruby projects - typical macros
CHECK_RUBY([1.8.2])
CHECK_RUBYGEMS([0.8.11])
AM_CONDITIONAL(IS_RPMBUILD, test x$enable_rpmbuild = xyes)

AC_CONFIG_FILES([
    Makefile
    etc/Makefile
    bin/Makefile
    config-plugin/Makefile
    config-plugin/test/Makefile
    cpp-executable/Makefile
    cpp-executable/src/Makefile
    cpp-library/Makefile
    cpp-library/src/Makefile
    cpp-library/include/Makefile
    ruby-module/Makefile
])
AC_OUTPUT
