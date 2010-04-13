#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'cdr'

require 'db/dao'

# Writes CDRs to the database
class CdrWriter < Dao
  
  def initialize(database_url, purge_age, log = nil)
    super(database_url, purge_age, 'cdrs', log)
  end
  
  def run(queue)
    connect do | dbh |
      sql = CdrWriter.insert_sql
      dbh.prepare(sql) do | sth |
        while cdr = queue.shift
          row = CdrWriter.row_from_cdr(cdr)
          sth.execute(*row)
          check_purge(dbh)
        end
      end
    end
  rescue DBI::DatabaseError => e
    log.error("#{e.err}, #{e.errstr}")
    retry        
  end
  
  def last_cdr_start_time
    connect do | dbh |
      return dbh.select_one(CdrWriter.last_cdr_sql)
    end
    return nil    
  end
  
  def purge_now(dbh, start_time_cdr)
    log.debug("Purging CDRs older than #{start_time_cdr}")  
    sql = CdrWriter.delete_sql
    dbh.prepare(sql) do | sth |
      sth.execute(start_time_cdr)
    end  
  end
  
  class << self
    def row_from_cdr(cdr)
      Cdr::FIELDS.collect { | f | cdr.send(f) }
    end
    
    def insert_sql()
      field_names = Cdr::FIELDS.collect { | f | f.to_s }
      field_str = field_names.join(', ')
      value_str = (['?'] * field_names.size).join(', ')
      "INSERT INTO cdrs ( #{field_str} ) VALUES ( #{value_str} )"
    end        
    
    def delete_sql
      "DELETE FROM cdrs WHERE start_time < ?"
    end
    
    def last_cdr_sql
      "SELECT start_time FROM cdrs ORDER BY start_time DESC LIMIT 1"
    end
  end  
end
