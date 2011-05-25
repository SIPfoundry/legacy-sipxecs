include $(top_srcdir)/config/utility.am

SUBDIRS = \
	. \
	database

EXTRA_DIST = \
        $(proc_DATA:.xml=.xml.in) \
	$(cfg_DATA:=.in)

procdir = \
        $(SIPX_DATADIR)/process.d

proc_DATA = \
	sipxcallresolver-agent-process.xml \
	sipxcallresolver-process.xml

cfgdir = @SIPX_CONFDIR@
cfg_DATA = \
	sipxcallresolver-agent-config

$(cfg_DATA) $(proc_DATA) : % : %.in
        @$(call SearchAndReplace, $<, $@)

CLEANFILES = $(proc_DATA) $(cfg_DATA)
