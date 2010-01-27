#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'utils/utils'

class CallLeg
  attr_reader :id, :connect_time, :end_time, :status, :to_tag, :failure_status, :failure_reason, :callee_contact, :reference
  attr_writer :log
  
  def initialize(id,log=nil)
    @id = id
    @status = nil
    @connect_time = nil
    @end_time = nil
    @failure_status = nil
    @failure_reason = nil
    @reference = nil
    # we count 'setup' and 'end/failure' events but the numbers do not have to match here
    @refcount = 0    
    @log = log
  end
  
  def has_duration?
    @connect_time && @end_time
  end
  
  def duration
    raise ArgumentError, "only defined for completed" unless has_duration?
    @end_time - @connect_time
  end
  
  def accept_setup(cse)
    @refcount += 1
    if @connect_time.nil? || cse.event_time < @connect_time
      @connect_time = cse.event_time
    end      
    @status ||= Cdr::CALL_IN_PROGRESS_TERM
    @to_tag = cse.to_tag
    @callee_contact = Utils.contact_without_params(cse.contact)
    @callee_route = cse.callee_route
  end
  
  def accept_end(cse)
    @refcount -= 1

    if @end_time.nil? || @end_time < cse.event_time
      @end_time = cse.event_time
    end
    @status = if cse.call_end? 
      Cdr::CALL_COMPLETED_TERM 
    else 
      @failure_reason = cse.failure_reason
      @failure_status = cse.failure_status
      if @failure_status == Cdr::SIP_REQUEST_CANCELLED
         Cdr::CALL_ABANDONED_TERM 
      else
         Cdr::CALL_FAILED_TERM 
      end
    end
    @to_tag ||= cse.to_tag
  end  
  
  include Comparable
  def <=>(other)
    # leg that has duration is always better than the one that does now
    return 1 if has_duration? && !other.has_duration?
    return -1 if !has_duration? && other.has_duration?
    
    # if both have duration longer one should win
    if has_duration? and other.has_duration? 
      if @status == other.status 
        return duration <=> other.duration
      else
        return compare_leg_status(@status, other.status)
      end
    end
    
    # if we are here none has duration
    if @status == other.status
      # the same status
      return other.connect_time <=> @connect_time if @status == Cdr::CALL_IN_PROGRESS_TERM
      return @end_time <=> other.end_time
    else
      return compare_leg_status(@status, other.status)
    end
  end
  
  # compare leg status - completed >> in_progress >> failed
  def compare_leg_status(first, second)
    return 0 if first == second
    return -1 if Cdr::CALL_FAILED_TERM == first
    return 1 if Cdr::CALL_COMPLETED_TERM == first
    return 1 if  Cdr::CALL_FAILED_TERM == second
    return -1 if  Cdr::CALL_COMPLETED_TERM == second
  end
  
  class << self
    # Compute call leg ID from 'from' and 'to' tags
    def leg_id(cse)
      from = cse.from_tag
      to = cse.to_tag
      if from < to
        "#{from}<>#{to}"
      else
        "#{to}<>#{from}"
      end        
    end    
  end
end


class CallLegs
  attr_reader :best_leg
  attr_writer :log
  
  def initialize(log=nil)
    @legs = {}
    @best_leg = nil
    @log = log
  end
  
  def accept_setup(cse)
    leg = get_leg(cse)
    leg.accept_setup(cse)
    check_best_leg(leg)
  end
  
  def accept_end(cse)
    leg = get_leg(cse)
    leg.accept_end(cse)
    check_best_leg(leg)
  end
  
  def check_best_leg(leg)
    if !@best_leg || @best_leg < leg
      @best_leg = leg                
    end
    return @best_leg
  end
  
  def done?
    @best_leg && @best_leg.status != Cdr::CALL_IN_PROGRESS_TERM
  end
  
  def established?
    @best_leg && @best_leg.status == Cdr::CALL_IN_PROGRESS_TERM
  end
  
  private
  def get_leg(cse)
    id = CallLeg.leg_id(cse)
    @legs[id] ||= CallLeg.new(id, @log)
  end
end

