include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(proc_DATA:=.in)

audiodir = @SIPX_VARDIR@/sipxpage/music
dist_audio_DATA = \
	audio/attention.wav \
	audio/fanfare.wav \
	audio/fluffy.wav \
	audio/notice.wav \
	audio/nowhearthis.wav \
	audio/reveille.wav \
	audio/TadaTada.wav

procdir = $(SIPX_DATADIR)/process.d
proc_DATA = sipxpage-process.xml

$(proc_DATA) : % : %.in Makefile
	@$(call SearchAndReplace, $<, $@)

CLEANFILES = $(proc_DATA)
