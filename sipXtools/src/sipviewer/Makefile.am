
include $(top_srcdir)/config/subdir.am
include ant-targets.am

EXTRA_DIST = \
    ant-targets.xml \
    ant-targets.am \
    config.properties.in \
	 build.xml \
	 src \
	 bin

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties

config.properties : config.properties.in Makefile
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $(srcdir)/config.properties.in > config.properties

