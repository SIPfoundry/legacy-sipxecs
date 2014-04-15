include $(top_srcdir)/config/utility.am

audiodir = @SIPX_VARDIR@/sipxpage/music
dist_audio_DATA = \
	audio/attention.wav \
	audio/fanfare.wav \
	audio/fluffy.wav \
	audio/notice.wav \
	audio/nowhearthis.wav \
	audio/reveille.wav \
	audio/TadaTada.wav

cfinputsdir = $(SIPX_CFINPUTS)/plugin.d
dist_cfinputs_DATA = \
	sipxpage.cf

EXTRA_DIST = \
	$(conf_DATA:=.in)

confdir = @SIPX_CONFDIR@/sipxpage
conf_DATA = \
	log4j.properties

$(conf_DATA) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)
