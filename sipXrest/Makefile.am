include config/project.am
include ant-targets.am

bin_SCRIPTS = sipxrest.sh 

@PACKAGE@_EXTRA_DIST = \
  README \
  ant-targets.xml \
  build.xml \
  config.properties.in \
  sipxrest-process.xml.in \
  sipxrest.sh.in \
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
