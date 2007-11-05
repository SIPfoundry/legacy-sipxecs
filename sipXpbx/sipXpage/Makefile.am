## Process this file with automake to produce Makefile.in

include $(top_srcdir)/config/subdir.am

audio_path=@SIPX_PAGEMUSICDIR@

AUDIO_DIST = \
   audio/attention.wav \
   audio/fanfare.wav \
   audio/fluffy.wav \
   audio/notice.wav \
   audio/nowhearthis.wav \
   audio/reveille.wav \
   audio/TadaTada.wav

# (src/build.xml installs these)
JAR_DIST = \
   lib/JainSipApi-1.2.jar \
   lib/jain-sip-ri-1.2.92.jar \
   lib/junit-3.8.1.jar \
   lib/log4j-1.2.14.jar \
   lib/nist-sdp-1.0.58.jar \
   lib/sip-sdp-1.0.jar

EXTRA_DIST = \
   config.properties.in \
   $(AUDIO_DIST) \
   $(JAR_DIST) \
   lib

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
	@for f in $(AUDIO_DIST); \
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

