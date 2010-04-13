AC_PREREQ(2.57)

# The second argument of AC_INIT must be a version number, but this
# version number affects nothing, so we use a dummy value.  The
# version numbers that matter are the arguments of AC_INIT in
# */configure.ac.
AC_INIT(sipX, 0.0.0, sipx-dev@list.sipfoundry.org)

AC_CONFIG_AUX_DIR(config)

m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

SFAC_SIPX_GLOBAL_OPTS

SFAC_SRCDIR_EXPAND

AC_CONFIG_SUBDIRS(sipXportLib)
AC_CONFIG_SUBDIRS(sipXtackLib)
AC_CONFIG_SUBDIRS(sipXmediaLib)
AC_CONFIG_SUBDIRS(sipXmediaAdapterLib)
AC_CONFIG_SUBDIRS(sipXcallLib)

AC_CONFIG_SUBDIRS(doc)

AC_ARG_ENABLE(server,
  AC_HELP_STRING([--disable-server],
    [SIP servers, proxy, registrar, webui, mediaserver, etc. ]),
     server=no, server=yes)

if test x$server = xyes
then
  AC_CONFIG_SUBDIRS(sipXacd)
  AC_CONFIG_SUBDIRS(sipXrelay)
  AC_CONFIG_SUBDIRS(sipXbridge)
  AC_CONFIG_SUBDIRS(sipXcallController)
  AC_CONFIG_SUBDIRS(sipXcdrLog)
  AC_CONFIG_SUBDIRS(sipXcommons)
  AC_CONFIG_SUBDIRS(sipXcommserverLib)
  AC_CONFIG_SUBDIRS(sipXconfig)
  AC_CONFIG_SUBDIRS(sipXecs)
  AC_CONFIG_SUBDIRS(sipXfreeSwitch)
  AC_CONFIG_SUBDIRS(sipXivr)
  AC_CONFIG_SUBDIRS(sipXopenfire)
  AC_CONFIG_SUBDIRS(sipXpage)
  AC_CONFIG_SUBDIRS(sipXpark)
  AC_CONFIG_SUBDIRS(sipXpbx)
  AC_CONFIG_SUBDIRS(sipXpresence)
  AC_CONFIG_SUBDIRS(sipXprovision)
  AC_CONFIG_SUBDIRS(sipXproxy)
  AC_CONFIG_SUBDIRS(sipXpublisher)
  AC_CONFIG_SUBDIRS(sipXrecording)
  AC_CONFIG_SUBDIRS(sipXregistry)
  AC_CONFIG_SUBDIRS(sipXrest)
  AC_CONFIG_SUBDIRS(sipXrls)
  AC_CONFIG_SUBDIRS(sipXsaa)
  AC_CONFIG_SUBDIRS(sipXsupervisor)
fi

AC_ARG_ENABLE(tools,
  AC_HELP_STRING([--disable-tools],
    [sipXpbx tools]),
      tools=no, tools=yes)

if test x$tools = xyes
then
  AC_CONFIG_SUBDIRS(sipXtools)
  AC_CONFIG_SUBDIRS(sipXinterop)
fi

SFAC_DIST_DIR

# sipXpbx/bin/sipxecs-config is specified here because it needs to
# have @subdirs@ substituted, and @subdirs@ is loaded by AC_CONFIG_SUBDIRS,
# which is done only in this configure.ac, the only one that knows
# the names of all the components.
AC_CONFIG_FILES([
	Makefile
	top.mak
	sipXpbx/bin/sipxecs-config
])

AC_OUTPUT
