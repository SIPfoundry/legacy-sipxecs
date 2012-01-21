include $(top_srcdir)/config/utility.am

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

cfinputsdir = @SIPX_CFINPUTS@/plugin.d
dist_cfinputs_DATA = \
	sipxcdr.cf