# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.

require 'events'
require 'state'
require 'state/acd'
require 'state/agent'
require 'state/acd_stats'
require 'state/history'
require 'state/terminated_calls'

module States
  
  class Queue < State
    attr_reader :queue_uri, :agents
    
    def initialize(queue_uri)
      @queue_uri = queue_uri
      @agents = []
      @tc = TerminatedCalls.new
      @uc = TerminatedCalls.new
      @pc = TerminatedCalls.new
    end  
    
    def accept_AgentSignIn(event)    
      @agents << event.agent_uri unless @agents.include?(event.agent_uri) 
    end
    
    def accept_AgentSignOut(event)
      @agents.delete(event.agent_uri)
    end
    
    # returns an array with [busy, idle] agents count
    def busy_idle_agents(agents)
      agents.inject([0, 0]) do |busy_idle, agent|
        if agent.queues.include?(@queue_uri)
          i = if agent.state == Agent::BUSY then 0 else 1 end
          busy_idle[i] += 1
        end
        busy_idle   
      end
    end
    
    def add_terminated_call(timestamp, wait_time, processing_time, state)
      @tc.add_call(timestamp, wait_time)

      if state == Call::UNANSWERED
        # update unanswered calls stats for abandoned calls
        @uc.add_call(timestamp, wait_time)
      elsif state == Call::TERMINATE
        @pc.add_call(timestamp, processing_time)
      end
    end
    
    def waiting_calls(calls, now)
      terminated_info = @tc.total_count_max(now)
      waiting_count = 0
      total, count, max = calls.inject(terminated_info) do |totals, call|
        if call.queue_uri == @queue_uri
          wait_time = call.total_wait_time(now)
          totals[0] += wait_time
          totals[1] += 1
          totals[2] = wait_time if wait_time > totals[2]
          waiting_count += 1 if call.state == Call::WAITING
        end
        totals
      end
      [waiting_count, safe_avg(total, count), max]
    end
    
    def abandoned_calls(now)
      total, count, max = @uc.total_count_max(now)
      [count, safe_avg(total, count), max]
    end
    
    def processed_calls(now)
      total, count, max = @pc.total_count_max(now)
      [count, safe_avg(total, count), max]    
    end
    
    def safe_avg(total, count)
      if count == 0
        0 
      else 
        total / count 
      end
    end    
  end
  
  class Queues < State
    attr_reader :queues
    
    def initialize
      @queues = {}  
    end
    
    def accept_Event(event)
      if event.respond_to?(:queue_uri)
        queue_uri = event.queue_uri
        queue = @queues[queue_uri] ||= Queue.new(queue_uri)
        queue.accept(event)
      end
    end
    
    def items; @queues end
    def stats_class; AcdStats::Queue end
    
    include AcdStats::StatsArrayGenerator    
  end
end
