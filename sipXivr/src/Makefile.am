include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

audio_path=@SIPX_VARDIR@/mediaserver/data/moh

EXTRA_DIST = \
	$(ivr_SRC) \
	$(moh_AUDIO) \
	$(ivr_RESOURCES)

noinst_DATA = javac-ivr

jardir = @SIPX_JAVADIR@/sipXivr
JAR_FILE = sipXivr.jar
jar_DATA = $(JAR_FILE)

ivr_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
ivr_RESOURCES = $(shell cd $(srcdir); find . \( -name '*.png' -o -name '*.properties' -o -name '*.xml' -o -name '*.xsd' \) )
moh_AUDIO = \
	audio/default.wav

ivr_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(ivr_PKGS)) \
	@SIPX_JAVADIR@/sipXopenfire/plugins/sipx-openfire-presence-plugin.jar

$(JAR_FILE) : javac-ivr $(ivr_RESOURCES)
	jar cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(ivr_RESOURCES))

install-exec-hook :

install-data-hook :
	@for f in $(moh_AUDIO); \
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