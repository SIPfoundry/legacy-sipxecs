AC_PREREQ(2.57)

AC_INIT(sipXecs-doc, 4.1.6, sipx-dev@list.sipfoundry.org)

AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

SFAC_SIPX_INSTALL_PREFIX

AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])
CHECK_AUTOCONF

AC_CONFIG_SRCDIR([coding-standard.html])

SFAC_SIPX_GLOBAL_OPTS

ENABLE_DOC
ENABLE_DOT
CHECK_DOCBOOKXML
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)
AM_CONDITIONAL(GENERATE_DOT, test x$have_dot = xYES)
AM_CONDITIONAL(GENERATE_DOCBOOK_HTML, test x$enable_xml2xhtml = xyes)
AM_CONDITIONAL(GENERATE_DOCBOOK_TXT, test x$enable_xml2txt = xyes)
AM_CONDITIONAL(GENERATE_DOCBOOK_PDF, test x$enable_xml2pdf = xyes)

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