#
#  CDR description
#  id                                 primary key
#  call_id text not null              SIP call ID from the INVITE 
#  from_tag text not null             SIP from tag provided by caller in the INVITE 
#  to_tag text not null               SIP to tag provided by callee in the 200 OK 
#  caller_aor text not null           caller's SIP address of record (AOR) 
#  callee_aor text not null           callee's AOR 
#  start_time timestamp               Start time in GMT: initial INVITE received 
#  connect_time timestamp             Connect time in GMT: ACK received for 200 OK 
#  end_time timestamp                 End time in GMT: BYE received  or other ending 
#  termination char(1)                Why the call was terminated 
#  failure_status int2                SIP error code if the call failed  e.g.  4xx 
#  failure_reason text                Text describing the reason for a call failure 
#  call_direction char(1)             Plugin feature  see below 
class Cdr
  attr_writer :log
  # Constants representing termination codes
  CALL_REQUESTED_TERM   = 'R'
  CALL_IN_PROGRESS_TERM = 'I'
  CALL_COMPLETED_TERM   = 'C'
  CALL_TRANSFERRED_TERM = 'T'
  CALL_UNKNOWN_COMPLETED_TERM = 'U'
  CALL_FAILED_TERM      = 'F'
  CALL_ABANDONED_TERM   = 'A'
  
  SIP_UNAUTHORIZED_CODE = 401
  SIP_PROXY_AUTH_REQUIRED_CODE = 407
  SIP_REQUEST_TIMEOUT_CODE = 408
  SIP_BAD_TRANSACTION_CODE = 481
  SIP_REQUEST_CANCELLED = 487

  SIP_UNAUTHORIZED_STR = 'Unauthorized'
  SIP_PROXY_AUTH_REQUIRED_STR = 'Proxy Authentication Required'
  SIP_REQUEST_TIMEOUT_STR = 'Request Timeout'
  SIP_BAD_TRANSACTION_STR = 'Call Leg/Transaction Does Not Exist'
  SIP_REQUEST_CANCELLED_STR = 'Call Request Abandoned'

  def initialize(call_id, log=nil)
    @call_id = call_id
    @from_tag = nil
    @to_tag = nil
    @start_time = nil
    @connect_time = nil
    @end_time = nil    
    @termination = nil
    @pendingtermination = nil
    @failure_status = nil
    @failure_reason = nil
    @call_direction = nil
    
    @got_original = false
    @log = log
    @legs = CallLegs.new(@log)
    
    @callee_contact = nil
    @caller_contact = nil
    @caller_internal = false
    @callee_route = nil
  end
  
  FIELDS = [:call_id, :from_tag, :to_tag, :caller_aor, :callee_aor, 
  :start_time, :connect_time, :end_time,    
  :termination, :failure_status, :failure_reason, 
  :call_direction, :reference, :caller_contact, :callee_contact, :caller_internal, :callee_route]
  
  attr_accessor(*FIELDS)
  
  
