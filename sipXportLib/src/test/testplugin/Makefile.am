## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/subdir.am

## HACK
##    We don't need these libraries installed, so we declare them noinst_
##    but that has the unfortunate side effect that automake then assumes
##    (reasonably) that there is no point in making them shared, which
##    is wrong because the PluginTest needs to dlopen them...
noinst_LTLIBRARIES = libtestpluginA.la libtestpluginB.la
##    So... we explicitly add the -rpath that automake would normally provide
##    when building a shared library.
libtestpluginA_la_LDFLAGS = \
	-rpath @abs_builddir@ \
	-version-info ${version_Current}:${version_Revision}:${version_Age}

libtestpluginB_la_LDFLAGS = $(libtestpluginA_la_LDFLAGS)

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/test/utl

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0


libtestpluginA_la_LIBADD = \
	@RT_LIBS@  \
	-lpthread  \
	-lstdc++

libtestpluginB_la_LIBADD = $(libtestpluginA_la_LIBADD)

## Compile the same source twice, changing the resulting class name
##    to create two different plugins
libtestpluginA_la_CXXFLAGS = \
	-DLIBRARY_NAME=\"TestPluginA\" \
	-DTestPlugin=TestPluginA

libtestpluginB_la_CXXFLAGS = \
	-DLIBRARY_NAME=\"TestPluginB\" \
	-DTestPlugin=TestPluginB

libtestpluginA_la_SOURCES =  \
   TestPlugin.cpp

libtestpluginB_la_SOURCES =  $(libtestpluginA_la_SOURCES)

DISTCLEANFILES = Makefile.in
