include config/project.am

@PACKAGE@_EXTRA_DIST = \
  README \
  config.properties.in \
  LICENSE \
  src

SUBDIRS = \
   . \
   src \
   bin \
   etc \
   regression \
   schema

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties

config.properties : config.properties.in Makefile
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $(srcdir)/config.properties.in > config.properties

