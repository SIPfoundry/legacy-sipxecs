# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.

require 'soap/mapping'
require 'state/acd_stats'

# Definition of objects for ACDHistoricalStatsService


module AcdStats
  
  class AgentAudit
    @@schema_type = "AgentAudit"
    @@schema_ns = StatsServerNamespace
    
    attr_accessor :agent_uri
    attr_accessor :queue_uri
    attr_accessor :sign_in_time
    attr_accessor :sign_out_time
    
    def initialize(agent, queue_uri, queue_info)
      @agent_uri = agent.agent_uri
      @queue_uri = queue_uri
      @sign_in_time = Time.at(queue_info.sign_in_time)
      @sign_out_time = Time.at(queue_info.sign_out_time)
    end
    
    def time
      @sign_out_time
    end
  end
  
  class CallAudit
    @@schema_type = "CallAudit"
    @@schema_ns = StatsServerNamespace
    
    attr_accessor :from
    attr_accessor :state
    attr_accessor :agent_uri
    attr_accessor :queue_uri
    attr_accessor :enter_time
    attr_accessor :pick_up_time
    attr_accessor :terminate_time
    
    def initialize(call)
      @from = call.from
      @state = call.state
      @agent_uri = call.agent_uri
      @queue_uri = call.queue_uri
      @enter_time = Time.at(call.enter_time)
      @pick_up_time = call.pick_up_time ? Time.at(call.pick_up_time) : nil
      @terminate_time = Time.at(call.terminate_time)
    end
    
    def time
      @terminate_time
    end
  end    
end
