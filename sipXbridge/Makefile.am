## Process this file with automake to produce Makefile.in

include $(top_srcdir)/config/subdir.am
include ant-targets.am

EXTRA_DIST = \
  README \
  config.properties.in \
  ant-targets.xml \
  build.xml \
  src \
  lib

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties

config.properties : config.properties.in Makefile
	$(LocalizeSipXconfig) $(srcdir)/config.properties.in > $(abs_builddir)/config.properties

