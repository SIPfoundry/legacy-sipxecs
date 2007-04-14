#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'test/unit'

$:.unshift(File.join(File.dirname(__FILE__), '..', '..', 'lib'))

require 'db/database_utils'

class DatabaseUtilsTest < Test::Unit::TestCase
  def test_exec_sql
    # Test a query
    str = DatabaseUtils.exec_sql('select sqrt(2);',
                                 DatabaseUtils::POSTGRESQL_INITIAL_DATABASE)
    assert(str =~ /.*1.4142135623731.*/, 'Result must be sqrt(2)')
  end
  
  def test_database_exists?
    assert(DatabaseUtils.database_exists?(DatabaseUtils::POSTGRESQL_INITIAL_DATABASE),
           "Database #{DatabaseUtils::POSTGRESQL_INITIAL_DATABASE} must exist")
    
    nonexistent_db = "name of database that doesn't exist"
    assert(!DatabaseUtils.database_exists?(nonexistent_db),
           "Database \"#{nonexistent_db}\" must not exist")
  end
  
  def test_create_drop_database
    test_db = 'database_utils_test_db'
    
    # Delete the test DB if it exists.
    if DatabaseUtils.database_exists?(test_db)
      DatabaseUtils.drop_database(test_db)
    end
    assert(!DatabaseUtils.database_exists?(test_db))
    
    # Create it
    DatabaseUtils.create_cdr_database(test_db)
    assert(DatabaseUtils.database_exists?(test_db))
    
    # Drop it
    DatabaseUtils.drop_database(test_db)
    assert(!DatabaseUtils.database_exists?(test_db))
  end
end
