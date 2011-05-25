include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(proc_DATA:=.in)

procdir = \
	$(SIPX_DATADIR)/process.d

proc_DATA = \
	sipxopenacd-process.xml

$(proc_DATA) : % : %.in
	@$(call SearchAndReplace, $<, $@)

logdir = @SIPX_LOGDIR@/openacd
vardir = @SIPX_VARDIR@/openacd

CLEANFILES = $(proc_DATA)
