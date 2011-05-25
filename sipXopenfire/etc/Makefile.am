include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(proc_DATA:=.in) \
	$(etc_DATA:=.in)

cfgdir = @SIPX_JAVADIR@/sipXopenfire/conf
dist_cfg_DATA = \
	openfire.xml \
	web.xml.enable_openfire_admin_console \
	web.xml.disable_openfire_admin_console

# need to copy over jars from sipxcommons
ofldir = @SIPX_JAVADIR@/sipXopenfire/lib
ofl_DATA = \
	$(call JRepo, @SIPX_JAVADIR@/sipXcommons, \
	  ws-commons-utl \
	  xmlrpc-server \
	  xmlrpc-client \
        )

dbdir = @SIPX_CONFDIR@/database
dist_db_DATA = \
	sipxopenfire-initdb.sql

procdir = $(SIPX_DATADIR)/process.d
proc_DATA = \
	sipxopenfire-process.xml

etcdir = @SIPX_CONFDIR@/openfire
dist_etc_DATA = \
	sipxopenfire-prompts.properties \
	openfire.properties \
	openfire.vm \
	openfire.xml \
	sipxopenfire.vm

etc_DATA = \
	sipxopenfire-database.xml

$(etc_DATA) $(proc_DATA) : % : %.in
	@$(call SearchAndReplace, $<, $@, OPENFIRE_HOME)

CLEANFILES = $(proc_DATA) $(etc_DATA)
