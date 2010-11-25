## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/project.am
include ant-targets.am

bin_SCRIPTS = 

EXTRA_DIST = \
  README \
  config/sipX-config.in \
  config/sipX-buildstamp.cpp.in \
  config/sipX-buildstamp.h.in \
  config/svn-version \
  ant-targets.xml \
  build.xml \
  config.properties.in \
  sipxcallcontroller.spec.in \
  sipxcallcontroller.spec \
  BUILDSTAMP \
  SVN-VERSION \
  LICENSE \
  plugin.xml \
  src 

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties

config.properties : config.properties.in 
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@


sipxcallcontroller.spec : sipxcallcontroller.spec.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

