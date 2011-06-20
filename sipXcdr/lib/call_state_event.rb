#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

# application requires
require 'utils/utils'

class CallStateEvent

  FIELDS = [:id, :observer, :event_seq, :event_time, :event_type, :cseq, :call_id, :from_tag, :to_tag,
    :from_url, :to_url, :contact, :refer_to, :referred_by, :failure_status, :failure_reason, :request_uri, :reference, :caller_internal, :callee_route, :branch_id, :via_count]
  
  attr_accessor(*FIELDS)
  
  # Constants representing event types
  CALL_REQUEST_TYPE =  'R'
  CALL_SETUP_TYPE =    'S'
  CALL_END_TYPE =      'E'
  CALL_TRANSFER_TYPE = 'T'
  CALL_FAILURE_TYPE =  'F'
  
  def to_s
    # Map the event type code to a human-readable string
    event_type_name = nil
    if @event_type
      event_type_name = EVENT_TYPE_NAMES[@event_type]
      if !event_type_name
        event_type_name = "unknown CSE type '#{@event_type}'"
      end
    end
    
    # Show '(nil)' for empty values if data is missing
    missing = '(nil)'
    event_type_name ||= missing
    from_url = @from_url || missing
    to_url = @to_url || missing
    call_id = @call_id || missing
    cseq = @cseq || missing
    
    # Create and return a human-readable description of the event
    "#<#{event_type_name} from #{from_url} to #{to_url} call_id=#{call_id} cseq=#{cseq}>"    
  end
  
  # Return the AOR part of the from_url, or nil if there is no from_url.
  # Raise BadSipHeaderException if the tag is missing from the from_url.
  def caller_aor
    Utils.get_aor_from_header(@from_url) if @from_url
  end
  
  # Return the AOR part of the to_url, or nil if there is no to_url.
  # Raise BadSipHeaderException if the tag is missing from the to_url when
  # it should be there.  All events except call request should have the to tag.
  def callee_aor
    Utils.get_aor_from_header(@to_url, !call_request?)  if @to_url
  end
  
  def caller_contact
    Utils.contact_without_params(@contact)    
  end
  
  # Convenience methods for checking the event type
  def call_request?
    @event_type == CALL_REQUEST_TYPE
  end
  def call_setup?
    @event_type == CALL_SETUP_TYPE
  end
  def call_end?
    @event_type == CALL_END_TYPE
  end
  def call_transfer?
    @event_type == CALL_TRANSFER_TYPE
  end
  def call_failure?
    @event_type == CALL_FAILURE_TYPE
  end
  
  private
  
  # Map event type codes to human-readable strings
  EVENT_TYPE_NAMES = { CALL_REQUEST_TYPE =>  'call_request',
    CALL_SETUP_TYPE =>    'call_setup',
    CALL_END_TYPE =>      'call_end',
    CALL_TRANSFER_TYPE => 'call_transfer',
    CALL_FAILURE_TYPE =>  'call_failure' }
  
end
