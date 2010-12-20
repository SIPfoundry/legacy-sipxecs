include config/project.am
include ant-targets.am

bin_SCRIPTS = sipxrelay.sh

@PACKAGE@_EXTRA_DIST = \
  README \
  ant-targets.xml \
  build.xml \
  config.properties.in \
  sipxrelay-process.xml.in \
  sipxrelay-process.xml \
  sipxrelay.sh.in \
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

sipxrelay-process.xml : sipxrelay-process.xml.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@
