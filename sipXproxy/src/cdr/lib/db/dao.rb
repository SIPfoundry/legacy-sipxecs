#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'dbi'

require 'utils/configure'

module DBI
  class Timestamp
    include Comparable
    
    def <=>(other)
      result = to_time <=> other.to_time
      return result unless result == 0
      return fraction <=> other.fraction
    end
    
    def -(other)
      return to_time <=> other.to_time
    end
    
  end
end


class Dao
  attr_reader :log
  
  def initialize(database_url, purge_age, table_name, log)
    @connection = database_url.to_dbi
    @username = database_url.username
    @purge_age = purge_age
    @table_name = table_name
    @last_purge_time = nil    
    @log = log
  end
  
  def connect(&block)
    DBI.connect(@connection, @username, &block)
  end
  
  # this is run to test if DB connection is working
  def test_connection
    connect do | dbh |
      check_purge(dbh)
    end
  end
  
  # purge unconditionally
  def purge(time)
    connect do | dbh |
      purge_now(dbh, time)
      vacuum_now(dbh, @table_name)
      @last_purge_time = now
    end    
  end
  
  # purge at least once a day
  def check_purge(dbh)
    return unless @purge_age
    now = Time.now
    if @last_purge_time.nil? || now - @last_purge_time > Configure::SECONDS_PER_DAY
      first_entry_time = now - (@purge_age * Configure::SECONDS_PER_DAY)
      purge_now(dbh, first_entry_time)
      @last_purge_time = now
    end
  end
  
  def vacuum_now(dbh, table_name)
    sql = "VACUUM ANALYZE #{table_name}"
    dbh.prepare(sql) do | sth |
      sth.execute
    end      
  end    
end
