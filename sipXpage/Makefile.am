include config/project.am

audio_path=@SIPX_VARDIR@/sipxpage/music

AUDIO_DIST = \
   audio/attention.wav \
   audio/fanfare.wav \
   audio/fluffy.wav \
   audio/notice.wav \
   audio/nowhearthis.wav \
   audio/reveille.wav \
   audio/TadaTada.wav

@PACKAGE@_EXTRA_DIST = \
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
