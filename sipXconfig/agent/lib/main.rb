# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.

require 'state'
require 'event/parser'
require 'event/source'
require 'server'

def process(event_source, acd_state)
  event_source.each do |event| 
    begin
      acd_state.accept(event)
    rescue EventError => error
      $stderr.puts error
    end      
  end
  return acd_state
end

def process_file(filename, state)
  reader = Events::FileReader.new(filename)
  process_reader(reader, state)
end

def process_reader(reader, state)
  parser =  Events::Parser.new
  source = Events::Source.new(reader, parser)
  process(source, state)
end

def main(filename, port)
  # create a new ACD state object
  acd_state = States::Acd.new
  
  # process events in a separate thread
  Thread.new do
    $stderr.puts "parsing: #{filename}"  
    reader = Events::TailReader.new(filename)
    process_reader(reader, acd_state)
    $stderr.puts "finish parsing: #{filename}"  
  end
  
  # create the SOAP server for our ACD state
  server = StatsServer.new(acd_state, :Port => port)
  %w( TERM INT ).each do | s |
    Signal.trap(s) do
      $stderr.puts "shutting down"
      server.shutdown
    end
  end
  trap("USR1") do
    puts "dumping state"
    puts acd_state.inspect
  end
  
  # start the server
  $stderr.puts "starting server on port #{port}"  
  server.start
end

if __FILE__ == $0
  # TODO: at the moment one can pass parameters through environment variables, it may be more convenient to do it through ARGV
  SIPXSTATS_PREFIX = ENV['SIPXSTATS_PREFIX'] || ''
  SIPXSTATS_EVENT_FILE = ENV['SIPXSTATS_EVENT_FILE'] || '/var/log/sipxpbx/sipxacd_events.log'
  SIPXSTATS_SOAP_PORT = ENV['SIPXSTATS_SOAP_PORT'] || 2000
  
  main(SIPXSTATS_PREFIX + SIPXSTATS_EVENT_FILE, SIPXSTATS_SOAP_PORT)  
end
