## Process this file with automake to produce Makefile.in
include config/project.am
include ant-targets.am

EXTRA_DIST = \
  README \
  config/sipX-config.in \
  config/sipX-buildstamp.cpp.in \
  config/sipX-buildstamp.h.in \
  config/svn-version \
  ant-targets.xml \
  build.xml \
  config.properties.in \
  sipxcdrlog.spec.in \
  sipxcdrlog.spec \
  BUILDSTAMP \
  SVN-VERSION \
  LICENSE \
  src 

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties

config.properties : config.properties.in 
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

sipxcdrlog.spec : sipxcdrlog.spec.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

