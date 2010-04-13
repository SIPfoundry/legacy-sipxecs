## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/project.am
include ant-targets.am

bin_SCRIPTS = sipxrelay.sh

RPMS=$(RPM)

EXTRA_DIST = \
  README \
  config/sipX-config.in \
  config/sipX-buildstamp.cpp.in \
  config/sipX-buildstamp.h.in \
  config/svn-version \
  ant-targets.xml \
  build.xml \
  config.properties.in \
  sipxrelay.spec.in \
  sipxrelay.spec \
  sipxrelay-process.xml.in \
  sipxrelay-process.xml \
  sipxrelay.sh.in \
  CONTRIBUTORS \
  BUILDSTAMP \
  SVN-VERSION \
  LICENSE \
  ant-targets.xml \
  src 

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties

SIPX_PROCESS_DEFS = \
	sipxrelay

config.properties : config.properties.in 
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

sipxrelay.sh : sipxrelay.sh.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

sipxrelay.spec : sipxrelay.spec.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

sipxrelay-process.xml : sipxrelay-process.xml.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@
