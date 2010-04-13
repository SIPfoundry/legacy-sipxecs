require 'dbi'

# defaultDriver.rb references default.rb that thinks its in root path
CLIENT_BINDINGS = File.join(File.dirname(__FILE__), "agent_client")
$:.unshift CLIENT_BINDINGS
require 'defaultDriver.rb'

def datetime_to_utc_time(datetime) 
  # DBI module does not consider timezone to convert to UTC
  Time.parse(datetime.new_offset.strftime("%c"))
end

module DBI
  class Timestamp
  
    # DBI module does not consider timezone to convert to UTC
    def to_utc_datetime
      DateTime.civil(year, mon, day, hour, min, sec, 0)
    end
  end
end

module Reports

  class Importer
    
    attr_accessor :dbi

    def run(url)
      service = AcdStatsService.new(url)
      dao = CallStatDao.new(@dbi)
      from_time = dao.from_time
      puts "requesting call stats from #{from_time} (class = #{from_time.class.name})"
      stats = service.getCallHistory(from_time)

      dao.persist(stats)
      
      dao = AgentStatDao.new(@dbi)
      from_time = dao.from_time
      puts "requesting agent stats from #{from_time} (class = #{from_time.class.name})"
      stats = service.getAgentHistory(from_time)

      dao.persist(stats)
    end 
    
    def run_stat(service)
    end
        
  end
  
  class Dao
  
    def initialize(table, dbi = nil)
      @dbi = dbi
      @table = table
    end
    
    def table
      @table
    end

    def columns
      return @columns if defined? @columns
      @columns = load_columns
    end
    
    def load_columns
      statement = @dbi.execute("SELECT * from #{table} limit 0")
      columns = statement.column_names
      # remove primary key
      columns.delete("#{table}_id")
      return columns
    end
    
    def insert_sql()
      Dao.insert_sql(table, columns)    
    end
    
    def Dao.insert_sql(table, columns)    
      sql = "INSERT into #{table} (#{columns.join(',')}) VALUES (?"
      for i in 1...columns.length 
        sql << ",?" 
      end
      sql << ")"
    end

    def get_param(obj, column)
      return ::LOCATION_FQDN if column == "location_fqdn"
      coerse_value(column, obj.send(column))
    end

    def coerse_value(column, value)      
      case value 
        # after reading code in DBI, DateTime turn into DBI::Date 
        # because DateTime extends Date and not Time.
        when ::DateTime 
          return datetime_to_utc_time(value)
      end
      return value
    end

    def valid?(obj)
      true
    end
    
    def persist(objs)
      sth = nil
      puts "objects received from server #{objs.size}"
      begin   
        sth = @dbi.prepare(insert_sql) 
        persist_count = 0
        objs.each do |obj|
          next if ! valid?(obj)
          column_index = 0
          columns.each do |column|
            param = get_param(obj, column)
            sth.bind_param(column_index += 1, param, false)
          end
          sth.execute()
          persist_count += 1
        end
        puts "record saved #{persist_count}"
      ensure
        sth.finish if sth
      end    
    end
  end
  
  class StatDao < Dao
  
    def initialize(table, from_time_sql, dbi = nil)
      super(table, dbi)
      @from_time = DateTime.new(0)
      @ignore_from = []      
      @from_time_sql = from_time_sql
    end
    
    def from_time
      load_window()
      @from_time
    end
        
    def load_window
      # generally 0-1 rows
      @dbi.select_all(@from_time_sql) do |row|
        @ignore_from << row[0]
        @from_time = row[1].to_utc_datetime # all rows should be same
      end
    end
    
  end  
  
  class CallStatDao < StatDao
  
    def initialize(dbi = nil)
      table = 'acd_call_stat'
from_sql = <<FROM_SQL
SELECT c.from_uri, c.terminate_time from #{table} c, 
 (SELECT max(terminate_time) as max_terminate_time from #{table}) max_results 
 where c.terminate_time = max_results.max_terminate_time
FROM_SQL
      super(table, from_sql, dbi)
    end
    
    def get_param(obj, column) 
      case column
        when /\bfrom_uri\b/ : return obj.from
      end
      super(obj, column)
    end
    
    def valid?(obj)
      if @from_time.eql?(obj.terminate_time)
        if @ignore_from.include? obj.from
          return false
        end
      end
      super(obj)
    end
    
  end
    
  class AgentStatDao < StatDao

    def initialize(dbi = nil)
      table = 'acd_agent_stat'
from_sql = <<FROM_SQL
SELECT a.agent_uri, a.sign_out_time from #{table} a, 
 (SELECT max(sign_out_time) as max_sign_out_time from #{table}) max_results 
 where a.sign_out_time = max_results.max_sign_out_time
FROM_SQL
      super(table, from_sql, dbi)
    end

    def valid?(obj)
      if @from_time.eql?(obj.sign_out_time)
        if @ignore_from.include? obj.agent_uri
          return false
        end
      end
      super(obj)
    end
    
  end
end
