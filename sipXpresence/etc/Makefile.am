include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
        $(proc_DATA:.xml=.xml.in)

procdir = \
        $(SIPX_DATADIR)/process.d

proc_DATA = \
        sipxpresence-process.xml

$(proc_DATA) : % : %.in Makefile
	@$(call SearchAndReplace, $<, $@)

CLEANFILES = $(proc_DATA)
