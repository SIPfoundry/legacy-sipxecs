## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/project.am
include ant-targets.am

bin_SCRIPTS = sipxbridge.sh

sipxbridge.sh: sipxbridge.sh.in
	$(LocalizeSipXconfig) $< > $@

RPMS=$(RPM)

.PHONY : editscripts
EXTRA_DIST = \
  README \
  config/sipX-config.in \
  config/sipX-buildstamp.cpp.in \
  config/sipX-buildstamp.h.in \
  config/svn-version \
  ant-targets.xml \
  build.xml \
  CONTRIBUTORS \
  BUILDSTAMP \
  SVN-VERSION \
  LICENSE \
  config.properties \
  ant-targets.xml \
  sipxbridge.spec \
  sipxbridge.process.xml \
  src \
  lib

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties

config.properties : config.properties.in Makefile
	$(LocalizeSipXconfig) $(srcdir)/config.properties.in > config.properties

sipxbridge.process.xml : sipxbridge.process.xml.in
	$(LocalizeSipXconfig) $< > $@

