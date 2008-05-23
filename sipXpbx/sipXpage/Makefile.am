## Process this file with automake to produce Makefile.in

include $(top_srcdir)/config/subdir.am

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
   config.properties.in \
   lib \
   src \
   audio

SUBDIRS = \
   . \
   src \
   bin \
   etc 

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties

config.properties : config.properties.in Makefile
	$(LocalizeSipXconfig) $(srcdir)/config.properties.in > config.properties


install-data-hook :
	@for f in $(AUDIO_DIST) ; \
	do \
	   target=`basename $$f` ; \
	   if [ -f $(DESTDIR)$(audio_path)/$$target ]; \
	   then \
	      echo "Using existing $(audio_path)/$$target"; \
	   else \
	      echo ""; echo "Installing default $(audio_path)/$$target"; \
	      $(INSTALL) -D -m 644 $(srcdir)/$$f $(DESTDIR)$(audio_path)/$$target; \
	   fi; \
	done

