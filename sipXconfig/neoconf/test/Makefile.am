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

test_jetty_PKGS = \
	jetty-continuation \
	jetty-http \
	jetty-io \
	jetty-security \
	jetty-server \
	jetty-util

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
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons $(srcdir)/lib, $(test_jetty_PKGS)) \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons $(top_srcdir)/lib, $(test_PKGS)) \
	@SIPX_JAVADIR@/sipXconfig/cdr-bindings.jar

test_SRC = $(shell cd $(srcdir); find org \( \
	 -name '*.java' \
	\) )

precommit : check
	$(MAKE) reset check-integration

reset : sipxconfig-reset-integration-test reset-cdr;

# reset-db is a convience target, not nec.
reset-db : sipxconfig-reset-integration-test

reset-cdr :	    
	if `$(PG_SQL) -l | grep SIPXCDR_TEST > /dev/null` ; then \
	  $(PG_DROPDB) SIPXCDR_TEST; \
	fi
	$(PG_CREATEDB) SIPXCDR_TEST                                                                                                                       
	$(PG_SQL) SIPXCDR_TEST -f @SIPX_CONFDIR@/cdr/schema.sql      
	$(PG_SQL) SIPXCDR_TEST -f $(srcdir)/org/sipfoundry/sipxconfig/cdr/cdr_seed.sql
