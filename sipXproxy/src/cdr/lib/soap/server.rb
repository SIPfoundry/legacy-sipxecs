#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'soap/rpc/standaloneServer'
require 'soap/mapping'

module CdrResolver
  
  module SOAP
    
    SERVICE_NAMESPACE = 'urn:CdrService'
    
    class ActiveCall; include ::SOAP::Marshallable
      @@schema_ns = SERVICE_NAMESPACE
      @@schema_type = 'ActiveCall'
      
      attr_accessor :from, :to, :recipient, :duration, :start_time
      
      def initialize(cdr, now)
        @from = cdr.caller_aor
        @to = cdr.callee_aor
        @recipient = cdr.callee_contact
        @start_time = cdr.start_time.to_time
        # convert time difference to milliseconds
        @duration = ((now - @start_time) * 1000).ceil
      end    
    end
    
    # marshall-able version of the standard Array
    class Array < ::Array; include ::SOAP::Marshallable
      @@schema_ns = SERVICE_NAMESPACE  
    end
    
    class CdrService      
      def initialize(state, log = nil)
        @state = state
        @log = log
      end
          
      def getActiveCalls
        active_calls = Array.new
        now = Time.now
        @state.active_cdrs.each do | cdr |
            active_calls << ActiveCall.new(cdr, now) if cdr.termination == Cdr::CALL_IN_PROGRESS_TERM && cdr.start_time
        end
        @log.debug("getActiveCalls #{active_calls.size}") if @log
        return active_calls
      end      
    end
    
    class Server < ::SOAP::RPC::StandaloneServer
      def initialize(state, config)
        @cdrService = CdrService.new(state, config.log)
        
        super('sipXproxyCdr', SERVICE_NAMESPACE, config.agent_address, config.agent_port)
      end
      
      def on_init
        #@log.level = Logger::Severity::DEBUG
        add_method(@cdrService, 'getActiveCalls')
      end  
    end
    
  end
  
end
