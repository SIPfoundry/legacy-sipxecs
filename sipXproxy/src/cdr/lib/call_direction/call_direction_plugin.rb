#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################


require 'utils/configure'
require 'utils/exceptions'
require 'call_direction/gateway'

# CallDirectionPlugin computes call direction for a CDR.
class CallDirectionPlugin
  attr_reader :log
  
  public
  
  # Configuration parameters
  CALL_DIRECTION = 'SIP_CALLRESOLVER_CALL_DIRECTION'
  CALL_DIRECTION_DEFAULT = Configure::DISABLE
  
  # Call direction: single-char codes stored in the call_direction column.
  INCOMING = 'I'        # calls that come in from a PSTN gateway
  OUTGOING = 'O'        # calls that go out to a PSTN gateway
  INTRANETWORK = 'A'    # calls that are pure SIP and don't go through a gateway
  
  # cdrin - input queue for CDRs
  # cdrout - output queue for CDRs
  def initialize(cdrin, cdrout, gateway_addresses, log)
    @cdrin = cdrin
    @cdrout = cdrout
    @log = log
    
    @gateway_addresses = gateway_addresses
  end
  
  # process CDR queue setting call direction for each CDR
  def run
    while cdr = @cdrin.deq
      set_call_direction(cdr)
      @cdrout << cdr
    end
    @cdrout << nil
  end
  
  # Compute and set call direction for the CDR.
  def set_call_direction(cdr)
    # Compute the call direction, based on whether the from or to contact
    # is a gateway address. At most one of them can be a gateway address.
    call_direction = case 
    when gateway_address?(cdr.caller_contact): INCOMING
    when gateway_address?(cdr.callee_contact): OUTGOING
    else INTRANETWORK
    end
    log.debug("CallDirectionPlugin#update: CDR has call ID = #{cdr.call_id}, " +
              "caller_contact = #{cdr.caller_contact}, " +
              "callee_contact = #{cdr.callee_contact}, " +
              "call_direction = #{call_direction}")
    
    # Update the CDR's call_direction
    cdr.call_direction = call_direction
  end

  private
    
  def gateway_address?(contact)
    return unless contact
    contact_addr = Utils.contact_host(contact)
    @gateway_addresses.any? {|g| g == contact_addr}
  end
  
  class << self    
    # Return true if call direction is enabled, false otherwise, based on the config.
    def call_direction?(config)
      config.enabled?(CALL_DIRECTION, CALL_DIRECTION_DEFAULT)
    end    
  end
    
end
