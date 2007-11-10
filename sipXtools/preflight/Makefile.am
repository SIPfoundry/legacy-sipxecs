## Process this file with automake to produce Makefile.in

include $(top_srcdir)/config/subdir.am
include ant-targets.am

EXTRA_DIST = \
  config.properties.in \
  ant-targets.xml \
  build.xml \
  preflight.mf \
  icons \
  src \
  lib

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties

config.properties : config.properties.in Makefile
	$(LocalizeSipXconfig) $(srcdir)/config.properties.in > config.properties

