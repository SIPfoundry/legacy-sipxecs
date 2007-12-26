#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'logger'

require 'call_direction/call_direction_plugin'
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
    @failed_calls_cleaner = Cleaner.new(@config.min_cleanup_interval, [:flush_failed_calls,  @config.max_failed_wait])
    @state = nil
  end
  
  def run_resolver
    resolve(@writer.last_cdr_start_time, nil)
  end
  
  # Resolve CSEs to CDRs
  def resolve(start_time, end_time)
    start_run = Time.now
    
    # start everything    
    
    cdr_queue = Queue.new
    cse_queue = Queue.new    

    reader_threads = @readers.collect do | reader |
      # TODO: we are passing nil as CSE start_id at them moment
      # it would be better if we stored the last read id for every CSE database
      Thread.new(reader, cse_queue) { |r, q| r.run(q, nil, start_time, end_time) }
    end

    long_calls_cleaner_thread = Thread.new(@long_calls_cleaner, cse_queue) do |cleaner, queue|
      cleaner.run(queue)
    end
    failed_calls_cleaner_thread = Thread.new(@failed_calls_cleaner, cse_queue) do |cleaner, queue|
      cleaner.run(queue)
    end

    # state copies from CSE queue to CDR queue
    @state = State.new(cse_queue, cdr_queue)
    
    Thread.new( @state ) { | state | 
      state.run
    }
    
    # create the SOAP server
    @server = CdrResolver::SOAP::Server.new(@state, @config)
    
    Thread.new( @server ) { | server | 
      server.start      
    }
    
    #FIXME: enable call direction plugin
    #cdr_queue = start_plugins(cdr_queue)
    
    writer_thread = Thread.new( @writer, cdr_queue ) { | w, q | w.run(q) }    
    
    reader_threads.each{ |thread| thread.join }

    # stop running housekeeping jobs
    @long_calls_cleaner.stop
    @faled_calls_cleaner.stop
    long_calls_cleaner_thread.join
    faled_calls_cleaner_thread.join

    # send sentinel event, it will stop plugins and state
    cse_queue.enq(nil)
    
    # wait for writer before exiting
    writer_thread.join
    
    @server.shutdown
    
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
  end  
  
  def start_plugins(raw_queue)
    return raw_queue unless CallDirectionPlugin.call_direction?(@config)    
    processed_queue = Queue.new
    
    cdp = CallDirectionPlugin.new(raw_queue, processed_queue)  
    Thread.new(cdp) { | plugin | plugin.run }
    
    return processed_queue
  end  
end
