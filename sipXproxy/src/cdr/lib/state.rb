#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'monitor'
require 'thread'

require 'cdr'
require 'call_state_event'
require 'dbi'
require 'utils/utils'

# Maintains currently processed CDRs.
class State
  # Temporary record of the CDR that is a candidate for failure
  # All legs are failed and there are no remaining legs open
  # It really might be a failure or we are still waiting for a successful leg
  class FailedCdr
    attr_reader :cdr, :generation
    
    def initialize(cdr, generation)
      @cdr = cdr
      @generation = generation
    end
  end

  attr_writer :log

  def initialize(cse_queue, cdr_queue, cdr_class = Cdr)
    @cse_queue = cse_queue
    @cdr_queue = cdr_queue
    @cdr_class = cdr_class
    @failed_calls = {}
    @retired_calls = {}
    @filtered_identities = [ "~~mh~" ]
    @generation = 0
    @last_event_time = nil
    @cdrs = {}
    @log = nil
    super()
  end
  
  def run
    begin
      while item = @cse_queue.shift
        if item.kind_of?(Array)
          @log.debug("Start #{item[0]}") if @log
          housekeeping(*item)
          @log.debug("Finished #{item[0]}") if @log
        else          
          accept(item)
        end            
      end
      flush_failed()
      @cdr_queue << nil
    rescue 
      @log.error($!) if @log
      raise
    end
  end
  
  def active_cdrs
    @cdrs.values
  end
  
  def dump_waiting
    @cdrs.each do | cdr |
      p(cdr.to_s)
    end
  end
  
  def dump_state
    p(to_s)
  end
  
  def to_s
    "Generation #{@generation}, Retired: #{@retired_calls.size}, Failed: #{@failed_calls.size}, Waiting: #{@cdrs.size}, CSE Queue: #{@cse_queue.length}"
  end
  
  # Analyze CSE, add CDR to queue if completed
  def accept(cse)
    @generation += 1

    
    if ( cse ) && (!cse.id )
       if (@last_event_time)
          # Synchronize event in order to trigger advancing last_event_time
          if ( @last_event_time.to_time() < cse.event_time.to_time() )
             @last_event_time = cse.event_time
             @log.debug("CSE last event time updated to #{@last_event_time}") if @log
          end
       end                 
       return
    else
       @last_event_time = cse.event_time
    end

    @log.debug("CSE #{cse.event_type}") if @log

    call_id = cse.call_id
    
    # ignore if we already notified world about this cdr
    return if @retired_calls[call_id]
    flush_retired(100) if 0 == @generation % 200
    
    # check if this is a message for call that we suspect might be failed
    failed_cdr = @failed_calls.delete(call_id)
    
    cdr = if failed_cdr
      @cdrs[call_id] = failed_cdr.cdr
    else    
      @cdrs[call_id] ||= @cdr_class.new(call_id, @log)
    end      
    
    if cdr.accept(cse)
      @cdrs.delete(call_id)
      if cdr.terminated?
        notify(cdr)
      else
        # delay notification for failed CDRs - they will get another chance
        @failed_calls[call_id] = FailedCdr.new(cdr, @generation)        
      end
    else
      if cse.reference && cse.reference.include?("rel=chain")
         # extract the call-id related to the chain.  If we don't know about the call
         # remove the whole chain reference as we do not want CDR to filter it as a chained call.
         cdrrefs = cdr.reference.split(",") 
         cdrrefs.each { |ref| 
            #Check each reference as there may be multiple ones.
            if ref.include?("rel=chain") 
               reftokens = ref.split(";")
               if @cdrs[reftokens[0]].nil? 
                  # We're not tracking the referenced call.  Remove the chain reference.
                  cdrrefs.delete(ref)
               end
            end
         }
         cdr.reference = cdrrefs.join(",")
      end
    end    
  end
  
  # Filter out calls that are simply chains of another call (i.e. via B2BUA).
  # The detection of chained calls is done by checking for a reference which indicates chaining.
  def filter_cdr_chained(cdr)
     if cdr.reference && cdr.reference.include?("rel=chain")
        return true
     end
     return false
  end

  # Some special user calls (i.e. to music on hold) we don't want to report on. If the cdr
  # involves one of these special identities then return that it should be ignored/filtered.
  def filter_cdr_user(cdr)
     to_id = cdr.callee_aor
     user = Utils.contact_user(to_id)
     filter_user = false
     @filtered_identities.each { |f_user| 
        if !user.nil? && user.include?(f_user) 
           filter_user = true 
        end
     }
     return filter_user
  end
  # Strictly speaking this function does not have to be called.
  # Since it is possible that we receive notifications after we already 
  # notified about the CDR we need to maintain a list of CD for which we are going to ignore all notifications.
  # If it's never flushed we are still OK but since every CDR has to be checked against this list the performance will suffer.
  def flush_retired(age)
    @retired_calls.delete_if do | key, value | 
      @generation - value >= age
    end      
  end
  
  # Call to notify listeners about calls that failed. 
  # We cannot really determine easily if the call had failed, since we do not not if we already process all the call legs.
  # That's why we store potential failures for a while and only notify observers when we done.
  def flush_failed(age = 0)
    @failed_calls.delete_if do |key, value|
      notify(value.cdr) if @generation - value.generation >= age
    end
  end
  
  # call to retire established (turn into CDRs) really long calls
  # the assumption is that we may be losing some events for some calls which permanently keeps them in 'ongoing' state
  def retire_long_calls(max_call_len)
    return unless @last_event_time
    return unless max_call_len > 0
    @cdrs.delete_if do | call_id, cdr |
      start_time = cdr.start_time
      if start_time && (@last_event_time.to_time() - start_time.to_time() > max_call_len)
        cdr.force_finish
        notify(cdr)
        true                
      end
    end    
  end
  
  # call to retire long ringing (turn into CDRs) calls
  # the assumption is that we may have lost an event for some call which permanently keeps them in 'requested' state
  def retire_long_ringing_calls(max_ringing_call_len)
    return unless @last_event_time
    return unless max_ringing_call_len > 0
    @cdrs.delete_if do | call_id, cdr |
      start_time = cdr.start_time
      if start_time && (@last_event_time.to_time() - start_time.to_time() > max_ringing_call_len) && (cdr.termination == Cdr::CALL_REQUESTED_TERM)
        cdr.force_failed_finish
        notify(cdr)
        true                
      end
    end    
  end
  
  # declare calls as failed if wait time elapses since the call start and no new event arrived
  def flush_failed_calls(max_wait_time)
    return unless @last_event_time
    return unless max_wait_time > 0
    @failed_calls.delete_if do |key, value|
      cdr = value.cdr
      start_time = cdr.start_time
      notify(cdr) if start_time && (@last_event_time.to_time() - start_time.to_time() > max_wait_time)
    end
  end

  private
  
  # if what we find in the queue is an array we assume 
  # it's been sent by one of the housekeeping threads to flush failed CDRs or clean up retired ones
  def housekeeping(method, *params)
    send(method, *params)
  end  
  
  # Add ready CDR to CDR queue, save it in retired_calls collection so that we can ignore new events for this CDR
  def notify(cdr)
    cdr.retire
    @retired_calls[cdr.call_id] = @generation
    @cdr_queue << cdr  if !filter_cdr_user(cdr) && !filter_cdr_chained(cdr)
  end
end
