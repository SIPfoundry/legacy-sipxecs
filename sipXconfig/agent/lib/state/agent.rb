# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.

require 'events'
require 'state'
require 'state/acd_audits'
require 'state/acd_stats'
require 'state/history'

module States
  
  class QueueInfo
    attr_accessor :sign_in_time, :sign_out_time
    
    def initialize(sign_in_time)
      @sign_in_time = sign_in_time
      @sign_out_time = nil
    end
  end
  
  class Agent < State
    IDLE = "idle"
    BUSY = "busy"
    
    attr_reader :agent_uri, :state
    
    def initialize(agent_uri)
      @agent_uri = agent_uri
      @queues = {}
      @state_change_time = nil
      @sign_in_time = nil
      @sign_out_time = nil
      @state = IDLE
    end
    
    def queues
      @queues.keys
    end
    
    def accept_PickUp(event)
      @state_change_time = event.time
      @state = BUSY
    end
    
    def accept_Transfer(event)
      @state_change_time = event.time
      if @agent_uri == event.agent_uri
        @state = BUSY # similar to pick up
      else
        @state = IDLE # similar to terminate
      end
    end
    
    def accept_Terminate(event)
      @state_change_time = event.time
      @state = IDLE
    end
    
    def accept_AgentSignIn(event)
      @sign_in_time = event.time
      @state_change_time = event.time      
      @state = IDLE
      @queues[event.queue_uri] ||= QueueInfo.new(event.time)
    end
    
    def accept_AgentSignOut(event)
      @sign_out_time = event.time
      @state_change_time = event.time
      @state = IDLE
      queue_info = @queues.delete(event.queue_uri)
      if queue_info
        queue_info.sign_out_time = event.time
        event.state.audit_agent(self, event.queue_uri, queue_info, event.time)
      end        
    end
    
    def current_state_time(now)
      if @state_change_time
        now - @state_change_time 
      else 
        0 
      end    
    end
  end  
  
  class Agents < State
    attr_reader :agents
    
    def initialize
      @agents = {}
    end
    
    def accept_AgentEvent(event)
      agent_uri = event.agent_uri      
      agent = @agents[agent_uri] ||= Agent.new(agent_uri)
      agent.accept(event)      
      if agent.queues.size == 0
        @agents.delete(agent_uri)
      end  
    end
    
    def accept_PickUp(event)
      agent_uri = event.agent_uri      
      agent = @agents[agent_uri]
      raise EventError.new("Unknown agent picked up the call: #{event.agent_uri}", event) unless agent
      agent.accept(event)
    end
    
    def items; @agents end
    def stats_class; AcdStats::Agent end
    include AcdStats::StatsArrayGenerator
  end
  
  class AgentHistory < History   
    def add(agent, queue_uri, queue_info, now)
      item = AcdStats::AgentAudit.new(agent, queue_uri, queue_info)
      super(item, now)
    end        
  end
end
