## Process this file with automake to produce Makefile.in
include config/project.am

EXTRA_DIST = \
   config/sipX-config.in \
   config/sipX-buildstamp.cpp.in \
   config/sipX-buildstamp.h.in \
   config/svn-version \
   BUILDSTAMP \
   SVN-VERSION \
   sipximbot.spec \
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
