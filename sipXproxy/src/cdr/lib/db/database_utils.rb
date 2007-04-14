#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

# system requires
require 'ipaddr'

# application requires
require 'db/database_url'
require 'utils/exceptions'


# DatabaseUtils contains utility methods that are primarily for test support.
class DatabaseUtils
  # Utils has only class methods, so don't allow instantiation.
  private_class_method :new

  # Constants
  
  # Name of the PostgreSQL command-line interface executable.
  POSTGRESQL_CLI = 'psql'

  # This is the first DB created by PostgreSQL.  Use it to do meta-queries like
  # which DBs exist.
  POSTGRESQL_INITIAL_DATABASE = 'template1'
  
  POSTGRESQL_SCHEMA_FILE = File.join('data', 'schema.sql')
    
  # Query to list PostgreSQL DBs.
  # See http://www.postgresql.org/docs/8.0/static/managing-databases.html#MANAGE-AG-OVERVIEW .
  QUERY_LIST_DATABASES = 'SELECT datname FROM pg_database;'
  
public

  # Execute a SQL command, accessing the database
  def DatabaseUtils.exec_sql(sql, db_name = POSTGRESQL_INITIAL_DATABASE)
    `#{POSTGRESQL_CLI} -U #{DatabaseUrl::USERNAME_DEFAULT} -d #{db_name} -c "#{sql}"`
  end

  # Execute SQL commands on the database, loading commands from a file.
  # If the quiet arg is true then suppress output.
  def DatabaseUtils.exec_sql_file(file, db_name = POSTGRESQL_INITIAL_DATABASE, quiet = true)
    db_cmd = "#{POSTGRESQL_CLI} -U #{DatabaseUrl::USERNAME_DEFAULT} -d #{db_name} -f \"#{file}\""
    db_cmd += " 2>/dev/null" if quiet
    `#{db_cmd}`
  end

  # Return a boolean indicating whether the named DB exists.
  # Comparison of db_name is case-sensitive.
  def DatabaseUtils.database_exists?(db_name)
    # Returns a list of DB names, each of which is followed by \n
    str = exec_sql(QUERY_LIST_DATABASES)
    
    # Find the DB name.  Including the \n terminator rules out substring matches.
    str.index(db_name + "\n") != nil
  end
  
  # Create a database instance with the specified name if one does not already
  # exist, and load the CDR schema.
  def DatabaseUtils.create_cdr_database(db_name)
    if !database_exists?(db_name)
      `createdb -U #{DatabaseUrl::USERNAME_DEFAULT} #{db_name}`
      puts "Schema file #{POSTGRESQL_SCHEMA_FILE}"
      exec_sql_file(POSTGRESQL_SCHEMA_FILE, db_name)
    end
  end

  # Delete the database instance with the specified name if it exists
  def DatabaseUtils.drop_database(db_name)
    if database_exists?(db_name)
      `dropdb -U #{DatabaseUrl::USERNAME_DEFAULT} #{db_name}`
    end
  end  

end
