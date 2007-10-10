AC_PREREQ(2.57)
AC_INIT(sipX, 3.9.3, sipx-dev@list.sipfoundry.org)

AC_CONFIG_AUX_DIR(config)

m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

SFAC_SIPX_GLOBAL_OPTS

AM_INIT_AUTOMAKE(foreign)

SFAC_SRCDIR_EXPAND

AC_CONFIG_SUBDIRS(sipXportLib)
AC_CONFIG_SUBDIRS(sipXtackLib)
AC_CONFIG_SUBDIRS(sipXmediaLib)
AC_CONFIG_SUBDIRS(sipXmediaAdapterLib)
AC_CONFIG_SUBDIRS(sipXcallLib)

AC_ARG_ENABLE(server, 
  AC_HELP_STRING([--disable-server], 
    [SIP servers, proxy, registrar, webui, mediaserver, etc. ]), 
     server=no, server=yes)

if test x$server = xyes
then
  AC_CONFIG_SUBDIRS(sipXcommserverLib)
  AC_CONFIG_SUBDIRS(sipXproxy)
  AC_CONFIG_SUBDIRS(sipXpublisher)
  AC_CONFIG_SUBDIRS(sipXregistry)
  AC_CONFIG_SUBDIRS(sipXvxml)
  AC_CONFIG_SUBDIRS(sipXconfig)
  AC_CONFIG_SUBDIRS(sipXacd)
  AC_CONFIG_SUBDIRS(sipXpbx)
fi

AC_ARG_ENABLE(tools,
  AC_HELP_STRING([--disable-tools],
    [sipXpbx tools]),
      tools=no, tools=yes)

if test x$tools = xyes
then
  AC_CONFIG_SUBDIRS(sipXtools)
fi

SFAC_DIST_DIR
   
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([top.mak])

AC_OUTPUT
