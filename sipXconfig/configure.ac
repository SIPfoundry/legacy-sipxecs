AC_PREREQ(2.57)
AC_INIT(sipXconfig,
  m4_esyscmd([config/git-version-gen .tarball-version | sed 's/\(.*\)\.\([0-9]\+\)-\([0-9a-z\-]\+\)/\1/g']),
  sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/java.m4])
m4_include([config/sipXlib.m4])
AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_AUTOMAKE_VERSION([1.6])
AC_CONFIG_SRCDIR([neoconf/src/org/sipfoundry/sipxconfig/common/User.java])
SFAC_INIT_FLAGS
AC_PATH_PROG([OPENSSL],[openssl])
AC_PATH_PROG([BASH],[bash])
AC_PATH_PROG([DATE_BINARY],[date],"/bin/date")
AC_SUBST(SIPX_DOWNLOADDIR, [${localstatedir}/sipxdata/ftpBackup])
AC_SUBST(BUILDDIR_EXPAND, [`cd $builddir && pwd`])
AM_CONDITIONAL(IS_RPMBUILD, test x$enable_rpmbuild = xyes)
CHECK_XARGS_REPLACE
CHECK_JDK
CHECK_ANT
CHECK_CLOVER
CHECK_RUBY([1.8.2])
CHECK_RUBY_MODULE([soap/wsdlDriver])
CHECK_GEM([0.8.11])
CHECK_RAKE([0.7])
CHECK_YUM
AC_ARG_ENABLE(reports,
  AC_HELP_STRING([--enable-reports], [Build optional call distribution reporting cron]),
  enable_reports=yes,)
AM_CONDITIONAL(REPORTS, test x$enable_reports = xyes)
if test x$enable_reports = xyes ; then
  CHECK_CRON
fi

AC_ARG_ENABLE(agent,
  AC_HELP_STRING([--enable-agent], [Build optional call distribution api]),
  enable_agent=yes,)
AM_CONDITIONAL(AGENT, test x$enable_agent = xyes)
if test false && x$enable_agent = xyes ; then
  CHECK_RUBY_GEM([file-tail])
fi

AC_ARG_ENABLE(mrtg,
  AC_HELP_STRING([--enable-mrtg], [Enable MRTG integration]),
  enable_mrtg=yes,)
AM_CONDITIONAL(MRTG, test x$enable_mrtg = xyes)
AC_SUBST(MRTG_ENABLED, [])
if test x$enable_mrtg = xyes ; then
  MRTG_ENABLED="mrtg-enabled=true"
else
  MRTG_ENABLED="mrtg-enabled=false"
fi

AC_ARG_WITH(mrtg-binary-location,[  --with-mrtg-binary-location=PFX   Location where mrtg is installed (optional)],
            mrtg_binary_location="$withval/mrtg", mrtg_binary_location="/usr/bin/mrtg")
AC_SUBST(MRTG_BINARY, $mrtg_binary_location)

AC_ARG_ENABLE(ruby-bindings,
  AC_HELP_STRING([--enable-ruby-bindings], [Build optional ruby bindings to web services]),
  enable_ruby_binding=yes,)

AC_SUBST(RUBY_BINDINGS_ENABLED, [])
if test x$enable_ruby_binding == xyes ; then
  RUBY_BINDINGS_ENABLED="ruby-bindings-enabled="
  SFAC_SRCDIR_EXPAND
  AC_PATH_PROG([WSDL2RUBY], wsd2lruby.rb, $abs_srcdir/web/meta/wsdl2ruby.rb)
fi
CHECK_POSTGRES
CHECK_SERVICEDIR
AC_CONFIG_FILES([
    Makefile
    bin/Makefile
    neoconf/Makefile
    gwt/Makefile
    web/Makefile
    meta/Makefile
    agent/Makefile
    report/Makefile
    report/etc/Makefile
    report/bin/Makefile
    report/src/Makefile
    mrtg/Makefile
    mrtg/bin/Makefile
    mrtg/etc/Makefile
    plugins/Makefile
    plugins/aastra/Makefile
    plugins/ciscoplus/Makefile
    plugins/clearone/Makefile
    plugins/gtek/Makefile
    plugins/gtek-aq/Makefile
    plugins/hitachi/Makefile
    plugins/ipdialog/Makefile
    plugins/karel/Makefile
    plugins/lg-nortel/Makefile
    plugins/linksys/Makefile
    plugins/mitel/Makefile
    plugins/nortel/Makefile
    plugins/nortel12x0/Makefile
    plugins/counterpath/Makefile
    plugins/cmcprov/Makefile
    plugins/openfire/Makefile
    plugins/polycom/Makefile
    plugins/snom/Makefile
    plugins/unidata/Makefile
    plugins/isphone/Makefile
])

AC_OUTPUT

# autoconf variables like datadir, localstatedir, etc are not resolved
# until make time, so here we get a jump start on this mainly for ANT
# purist that don't do make and got right in ant
make build.properties
