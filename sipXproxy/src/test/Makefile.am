
if ENABLE_CSE_TESTS
TEST_CSE_OPT = cse_test.sh
endif

TESTS = proxytest $(TEST_CSE_OPT)

check_PROGRAMS = \
	proxytest

COMMON_CXX_FLAGS = \
	-DTEST_WORK_DIR=\"@abs_builddir@\" \
	-DTEST_DATA_DIR=\"@abs_top_srcdir@/src/test\" \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/lib/authplugins

COMMON_LIBS = \
   @SIPXUNIT_LIBS@ \
   @SIPXCOMMSERVERTEST_LIBS@ \
   @SIPXCOMMSERVER_LIBS@ \
	../libsipXproxy.la 

proxytest_CXXFLAGS = \
	$(COMMON_CXX_FLAGS)

proxytest_SOURCES = \
   ForwardRulesTest.cpp \
   CallStateEventBuilder_XMLTest.cpp \
   CallStateEventBuilder_DBTest.cpp \
   RouteStateTest.cpp \
   SipRouterTest.cpp 

proxytest_LDFLAGS = \
    -rdynamic

proxytest_LDADD = \
	$(COMMON_LIBS)

EXTRA_DATA = \
   rulesdata/simple.xml \
   siproutertestdata/routing.xml \
   siproutertestdata/credential.xml

EXTRA_DIST = \
  $(EXTRA_DATA) \
  cse_test.rb \
  environment.rb \
  sipsupport.rb

# [DEBUG ASSISTANT] List a single unit test to isolate a unit test for
# debugging or when developing a new unit test. As courtesy remove all
# unit tests before checking in, although forgetting to has no ill
# effects. To build and run:
#
#   cd src/test
#   make sandbox
#   ./sandbox
#
#sandbox_LDFLAGS = $(proxytest_LDFLAGS)

#sandbox_LDADD = $(proxytest_LDADD)

#sandbox_CXXFLAGS = $(proxytest_CXXFLAGS)
#sandbox_SOURCES = \
#   SipRouterTest.cpp


# disabled testsuite, this entire makefile needs to be refactored, this
# was giving errors in 'make dist' target and non-standard usage of build
# switches had to be removed -- DLH

# Define values used in local database unit tests. This only
# happens if configure was run with --with-dbtests.
db_deps = db_setup
PG_USER = @POSTGRESQL_USER@

SCHEMA_FILE = $(top_srcdir)/etc/database/schema.sql

CSE_TEST_FILES = \
  $(srcdir)/cse_test.rb \
  $(srcdir)/environment.rb \
  $(srcdir)/sipsupport.rb

# Let make check create a small shell script to start the real Ruby 
# file. This satisfies the automake requirement that a testsuite must 
# be an executable file that is created during make check.
# check_PROGRAMS = $(testsuite)

# Create simple shell script to start up Ruby, pass in the correct
# parameters. First copy all Ruby script files from src/cse/test
# to the build directory so all required files can be found.
cse_test.sh: $(db_deps) $(CSE_TEST_FILES)
	@echo "Generating test start script"
	@cp $(srcdir)/*.rb .
	@echo "ruby cse_test.rb -- --verbose --prefix=${prefix} --dbname=$(SIPXTEST_DATABASE)" \
         > cse_test.sh
	@chmod +x cse_test.sh

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
