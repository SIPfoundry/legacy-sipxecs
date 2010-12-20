include config/project.am

@PACKAGE@_EXTRA_DIST = \
   config.properties.in \
   lib \
   etc/sipximbot-prompts.properties \
   src

SUBDIRS = \
   . \
   src \
   bin \
   etc \
   image

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties

config.properties : config.properties.in Makefile
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $(srcdir)/config.properties.in > config.properties
