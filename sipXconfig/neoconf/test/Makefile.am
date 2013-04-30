include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(test_SRC) \
	$(test_RESOURCES) 

noinst_DATA = javac-test

test_PKGS = \
	$(neoconf_PKGS) \
	junit \
	easymock \
	easymockclassextension \
	dbunit \
	jta \
	c3p0 \
	junitperf \
	p6spy \
	xmlunit \
	xercesImpl \
	xml-apis \
	spring-mock

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

TEST_CONFIG_FILES = test.properties sipxconfig.properties
javac-test : $(TEST_CONFIG_FILES)
CLEANFILES = $(TEST_CONFIG_FILES)

test.properties sipxconfig.properties : SIPX_CONFDIR=$(top_srcdir)/etc/sipxpbx
sipxconfig.properties : $(top_srcdir)/etc/sipxpbx/sipxconfig/sipxconfig.properties.install Makefile
	@$(call SearchAndReplace,$<,$@.tmp,$(EXTRA_REPLACE))
	@echo "$$sipxconfig_properties" | tr -d '\011' >> $@.tmp
	mv $@.tmp $@

# send autoconf variables to prop file available to test env.
local.etc.dir = $(top_srcdir)/etc/sipxpbx
test.properties : Makefile
	@echo -e "$(subst $(space),\n,$(foreach V,$(CommonVariablesNames),$(V)=$($(V))))" > $@

ant_PKGS = \
	ant \
	ant-launcher \
	postgresql

ant_DEPS = $(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(ant_PKGS))

precommit : check
	$(MAKE) reset check-integration

reset : reset-db reset-acd reset-cdr;

reset-db :
	if ! `$(PG_SQL) -l | grep SIPXCONFIG_TEST > /dev/null` ; then \
	  $(PG_CREATEDB) SIPXCONFIG_TEST; \
	fi
	cd $(top_srcdir)/etc/sipxpbx/database; \
	  $(call JavaCommand,ant) org.apache.tools.ant.launch.Launcher \
	    -Dsipxconfig.db.name=SIPXCONFIG_TEST \
	    -Dsipxconfig.db.user=$(POSTGRESQL_USER) \
	    -Dlog.dir=$(abspath $(builddir)) \
	    -Dtmp.dir=$(abspath $(builddir)) \
	    -f database.xml unittest-reset

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

# customize spring beans for unit test environment, must include tab as first char otherwise
# automake treats it differently
define sipxconfig_properties
	domainManagerImpl.domainConfigFilename=$(abspath .)/domain-config
	dataSource.jdbcUrl=jdbc:postgresql://localhost/SIPXCONFIG_TEST
	cdrDataSource.jdbcUrl=jdbc:postgresql://localhost/SIPXCDR_TEST
	acdHistoryDataSource.jdbcUrl=jdbc:postgresql://localhost/SIPXACD_HISTORY_TEST
	mongoDbFactory.connectionUrl=mongodb://127.0.0.1:27017
	sysdir.mongo_ns=_TEST
	setupManagerImpl.enabled=false
	domainManagerImpl.configuredDomain=example.org
	domainManagerImpl.configuredSipDomain=example.org
	domainManagerImpl.configuredRealm=example.org
	domainManagerImpl.configuredSecret=pHJhejhCcSy5nc+UfcMF2wMN
	domainManagerImpl.configuredFqdn=$(shell hostname)
	domainManagerImpl.configuredIp=127.0.0.1
	locationsManagerImpl.primaryFqdn=$(shell hostname)
	locationsManagerImpl.primaryIp=127.0.0.1
	mongoReplicaSetManager.primaryFqdn=$(shell hostname)
endef
export sipxconfig_properties
