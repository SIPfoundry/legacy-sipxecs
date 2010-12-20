include config/project.am
include ant-targets.am

@PACKAGE@_EXTRA_DIST = \
  README \
  ant-targets.xml \
  build.xml \
  config.properties.in \
  LICENSE \
  src 

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties

config.properties : config.properties.in 
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@
