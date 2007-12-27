INCLUDES = -I$(top_srcdir)/include

# disabled testsuite, this entire makefile needs to be refactored, this
# was giving errors in 'make dist' target and non-standard usage of build
# switches had to be removed -- DLH

# Define values used in local database unit tests. This only
# happens if configure was run with --with-dbtests.
db_deps = db_setup
PG_USER = @POSTGRESQL_USER@
# testsuite_SCRIPTS = cse_test.sh
SCHEMA_FILE = $(top_srcdir)/etc/database/schema.sql

EXTRA_DIST = \
  cse_test.rb \
  environment.rb \
  sipsupport.rb

CSE_TEST_FILES = \
  $(srcdir)/cse_test.rb \
  $(srcdir)/environment.rb \
  $(srcdir)/sipsupport.rb

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
# TESTS = $(testsuite)

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