# Return true if the CDR is complete, false otherwise.
  def complete?
    @termination == CALL_COMPLETED_TERM || @termination == CALL_FAILED_TERM || @termination == CALL_UNKNOWN_COMPLETED_TERM || @termination == CALL_ABANDONED_TERM
  end
  
  def terminated?
    @termination == CALL_COMPLETED_TERM || @termination == CALL_UNKNOWN_COMPLETED_TERM || @termination == CALL_TRANSFERRED_TERM || @termination == CALL_ABANDONED_TERM
  end
  
  # Return a text description of the termination status for this CDR.
  def termination_text
    TERMINATION_NAMES[@termination] if @termination
  end  
  
  def accept(cse)
    case
    when cse.call_request?
      accept_call_request(cse)
    when cse.call_setup?
      accept_call_setup(cse)
    when cse.call_failure?
      if @legs.established? then
         # established calls that receive a failure of bad transaction or request timeout are considered failed.
         accept_call_end(cse)  unless ((cse.failure_status != SIP_BAD_TRANSACTION_CODE) && (cse.failure_status != SIP_REQUEST_TIMEOUT_CODE))
      else
         # non-established calls only consider a failure if the reason is not a timeout, auth required or unauthorized.
         accept_call_end(cse)  unless ((cse.failure_status == SIP_REQUEST_TIMEOUT_CODE) || (cse.failure_status == SIP_PROXY_AUTH_REQUIRED_CODE) || (cse.failure_status == SIP_UNAUTHORIZED_CODE))
      end
    when cse.call_transfer?
      accept_call_transfer(cse)
    when cse.call_end?
      accept_call_end(cse)
    end
  end
  
  # called when we are done with processing of this CDR
  # cleans temporary data structures
  def retire
    @legs = nil
  end
  
  # called if we suspect termination event has been lost
  def force_finish
    leg = @legs.best_leg
    @to_tag ||= 1;
    apply_leg(leg) if leg
    @termination = CALL_UNKNOWN_COMPLETED_TERM
  end
  
  # called if we suspect a failed termination event has been lost
  def force_failed_finish
    leg = @legs.best_leg
    apply_leg(leg) if leg
    @to_tag ||= 1;
    @failure_status = SIP_REQUEST_TIMEOUT_CODE
    @failure_reason = SIP_REQUEST_TIMEOUT_STR
    @termination = CALL_FAILED_TERM

  end
  
  def to_s
    "CDR: #{@call_id}, from #{@caller_aor} to #{@callee_aor} status #{@termination}"
  end
  
  private
  
  # original (without to_tag) request is always better than not original (with to_tag) request
  def accept_call_request(cse)
    original = !cse.to_tag
    @caller_internal = cse.caller_internal 
    # bailout if we already have original request and this one is not
    return if(@got_original && !original)
    
    # continue if we are original or if we are older 
    if((!@got_original && original) || !@start_time || @start_time > cse.event_time)
      
      @from_tag = cse.from_tag
      @caller_aor = cse.caller_aor
      @callee_aor = cse.callee_aor
      @start_time = cse.event_time      
      @reference = cse.reference
      @termination = CALL_REQUESTED_TERM unless @termination
      
      @caller_contact = Utils.contact_without_params(cse.contact)
      
      @got_original ||= !cse.to_tag      
    end
    return nil
  end
  
  def accept_call_setup(cse)
    if !@start_time
        # probably a case where we've missed the request.  Setup necessary
        # info as if a request was seen.
        @from_tag = cse.from_tag
        @caller_aor = cse.caller_aor
        @callee_aor = cse.callee_aor
        @start_time = cse.event_time     
        @reference = cse.reference
        @caller_internal = cse.caller_internal 
        @caller_contact = Utils.contact_without_params(cse.contact)
    end
    @legs.accept_setup(cse)
    @callee_route = cse.callee_route if !@callee_route
    @callee_contact = cse.contact if !@callee_contact
    @termination = CALL_IN_PROGRESS_TERM 
    finish
  end
  
  def accept_call_transfer(cse)
    @pendingtermination = CALL_TRANSFERRED_TERM
  end
  
  def accept_call_end(cse)
    @legs.accept_end(cse)
    finish
  end
  
  def finish
    return unless @legs.done?
    apply_leg(@legs.best_leg)
    return self
  end
  
  def apply_leg(leg)
    @to_tag = leg.to_tag
    @connect_time = leg.connect_time
    @end_time = leg.end_time
    @termination = leg.status
    if @pendingtermination and leg.status == CALL_COMPLETED_TERM
       @termination = @pendingtermination
    else
       @termination = leg.status
    end
    @failure_reason = leg.failure_reason
    @failure_status = leg.failure_status
    @callee_contact = leg.callee_contact  
  end
  
  # Map termination codes to human-readable strings
  TERMINATION_NAMES = { 
    CALL_REQUESTED_TERM   => 'requested',
    CALL_IN_PROGRESS_TERM => 'in progress',
    CALL_COMPLETED_TERM   => 'completed',
    CALL_TRANSFERRED_TERM   => 'transferred',
    CALL_UNKNOWN_COMPLETED_TERM   => 'unknown',
    CALL_FAILED_TERM      => 'failed'}  
end
