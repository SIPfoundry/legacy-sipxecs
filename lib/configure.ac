AC_PREREQ(2.57)
AC_INIT(sipXlib, 3.9.0, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
AM_INIT_AUTOMAKE(foreign)

SFAC_SRCDIR_EXPAND

AC_ARG_ENABLE(suse10,
        AC_HELP_STRING([--enable-suse10], [Enable dependencies for openSUSE 10.2]),
        [
          w3c_libwww=yes
          cgicc=yes
          ruby_dbi=yes
        ])

AC_ARG_ENABLE(fc4,
	AC_HELP_STRING([--enable-fc4], [Enable dependencies for Fedora Core 4]),
	[ 
          java_sun=yes
          ruby_gems=yes
          ruby_postgres=yes
          xerces_c=yes
          cgicc=yes
          ruby_dbi=yes
          soap4r=yes
          http_access=yes
        ])

AC_ARG_ENABLE(fc5,
	AC_HELP_STRING([--enable-fc5], [Enable dependencies for Fedora Core 5]),
	[ 
          java_sun=yes
          ruby_gems=yes
          cgicc=yes
          ruby_dbi=yes
        ])

AC_ARG_ENABLE(fc6,
	AC_HELP_STRING([--enable-fc6], [Enable dependencies for Fedora Core 6]),
	[ 
          java_sun=yes
          ruby_gems=yes
          cgicc=yes
          ruby_dbi=yes
        ])

AC_ARG_ENABLE(f7,
        AC_HELP_STRING([--enable-f7], [Enable dependencies for Fedora 7]),
        [
          java_sun=yes
          ruby_gems=yes
          cgicc=yes
          ruby_dbi=yes
        ])

AC_ARG_ENABLE(rhe4,
	AC_HELP_STRING([--enable-rhe4], [Enable dependencies for RHE4 and Centos4]),
	[ 
          java_sun=yes
          w3c_libwww=yes
          xerces_c=yes
          cppunit=yes
          ruby=yes
          ruby_gems=yes
          ruby_postgres=yes
          cgicc=yes
          ruby_dbi=yes
          soap4r=yes
          http_access=yes
        ])

AC_ARG_ENABLE(rhe5,
        AC_HELP_STRING([--enable-rhe5], [Enable dependencies for RHE5 and Centos5]),
        [
          java_sun=yes
          w3c_libwww=yes
          xerces_c=yes
          ruby_gems=yes
          ruby_postgres=yes
          cgicc=yes
          ruby_dbi=yes
          cppunit=yes
        ])

AC_ARG_ENABLE(soap4r,
	AC_HELP_STRING([--enable-soap4r], [Build soap4r dependency]),
	[ soap4r=${enableval} ])
test x$soap4r = xyes && AC_CONFIG_SUBDIRS(soap4r)

AC_ARG_ENABLE(http-access,
	AC_HELP_STRING([--enable-http-access], [Build http-access dependency]),
	[ http_access=${enableval} ])
test x$http_access = xyes && AC_CONFIG_SUBDIRS(http-access)

AC_ARG_ENABLE(cgicc,
	AC_HELP_STRING([--enable-cgicc], [Build cgicc dependency]),
	[ cgicc=${enableval} ])
test x$cgicc = xyes && AC_CONFIG_SUBDIRS(cgicc)

AC_ARG_ENABLE(cppunit,
	AC_HELP_STRING([--enable-cppunit], [Build cppunit dependency]),
	[ cppunit=${enableval} ])
test x$cppunit = xyes && AC_CONFIG_SUBDIRS(cppunit)

AC_ARG_ENABLE(java-ibm,
	AC_HELP_STRING([--enable-java-ibm], [Build IBM Java SDK dependency]),
	[ java_ibm=${enableval} ])
test x$java_ibm = xyes && AC_CONFIG_SUBDIRS(java-ibm)

AC_ARG_ENABLE(java-sun,
	AC_HELP_STRING([--enable-java-sun], [Build Sun Java SDK dependency]),
	[ java_sun=${enableval} ])
test x$java_sun = xyes && AC_CONFIG_SUBDIRS(java-sun)

AC_ARG_ENABLE(pcre,
	AC_HELP_STRING([--enable-pcre], [Build pcre dependency]),
	[ pcre=${enableval} ])
test x$pcre = xyes && AC_CONFIG_SUBDIRS(pcre)

AC_ARG_ENABLE(ruby,
	AC_HELP_STRING([--enable-ruby], [Build ruby dependency]),
	[ ruby=${enableval} ])
test x$ruby = xyes && AC_CONFIG_SUBDIRS(ruby)

AC_ARG_ENABLE(ruby-dbi,
	AC_HELP_STRING([--enable-ruby-dbi], [Build ruby dbi dependency]),
	[ ruby_dbi=${enableval} ])
test x$ruby_dbi = xyes && AC_CONFIG_SUBDIRS(ruby-dbi)

AC_ARG_ENABLE(ruby-gems,
	AC_HELP_STRING([--enable-ruby-gems], [Build ruby gems dependency]),
	[ ruby_gems=${enableval} ])
test x$ruby_gems = xyes && AC_CONFIG_SUBDIRS(ruby-gems)

AC_ARG_ENABLE(ruby-postgres,
	AC_HELP_STRING([--enable-ruby-postgres], [Build ruby postgres dependency]),
	[ ruby_postgres=${enableval} ])
test x$ruby_postgres = xyes && AC_CONFIG_SUBDIRS(ruby-postgres)

AC_ARG_ENABLE(w3c-libwww,
	AC_HELP_STRING([--enable-w3c-libwww], [Build w3c-libwww w/ssl dependency]),
	[ w3c_libwww=${enableval} ])
test x$w3c_libwww = xyes && AC_CONFIG_SUBDIRS(w3c-libwww)

AC_ARG_ENABLE(xerces-c,
	AC_HELP_STRING([--enable-xerces-c], [Build xerces-c dependency]),
	[ xerces_c=${enableval} ])
test x$xerces_c = xyes && AC_CONFIG_SUBDIRS(xerces-c)

SFAC_DIST_DIR
   
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([top.mak])

AC_OUTPUT

