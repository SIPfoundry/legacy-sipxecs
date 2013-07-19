include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/config/sipxconfig-integration-testing.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(test_SRC) \
	$(test_RESOURCES) 

noinst_DATA = javac-test

test_PKGS = \
	$(sipxconfig_test_integration_core_PKGS)

test_RESOURCES = \
	$(shell cd $(srcdir); find org -not -name '*.java') \
	log4j.properties \
	sipxplugin.beans.xml \
	spy.properties

test_DEPS = \
	. \
	$(JAVAROOT) \
	$(neoconf_SRCDIR) \
	$(neoconf_JAVAROOT) \
	$(srcdir) \
	$(top_srcdir)/etc/sipxpbx \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons $(top_srcdir)/lib, $(test_PKGS)) \
	@SIPX_JAVADIR@/sipXconfig/agent-bindings.jar \
	@SIPX_JAVADIR@/sipXconfig/cdr-bindings.jar

test_SRC = $(shell cd $(srcdir); find org \( \
	 -name '*.java' \
	\) )

precommit : check
	$(MAKE) reset check-integration

reset : sipxconfig-reset-integration-test reset-acd reset-cdr;

$(SIPXCONFIG_TEST_CONFIG_FILES) sipxconfig-reset-integration-test : SIPX_CONFDIR = $(top_srcdir)/etc/sipxpbx
test.properties : local.etc.dir = $(top_srcdir)/etc/sipxpbx

# reset-db is a convience target, not nec.
reset-db : sipxconfig-reset-integration-test

reset-cdr :	    
	if `$(PG_SQL) -l | grep SIPXCDR_TEST > /dev/null` ; then \
	  $(PG_DROPDB) SIPXCDR_TEST; \
	fi
	$(PG_CREATEDB) SIPXCDR_TEST                                                                                                                       
	$(PG_SQL) SIPXCDR_TEST -f @SIPX_CONFDIR@/cdr/schema.sql      
	$(PG_SQL) SIPXCDR_TEST -f $(srcdir)/org/sipfoundry/sipxconfig/cdr/cdr_seed.sql

reset-acd :	    
	if `$(PG_SQL) -l | grep SIPXACD_HISTORY_TEST > /dev/null` ; then \
	  $(PG_DROPDB) SIPXACD_HISTORY_TEST; \
	fi
	$(PG_CREATEDB) SIPXACD_HISTORY_TEST
	$(PG_SQL) SIPXACD_HISTORY_TEST -f @SIPX_CONFDIR@/report/schema.sql
	$(PG_SQL) SIPXACD_HISTORY_TEST -f @SIPX_CONFDIR@/report/enable_upgrades.sql
	$(PG_SQL) SIPXACD_HISTORY_TEST -f @SIPX_CONFDIR@/report/location_fqdn.sql
	$(PG_SQL) SIPXACD_HISTORY_TEST -f $(srcdir)/org/sipfoundry/sipxconfig/acd/stats/historical/history-seed.sql

