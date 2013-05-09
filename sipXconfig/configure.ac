AC_PREREQ(2.57)
AC_INIT(sipXconfig, 4.6.0, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
m4_include([config/java.m4])
m4_include([config/java2.m4])
m4_include([config/dart.m4])
AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_AUTOMAKE_VERSION([1.6])
AC_CONFIG_SRCDIR([neoconf/src/org/sipfoundry/sipxconfig/common/User.java])
SFAC_INIT_FLAGS
AC_PATH_PROG([OPENSSL],[openssl])
AC_PATH_PROG([BASH],[bash])
AC_PATH_PROG([DATE_BINARY],[date],"/bin/date")
AC_SUBST(SIPX_DOWNLOADDIR, [${localstatedir}/sipxdata/ftpBackup])
CHECK_XARGS_REPLACE
AC_PROG_JAVA_CC([javac])
AC_PROG_JAVA([java])
PROG_JAVA_DEP
CHECK_CLOVER
CHECK_RUBY([1.8.2])
# not avail on ruby 1.9, default on fedora 17
#CHECK_RUBY_MODULE([soap/wsdlDriver])
CHECK_RUBYGEMS([0.8.11])
CHECK_YUM
AC_PATH_PROG([WSDL2RUBY], wsd2lruby.rb, $abs_srcdir/meta/wsdl2ruby.rb)
AC_PATH_PROG([XSLTPROC], xsltproc)
CHECK_DART_SDK
CHECK_POSTGRES
CHECK_SERVICEDIR
AC_SUBST(SIPX_IS_SRC, [true])
if test "${enable_rpmbuild}" == "yes" ; then
  SIPX_IS_SRC=false
fi
AC_ARG_ENABLE([testmode], 
  AC_HELP_STRING([--enable-testmode], [Build project with unit test data in place. !!!NOTE: This is not secure. Default is off]))
AM_CONDITIONAL([TEST_MODE],[test "x$enable_testmode" = "xyes"])

AC_CONFIG_FILES([
    Makefile
    bin/Makefile
    etc/Makefile
    lib/Makefile
    neoconf/Makefile
    neoconf/src/Makefile
    neoconf/test/Makefile
    web/Makefile
    web/test/Makefile
    report/Makefile
])

AC_OUTPUT
