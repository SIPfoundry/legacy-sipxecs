include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(proc_DATA:=.in)

procdir = $(SIPX_DATADIR)/process.d
proc_DATA = sipxprovision-process.xml

$(proc_DATA) : % : %.in
	@$(call SearchAndReplace, $<, $@)

CLEANFILES = $(proc_DATA)
