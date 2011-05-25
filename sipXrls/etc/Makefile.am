include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
        $(proc_DATA:.xml=.xml.in)

procdir = \
        $(SIPX_DATADIR)/process.d

proc_DATA = \
        sipxrls-process.xml

$(proc_DATA) : % : %.in
	@$(call SearchAndReplace, $<, $@)

CLEANFILES = $(proc_DATA)
