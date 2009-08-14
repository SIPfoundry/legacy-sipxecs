INCLUDES = -I$(top_srcdir)/include

SUBDIRS = testlib

# Define values used in local database unit tests. This only
# happens if configure was run with --enable-dbtests.
if ENABLE_ODBC_TESTS
db_FILES = \
  OdbcWrapperTest.cpp
db_CXXFLAGS = -DTESTDATABASE=\"$(SIPXTEST_DATABASE)\" -DPOSTGRESQL_USER=\"@POSTGRESQL_USER@\"
db_deps = db_setup
PG_USER = @POSTGRESQL_USER@
SCHEMA_FILE = $(top_srcdir)/../sipXproxy/etc/database/schema.sql
else
db_FILES =
db_CXXFLAGS =
db_deps =
endif

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
TESTS = \
	testsuite \
	sipx_config_value_test

check_PROGRAMS = testsuite sandbox

testsuite_CXXFLAGS = \
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
    $(db_CXXFLAGS)

testsuite_LDADD = \
	testlib/libsipXcommserverTest.la \
   ../libsipXcommserver.la

# FAILING - leaks semaphores and eventually buildserver will proceed to fail here
# see XCS-78
#    RegistryDbTest.cpp

testsuite_SOURCES = \
    ResultSetRpcTest.cpp \
    MappingRulesUrlMappingTest.cpp \
    AuthRulesUrlMappingTest.cpp \
    FallbackRulesUrlMappingTest.cpp \
    LocationDbTest.cpp \
    UserLocationDbTest.cpp \
	 SipXecsServiceTest.cpp \
	 SharedSecretTest.cpp \
	 SipNonceDbTest.cpp \
    $(db_FILES)

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

# [DEBUG ASSISTANT] List a single unit test to isolate a unit test for
# debugging or when developing a new unit test. As courtesy remove all
# unit tests before checking in, although forgetting to has no ill
# effects. To build and run:
#
#   cd src/test
#   make sandbox
#   ./sandbox
#
#sandbox_LDFLAGS = $(testsuite_LDFLAGS)
sandbox_LDADD = $(testsuite_LDADD)

sandbox_CXXFLAGS = $(testsuite_CXXFLAGS)
sandbox_SOURCES = \
    RegistryDbTest.cpp

# Make sure database is set up if --enable-dbtests was specified
all-local: $(db_deps)

# Set up PostgreSQL database for local unit test
.PHONY: db_setup
db_setup:
	@echo "Setting up database"
	-dropdb -U $(PG_USER) $(SIPXTEST_DATABASE) &>/dev/null
	createdb -U $(PG_USER) $(SIPXTEST_DATABASE) &>/dev/null
	@if [ -f $(SCHEMA_FILE) ]; then \
    psql -U $(PG_USER) -d $(SIPXTEST_DATABASE) \
      -f $(SCHEMA_FILE) &>/dev/null; \
  else \
    @echo "Schema file $(SCHEMA_FILE) not found"; \
  fi
	@echo "Done."
