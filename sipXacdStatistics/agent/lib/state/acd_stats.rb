# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.

require 'soap/mapping'

# Definition of object send through SOAP API
# It should correspond to objects from org.sipfoundry.sipxconfig.acd.stats package

StatsServerNamespace = 'urn:StatsService'

module AcdStats
  
  class Agent; include SOAP::Marshallable
    @@schema_ns = StatsServerNamespace
    @@schema_type = 'AgentStats'
    
    attr_accessor :agent_uri, :state, :current_state_time, :queues
    
    def init(acd, agent, now)
      @agent_uri = agent.agent_uri
      @state = agent.state
      @queues = Array.new(agent.queues)
      @current_state_time = agent.current_state_time(now)
      self
    end
  end
  
  class Queue; include SOAP::Marshallable
    @@schema_ns = StatsServerNamespace
    @@schema_type = 'QueueStats'
    
    attr_accessor :queue_uri,
      :waiting_calls, :avg_wait_time, :max_wait_time, 
      :abandoned_calls, :avg_abandoned_time, :max_abandoned_time,
      :processed_calls, :avg_processing_time, :max_processing_time,
      :idle_agents, :busy_agents      
    
    def init(acd, queue, now)
      @queue_uri = queue.queue_uri
      
      @waiting_calls, @avg_wait_time, @max_wait_time = queue.waiting_calls(acd.calls, now)
      @abandoned_calls, @avg_abandoned_time, @max_abandoned_time = queue.abandoned_calls(now)
      @processed_calls, @avg_processing_time, @max_processing_time = queue.processed_calls(now)
      
      @busy_agents, @idle_agents = queue.busy_idle_agents(acd.agents)
      self
    end
  end
  
  class Call; include SOAP::Marshallable
    @@schema_ns = StatsServerNamespace
    @@schema_type = 'CallStats'    
    
    attr_accessor :from, :state, :agent_uri, :queue_uri, :total_wait_time, :processing_time
    
    def init(acd, call, now)
      @state = call.state
      @from = call.from
      @agent_uri = call.agent_uri
      @queue_uri = call.queue_uri
      @processing_time = call.processing_time(now)
      @total_wait_time = call.total_wait_time(now)
      self
    end    
  end
  
  # marshall-able version of the standard Array
  class Array < ::Array; include SOAP::Marshallable
    @@schema_ns = StatsServerNamespace  
  end
  
  module StatsArrayGenerator    
    # Requires items function to be defined - representing hash, values of which are used to fill the resulting stats objects array 
    # and stats_class representing class of the stats object
    def stats(acd, now)
      stats_array = AcdStats::Array.new
      items.each_value do | item |
        item_stats = stats_class.new
        item_stats.init(acd, item, now)      
        stats_array << item_stats
      end
      return stats_array
    end
    
  end
  
end
