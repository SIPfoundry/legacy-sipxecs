# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.

require 'events'
require 'monitor'
require 'state'
require 'state/agent'
require 'state/call'
require 'state/queue'
require 'state/acd_stats'

module States

  class Acd < State
    include MonitorMixin    
    
    def initialize
      @start_time = nil
      @stop_time = nil
      @calls = Calls.new
      @agents = Agents.new
      @queues = Queues.new
      @call_history = CallHistory.new
      @agent_history = AgentHistory.new
      super
    end    
    
    def accept(event)
      synchronize do        
        event.state = self
        super(event)
        @calls.accept(event)
        @agents.accept(event)
        @queues.accept(event)
      end        
    end
    
    def accept_Start(event)
      @start_time = event.time
      @stop_time = nil
      @calls = Calls.new
      @agents = Agents.new
      @queues = Queues.new
    end
    
    def accept_Stop(event)
      @stop_time = event.time
    end
    
    def get_running_time(now)
      States.duration(@start_time, @stop_time, now)
    end
    
    def agent_stats(now)
      synchronize do        
        @agents.stats(self, now)
      end
    end
    
    def queue_stats(now)
      synchronize do
        @queues.stats(self, now)
      end
    end
    
    def call_stats(now)
      synchronize do
        @calls.stats(self, now)
      end
    end
    
    def audit_call(call, now)
      @call_history.add(call, now)
    end
    
    def audit_agent(agent, queue_uri, queue_info, now)
      @agent_history.add(agent, queue_uri, queue_info, now)
    end
    
    def call_history(from_time)
      synchronize do        
        @call_history.get_history(from_time)
      end        
    end
    
    def agent_history(from_time)
      synchronize do        
        @agent_history.get_history(from_time)
      end
    end
    
    def calls
      @calls.calls.values
    end
    
    def agents
      @agents.agents.values
    end
    
    def agent(agent_uri)
      @agents.agents[agent_uri]
    end
    
    def queue(queue_uri)
      @queues.queues[queue_uri]
    end    
  end  
end
