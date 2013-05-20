#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'dbi'

require 'call_state_event'
require 'db/dao'
require 'utils/terminator'

# Obtains CSEs from and puts them into CSE queue
class CseReader < Dao

  # Limit the maximum number of CSEs that get processed at once to avoid
  # system lockup. This value was selected by measuring and maximizing 
  # overall throughput of the CSE records.
  MAX_CSES = 1500

  def initialize(database_url, purge_age, polling_interval, log = nil)
    super(database_url, purge_age, 'call_state_events', log)
    @last_read_id = nil
    @last_read_time = nil
    @last_cse_event_time = nil
    log.debug("Polling CSE DB every #{polling_interval} seconds.") if log
    @poll_time = polling_interval
    @synch_time = 120
    if @synch_time < @poll_time
       @synch_time = @poll_time
    end
    @synch_timeslice = @synch_time / @poll_time
    @database_url = database_url
    @log = log
    @stop = Terminator.new(polling_interval)
  end

  # Another way of fetching the row
  #        while row = sth.fetch_scroll(DBI::SQL_FETCH_NEXT)
  #          cse_queue << CseReader.cse_from_row(row)
  #        end
  # If stop time is nil reader will keep on reading the connection forever
  def run(cse_queue, start_id, start_time, stop_time = nil)
    @log.debug("Connecting CSE database. #{@database_url}") if @log
    connect do | dbh |
      if stop_time
        check_purge(dbh)
        read_cses(dbh, cse_queue, nil, start_time, stop_time)
      else
        @last_read_time ||= start_time
        @last_read_id ||= start_id
        @last_cse_loop_read = 0
        loop do
          check_purge(dbh)
          first_id = @last_read_id
          first_time = @last_read_time unless first_id
          @log.debug("Read CSEs from ID:#{first_id} Time:#{first_time}") if @log
          read_cses(dbh, cse_queue, first_id, first_time, nil)

          # If we read less than MAX_CSES then we're done reading for now.
          if ((first_id.to_i + MAX_CSES) > @last_read_id.to_i)
             @log.debug("Going to sleep. Connection #{@database_url.host}:#{@database_url.port}") if @log
             break if @stop.wait
             @log.debug("Waking up. Connection #{@database_url.host}:#{@database_url.port}") if @log
          end
        end
      end
    end
  rescue DBI::DatabaseError, DBI::OperationalError, DBI::ProgrammingError => excp
    @log.error("Loss of connection to database #{@database_url} - retrying to connect after sleep") if @log
    if !@stop.wait
       dbh.disconnect
       retry
    end
  rescue
    @log.error("Exception in reader thread: <#{$!}> - Database = #{@database_url}") if @log
    # save current exception so that we can re-raise it
    Thread.current[:exception] = $!
    raise
  ensure
    @log.debug("Stopping CSE reader - #{@database_url.host}:#{@database_url.port}.") if @log
  end

  def stop
    @log.debug("CSE Reader - #{@database_url.host}:#{@database_url.port} - calling stop") if @log
    @stop.stop
  end

  # set start_id or start_time but not both
  def read_cses(dbh, cse_queue, start_id, start_time, stop_time)
    sql = CseReader.select_sql(start_id, start_time, stop_time)
    params = [start_id, start_time, stop_time].find_all { | i | i }
    # FIXME: converting Timestamp to strings for now, not sure why it's not working without conversion yet
    params = params.collect{ | i | i.to_s }
    reccount = 0
    dbh.prepare(sql) do | sth |
      sth.execute(*params)
      sth.fetch do |row|
        cse = CseReader.cse_from_row(row)
        @last_read_id = cse.id
        reccount += 1
        # since we have last_read_id we do not need time
        @last_read_time = nil
        @last_cse_event_time = cse.event_time
        cse_queue << cse
      end
    end
    if ( reccount == 0 )
       @last_cse_loop_read += 1
       if ( @last_cse_loop_read >= @synch_timeslice ) &&  (@last_cse_event_time)
          # no new CSEs.  Inject a synchronize event with a suggested event time as the polling time in order to advance the last event time in the state.
          # A CSE with a nil cse_id is the trigger that indicates that the cse is a time synch event.
          cse = CallStateEvent.new
          @last_cse_event_time = cse.event_time = DBI::Timestamp.new(@last_cse_event_time.to_time() + @synch_time )
          @log.debug("Injecting synchronize event: new time #{cse.event_time}") if @log
          cse_queue << cse

          # re-init the no cse's read loop counter because we've sent a time synch event.
          @last_cse_loop_read = 0
       end
    else
       # re-init the cse's read loop counter because we've succeeded in reading an actual new cse.
       @last_cse_loop_read = 0
    end
  end

  def purge_now(dbh, start_time_cse)
    @log.debug("Purging CSEs older than #{start_time_cse}") if @log
    sql = CseReader.delete_sql(nil, start_time_cse)
    dbh.prepare(sql) do | sth |
      sth.execute(start_time_cse)
    end
  end

  class << self
    def cse_from_row(row)
      cse = CallStateEvent.new
      CallStateEvent::FIELDS.each_with_index do
        | field, i |
        setter = (field.to_s + "=").to_sym
        cse.send( setter, row[i] )
      end
      return cse
    end

    def select_sql(start_id = nil, start_time = nil, end_time = nil)
      field_names = CallStateEvent::FIELDS.collect { | f | f.to_s }
      select_str = field_names.join(', ')
      sql = "SELECT #{select_str} FROM call_state_events"
      sql = append_where_clause(sql, start_id, start_time, end_time)
      sql += " ORDER BY event_time"
      sql += " LIMIT #{MAX_CSES}"
    end

    def delete_sql(start_time, end_time)
      sql  = "DELETE FROM call_state_events"
      append_where_clause(sql, nil, start_time, end_time)
    end

    def append_where_clause(sql, start_id, start_time, end_time)
      if start_id || start_time || end_time
        sql += " WHERE"
        sql += " id > ?" if start_id
        sql += " AND" if start_id && (start_time || end_time)
        sql += " event_time >= ?" if start_time
        sql += " AND" if start_time && end_time
        sql += " event_time <= ?" if end_time
      end
      return sql
    end
  end
end
