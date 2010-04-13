BUILD_DIR = ENV['builddir'] || '../build/report'

# HACK : Cannot get Gem Specification to include files that live in 
# relative paths. Until I come up w/different plan, generate ruby 
# into source directory.  
# DIST_DIR = "#{BUILD_DIR}/dist"
DIST_DIR = "lib"

CLIENT_BINDINGS = "#{DIST_DIR}/agent_client"
DB_USER = ENV['DB_USER'] || 'postgres'
TEST_DB = ENV['TEST_DB'] || 'SIPXCONFIG_REPORT_TEST'
DB_URI = "dbi:Pg:#{TEST_DB}:localhost" if ! defined? DBURI
