include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(proc_DATA:=.in)

propsdir = @SIPX_CONFDIR@/imbot
dist_props_DATA = sipximbot-prompts.properties

imagedir = @SIPX_VARDIR@/sipximbot/image
dist_image_DATA = avatar.jpg

procdir = $(SIPX_DATADIR)/process.d
proc_DATA = sipximbot-process.xml

$(proc_DATA) : % : %.in
	@$(call SearchAndReplace, $<, $@)

CLEANFILES = $(proc_DATA) 
