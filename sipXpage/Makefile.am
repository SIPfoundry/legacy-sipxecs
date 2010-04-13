## Process this file with automake to produce Makefile.in
include config/project.am

RPMS = $(RPM)

audio_path=@SIPX_VARDIR@/sipxpage/music

AUDIO_DIST = \
   audio/attention.wav \
   audio/fanfare.wav \
   audio/fluffy.wav \
   audio/notice.wav \
   audio/nowhearthis.wav \
   audio/reveille.wav \
   audio/TadaTada.wav

EXTRA_DIST = \
   config/sipX-config.in \
   config/sipX-buildstamp.cpp.in \
   config/sipX-buildstamp.h.in \
   config/svn-version \
   BUILDSTAMP \
   SVN-VERSION \
   sipxpage.spec \
   config.properties.in \
   lib \
   src


SUBDIRS = \
   . \
   src \
   bin \
   etc \
   audio

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties

config.properties : config.properties.in Makefile
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $(srcdir)/config.properties.in > $(top_builddir)/config.properties
