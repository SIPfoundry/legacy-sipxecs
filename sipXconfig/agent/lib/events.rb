# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.

# base class for all events types in the system
class Event
  attr_reader :time
  attr_accessor :state
  
  def initialize(time)
    @time = time;
  end 
end

# usually event out of order or with unexpected data
class EventError < StandardError
  attr_reader :event

  def initialize(msg, event)
    super(msg)
    @event = event
  end
end

module AcdEvents
  
  class Start < Event; end
  
  class Stop < Event; end
  
  # base class for all call events
  class CallEvent < Event
    attr_reader :call_id, :from
    
    def initialize(time, from, call_id = nil)
      super(time)
      # if call_id is not set use from field as call_id
      @call_id = @from = from
      @call_id = call_id if call_id
    end
  end
  
  class EnterQueue < CallEvent
    attr_reader :queue_uri
    
    def initialize(time, queue_uri, from, call_id = nil)
      super(time, from, call_id)
      @queue_uri = queue_uri
    end 
  end
  
  class Terminate < CallEvent; end
  
  class PickUp < CallEvent
    attr_reader :agent_uri, :queue_uri
    
    def initialize(time, queue_uri, agent_uri, from, call_id = nil)
      super(time, from, call_id)
      @agent_uri = agent_uri
      @queue_uri = queue_uri
    end    
  end
  
  class Transfer < CallEvent
    attr_reader :agent_uri
    
    def initialize(time, agent_uri, from, call_id = nil)
      super(time, from, call_id)
      @agent_uri = agent_uri      
    end    
  end
  
  # base class for all agent events
  class AgentEvent < Event
    attr_reader :agent_uri, :queue_uri    
    
    def initialize(time, queue_uri, agent_uri)
      super(time)
      @agent_uri = agent_uri
      @queue_uri = queue_uri
    end
  end
  
  class AgentSignIn < AgentEvent; end
  
  class AgentSignOut < AgentEvent; end
end
