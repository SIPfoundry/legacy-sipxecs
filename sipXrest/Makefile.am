## Process this file with automake to produce Makefile.in
include config/project.am
include ant-targets.am

bin_SCRIPTS = sipxrest.sh 

EXTRA_DIST = \
  README \
  config/sipX-config.in \
  config/sipX-buildstamp.cpp.in \
  config/sipX-buildstamp.h.in \
  config/svn-version \
  ant-targets.xml \
  build.xml \
  config.properties.in \
  sipxrest.spec.in \
  sipxrest.spec \
  sipxrest-process.xml.in \
  sipxrest.sh.in \
  BUILDSTAMP \
  SVN-VERSION \
  LICENSE \
  ant-targets.xml \
  src 

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties

SIPX_PROCESS_DEFS = \
	sipxrest

config.properties : config.properties.in 
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

sipxrest.sh : sipxrest.sh.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

sipxrest.spec : sipxrest.spec.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

