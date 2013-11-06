SUBDIRS= . sipXecsService sipdb

INCLUDES = -I$(top_srcdir)/include
SCHEMA_FILE = $(top_srcdir)/../sipXproxy/etc/database/schema.sql

AM_CPPFLAGS = \
	-DTEST_DATA_DIR=\"@abs_top_srcdir@/src/test\" \
	-DTEST_WORK_DIR=\"@abs_top_builddir@/src/test/work\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\" \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DSIPX_BINDIR=\"@SIPX_BINDIR@\" \
	-DSIPX_LIBEXECDIR=\"@SIPX_LIBEXECDIR@\" \
	-DSIPX_DBDIR=\"@SIPX_DBDIR@\" \
	-DSIPXECS_NAME=\"@SIPXECS_NAME@\" \
	-DTESTDATABASE=\"$(SIPXTEST_DATABASE)\" \
	-DPOSTGRESQL_USER=\"@POSTGRESQL_USER@\"

LDADD = \
	@SIPXUNIT_LIBS@ \
	-lmongoclient \
	../libsipXcommserver.la

# Define values used in local database unit tests. This only
# happens if configure was run with --enable-dbtests.
if ENABLE_ODBC_TESTS
db_TESTS = OdbcWrapperTest
db_SETUP = db_setup
endif

check : $(db_SETUP)

TESTS = \
	ResultSetRpcTest \
	MappingRulesUrlMappingTest \
	AuthRulesUrlMappingTest \
	FallbackRulesUrlMappingTest \
	SipXecsServiceTest \
	SharedSecretTest \
	$(db_TESTS)

check_PROGRAMS = $(TESTS)


ResultSetRpcTest_SOURCES = ResultSetRpcTest.cpp
MappingRulesUrlMappingTest_SOURCES = MappingRulesUrlMappingTest.cpp
AuthRulesUrlMappingTest_SOURCES = AuthRulesUrlMappingTest.cpp
FallbackRulesUrlMappingTest_SOURCES = FallbackRulesUrlMappingTest.cpp
SipXecsServiceTest_SOURCES = SipXecsServiceTest.cpp
SharedSecretTest_SOURCES = SharedSecretTest.cpp
OdbcWrapperTest_SOURCES = OdbcWrapperTest.cpp

EXTRA_DIST = \
	sharedsecret/domain-config \
	mapdata/digits.xml \
	mapdata/escape.xml \
	mapdata/params.xml \
	mapdata/simple.xml \
	mapdata/specials.xml \
	mapdata/userpat.xml \
    mapdata/vdigits.xml \
    mapdata/authrules.xml \
    mapdata/fallbackrules.xml \
	regdbdata/getMaxUpdate.xml \
	sipx_config_value/testdata

# Set up PostgreSQL database for local unit test
db_setup:
	@echo "Setting up database"
	-dropdb -U $(POSTGRESQL_USER) $(SIPXTEST_DATABASE) &>/dev/null
	createdb -U $(POSTGRESQL_USER) $(SIPXTEST_DATABASE) &>/dev/null
	@if [ -f $(top_srcdir)/../sipXproxy/etc/database/schema.sql ]; then \
	psql -U $(POSTGRESQL_USER) -d $(SIPXTEST_DATABASE) \
	  -f $(SCHEMA_FILE) &>/dev/null; \
	else \
	  @echo "Schema file $(SCHEMA_FILE) not found"; \
	fi
	@echo "Done."

