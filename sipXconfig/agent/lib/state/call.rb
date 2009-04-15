# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.

require 'events'
require 'state'
require 'state/history'
require 'state/acd_stats'
require 'state/acd_audits'

module States
  
  class Call < State  
    attr_reader :queue_uri, :agent_uri, :state, :call_id, :from
    attr_reader :enter_time, :pick_up_time, :terminate_time
    
    # Values need to match constantct in AcdCallStats.java
    WAITING = "waiting"
    IN_PROGRESS = "in_progress"
    TERMINATE = "terminate"
    UNANSWERED = "unanswered"
    
    def initialize(call_id)
      @call_id = call_id
      @agent_uri = nil
      @state = nil
      @enter_time = nil
      @pick_up_time = nil
      @terminate_time = nil
      @from = nil
    end
    
    def accept_EnterQueue(event)    
      @state = WAITING
      @enter_time = event.time if ! @enter_time
      # current queue
      @queue_uri = event.queue_uri
      @from = event.from
    end
    
    def accept_PickUp(event)
      @state = IN_PROGRESS
      @pick_up_time = event.time
      @agent_uri = event.agent_uri
      # this should be the same as current queue - but just in case we did not get proper enter queue event...
      # it means the call was quietly rolled over to another queue
      @queue_uri = event.queue_uri
      @from = event.from
    end
    
    def accept_Terminate(event)
      @state = if @state == IN_PROGRESS then TERMINATE else UNANSWERED end
      @terminate_time = event.time
      # if agent_uri is set we need to inform the agent that call has been terminated
      if @queue_uri
        queue = event.state.queue(@queue_uri)
        wait_time = total_wait_time(@terminate_time)
        processing_time = processing_time(@terminate_time)
        queue.add_terminated_call(@terminate_time, wait_time, processing_time, @state) if queue
      end
      if @agent_uri
        agent = event.state.agent(@agent_uri)
        agent.accept(event) if agent
      end
    end
    
    def accept_Transfer(event)
      old_agent = event.state.agent(@agent_uri)
      old_agent.accept(event) if old_agent
      @agent_uri = event.agent_uri
      new_agent = event.state.agent(@agent_uri) 
      new_agent.accept(event) if new_agent      
    end    
    
    def processing_time(now)
      States.duration(@pick_up_time, @terminate_time, now)
    end
    
    def total_wait_time(now)
      States.duration(@enter_time, @pick_up_time, now)
    end
  end
  
  class Calls < State
    attr_reader :calls
    
    def initialize
      @calls = {}
    end
    
    def accept_EnterQueue(event)
      call = @calls[event.call_id] ||= Call.new(event.call_id)
      call.accept(event)
    end
    
    def accept_PickUp(event)
      call = @calls[event.call_id]
      raise EventError.new("Unknown call pick up: #{event.call_id}", event) unless call
      call.accept(event)
    end    
    
    def accept_Terminate(event)
      call = @calls[event.call_id]
      raise EventError.new("Unknown call entered queue: #{event.call_id}", event) unless call
      call.accept(event)
      event.state.audit_call(call, event.time)
      @calls.delete(event.call_id)
    end
    
    def accept_Transfer(event)
      call = @calls[event.call_id]
      raise EventError.new("Unknown call transferred: #{call_id}", event) unless call
      call.accept(event)
    end
    
    def items; @calls end    
    def stats_class; AcdStats::Call end
    
    include AcdStats::StatsArrayGenerator
  end
    
  class CallHistory < History   
    def add(call, now)
      item = AcdStats::CallAudit.new(call)      
      super(item, now)
    end        
  end
end
