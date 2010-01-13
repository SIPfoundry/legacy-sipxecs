#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'logger'
require 'thwait'

require 'db/cse_reader'
require 'db/cdr_writer'
require 'soap/server'
require 'utils/cleaner'
require 'utils/configure'
require 'state'


# The CallResolver analyzes call state events (CSEs) and computes call detail
# records (CDRs).  It loads CSEs from a database and writes CDRs back into the
# same database.
class CallResolver
  attr_reader :log

  def initialize(config)
    @config =  config
    @log = config.log
    urls = @config.cse_database_urls
    purge_age_cse =  @config.purge_age_cse
    # readers put events in CSE queue
    @readers = urls.collect do | url |
      CseReader.new(url, purge_age_cse, config.cse_polling_interval, log)
    end
    install_signal_handler(@readers)
    @writer = CdrWriter.new(@config.cdr_database_url, @config.purge_age_cdr, log)
    @long_calls_cleaner = Cleaner.new(@config.min_cleanup_interval, [:retire_long_calls,  @config.max_call_len])
    @long_ringing_calls_cleaner = Cleaner.new(@config.min_cleanup_interval, [:retire_long_ringing_calls,  @config.max_ringing_call_len])
    @failed_calls_cleaner = Cleaner.new(@config.min_cleanup_interval, [:flush_failed_calls,  @config.max_failed_wait])
    @state = nil
  end

  # make sure we can connect to the CDR DB. The CSE DB connections will be checked/(re-)established on running the reader.
  def check_connections
    @writer.test_connection
  rescue
    log.error("Problems with CDR DB connection.")
    raise
  end

  def run_resolver
    resolve(@writer.last_cdr_start_time, nil)
  end

  # Resolve CSEs to CDRs
  def resolve(start_time, end_time)
    start_run = Time.now

    # limit the size of the queues: it's better to get starved than too eat all available memory
    @log.debug("CSE Queue Size = #{@config.cse_queue_size}  CDR Queue Size = #{@config.cdr_queue_size}")
    cdr_queue = SizedQueue.new(@config.cdr_queue_size)
    cse_queue = SizedQueue.new(@config.cse_queue_size)

    # start everything
    reader_threads = @readers.collect do | reader |
      # TODO: we are passing nil as CSE start_id at the moment
      # it would be better if we stored the last read id for every CSE database
      Thread.new(reader, cse_queue) do |r, q|
        r.run(q, nil, start_time, end_time)
      end
    end

    long_calls_cleaner_thread = Thread.new(@long_calls_cleaner, cse_queue) do |cleaner, queue|
      cleaner.run(queue)
    end
    failed_calls_cleaner_thread = Thread.new(@failed_calls_cleaner, cse_queue) do |cleaner, queue|
      cleaner.run(queue)
    end
    long_ringing_calls_cleaner_thread = Thread.new(@long_ringing_calls_cleaner, cse_queue) do |cleaner, queue|
      cleaner.run(queue)
    end

    # state copies from CSE queue to CDR queue
    @state = State.new(cse_queue, cdr_queue)
    @state.log = @log

    Thread.new( @state ) do | state |
      state.run
    end

    # create the SOAP server
    @server = CdrResolver::SOAP::Server.new(@state, @config)

    Thread.new( @server ) do | server |
      server.start
    end

    writer_thread = Thread.new( @writer, cdr_queue ) do | w, q |
      w.run(q)
    end

    ThreadsWait.all_waits(reader_threads) do | t |
      # re-raise the exception if any of the reader threads terminated abnormally
      raise t[:exception] if t.status.nil?
    end
    log.debug("CSE Readers threads stopped.")
    log.debug("CSEs in Queue = #{cse_queue.size()}")

    # stop running housekeeping jobs
    @long_calls_cleaner.stop
    @failed_calls_cleaner.stop
    @long_ringing_calls_cleaner.stop

    ThreadsWait.all_waits(long_calls_cleaner_thread, failed_calls_cleaner_thread, long_ringing_calls_cleaner_thread)
    log.debug("Clean-up threads stopped.")

  ensure
    @server.shutdown if @server

    # send sentinel event, it will stop plugins and state
    cse_queue.enq(nil)

    # wait for writer before exiting - but do not wait forever (all readers might be dead)
    writer_thread.join(10) if writer_thread
    log.debug("CDR writer thread stopped.")

    log.info("resolve: Done. Analysis took #{Time.now - start_run} seconds.")
  end

  # install handler for INT (2) and TERM (9) signals to cleanly terminate readers threads
  def install_signal_handler(readers)
    %w( TERM INT ).each do | s |
      Signal.trap(s) do
        log.info("#{s} intercepted. Terminating reader threads.")
        readers.each { |r| r.stop() }
      end
    end
    Signal.trap("USR1") do
      log.debug(@state.to_s)
    end
    Signal.trap("ABRT") do
      # A signal abort has been received.  Handle it by simply exiting.
      exit 0
    end
  end
end
