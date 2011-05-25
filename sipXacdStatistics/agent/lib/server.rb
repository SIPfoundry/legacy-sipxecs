# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.

require 'soap/rpc/standaloneServer'
require 'soap/mapping'
require 'state/acd'
require 'state/acd_stats'
require 'time'

module Utils
  def datetime2time(datetime) 
    time_s = datetime.strftime("%+")
    Time.parse(time_s)
  end
end  

class AcdStatsService
  def initialize(acd)
    @acd = acd
  end
  
  def getBirdArray()    
    robin = Bird.new
    robin.species = "robin"
    bluejay = Bird.new
    bluejay.species = "bluejay"
    return BirdArray[robin, bluejay]
  end
  
  def getStringCallStats
    "hello"
  end
  
  def getAgentStats()
    @acd.agent_stats(Time.now.to_i)    
  end
  
  def getQueueStats()
    @acd.queue_stats(Time.now.to_i)    
  end
  
  def getCallStats()
    @acd.call_stats(Time.now.to_i)
  end  
end

class AcdHistoricStatsService
  include Utils
  
  def initialize(acd)
    @acd = acd
  end
  
  def getCallHistory(from_time)
    return @acd.call_history(datetime2time(from_time))
  end
  
  def getAgentHistory(from_time)
    return @acd.agent_history(datetime2time(from_time))
  end
end

class StatsServer < SOAP::RPC::StandaloneServer
  def initialize(acd, config)
    @acdStatsService = AcdStatsService.new(acd)
    @acdHistoricStatsService = AcdHistoricStatsService.new(acd)
    
    config[:BindAddress] = '0.0.0.0'
    
    # this section only works for releases post 1_5_5 (as yet unavailable in released ruby)
    # but harmless for earlier releases
    thisdir = File.dirname __FILE__
    config[:WSDLDocumentDirectory] = thisdir    
    config[:SOAPDefaultNamespace] = StatsServerNamespace
    
    # config is not used yet - pass explicit parameters
    super('sipXconfigAgent', StatsServerNamespace, config[:BindAddress], config[:Port])
  end
  
  def on_init
    #@log.level = Logger::Severity::DEBUG
    #TODO: register services using with 2 different URLS
    add_method(@acdStatsService, 'getBirdArray')
    add_method(@acdStatsService, 'getAgentStats')
    add_method(@acdStatsService, 'getQueueStats')
    add_method(@acdStatsService, 'getCallStats')
    add_method(@acdStatsService, 'getStringCallStats')
    add_method(@acdHistoricStatsService, 'getCallHistory')
    add_method(@acdHistoricStatsService, 'getAgentHistory')    
  end  
end

BirdNamespace = 'urn:StatsService'
class Bird; include SOAP::Marshallable
  @@schema_ns = StatsServerNamespace
  attr_accessor :species
end

BirdArrayNamespace = 'urn:StatsService'
class BirdArray < Array; include SOAP::Marshallable
  @@schema_ns = StatsServerNamespace
end

if $0 == __FILE__
  # this is just a test - pass empty acd server here
  server = StatsServer.new(States::Acd.new, :Port => 2000)
  puts "starting server on port 2000"
  server.start
end
