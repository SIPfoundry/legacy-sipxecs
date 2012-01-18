include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
        $(conf_DATA:=.in)

confdir = $(SIPX_CONFDIR)
conf_DATA = sipxacd-config

$(conf_DATA) : % : %.in Makefile
	@$(call SearchAndReplace, $<, $@)

CLEANFILES = $(conf_DATA)
