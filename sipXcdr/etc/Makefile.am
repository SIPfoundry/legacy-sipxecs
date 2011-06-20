include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
        $(proc_DATA:=.in)

sqldir = @SIPX_CONFDIR@/cdr
dist_sql_DATA = \
	database/schema.sql \
	database/refer_uri.sql \
	database/view_cdrs.sql \
	database/index_on_time.sql \
	database/reference.sql \
	database/contact.sql \
	database/calldirection.sql \
	database/cdrremote.sql \
	database/transaction.sql

procdir = \
        $(SIPX_DATADIR)/process.d

proc_DATA = \
	sipxcallresolver-process.xml

$(proc_DATA) : % : %.in Makefile
	@$(call SearchAndReplace, $<, $@)

CLEANFILES = $(proc_DATA)
