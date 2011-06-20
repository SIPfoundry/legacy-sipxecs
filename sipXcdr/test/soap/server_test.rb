#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'test/unit'
require 'thread'
require 'soap/rpc/driver'
require 'dbi'

$:.unshift(File.join(File.dirname(__FILE__), '..', '..', 'lib'))

require 'cdr'
require 'soap/server'

class SoapServerTest < Test::Unit::TestCase
  
  Config = Struct.new(:agent_address, :agent_port, :log)
  
  State = Struct.new(:active_cdrs)
  
  def start_server(state)
    @server = CdrResolver::SOAP::Server.new(state, Config.new('0.0.0.0', 2001, nil))
    Thread.new(@server) { |s|  s.start }
  end
  
  def setup
    @service = SOAP::RPC::Driver.new('http://localhost:2001/', 'urn:CdrService')
    @service.wiredump_dev = STDERR if $DEBUG
    @service.add_method('getActiveCalls')
  end
  
  def teardown
    @server.shutdown if @server
  end  
  
  def testGetActiveCallsEmpty
    cdrs = []
    start_server(State.new(cdrs))
    calls = @service.getActiveCalls
    assert calls.empty?
  end
  
  def testGetActiveCallsOne
    cdr1 = Cdr.new('test')
    cdr1.start_time = DBI::Timestamp.new(Time.now)
    cdr1.caller_aor = 'from@example.org'
    cdr1.callee_aor = 'to@example.org'
    cdr1.termination = Cdr::CALL_IN_PROGRESS_TERM
    
    cdrs = [cdr1]    
    start_server(State.new(cdrs))
    calls = @service.getActiveCalls
    assert_equal(1,  calls.size)
    call = calls[0]
    assert(call.duration >= 0)
    assert_equal(cdr1.caller_aor, call.from)
    assert_equal(cdr1.callee_aor, call.to)
  end
  
  def testGetActiveCallsMany
    cdrs = []
     (0..4).each { |i|
      cdr = Cdr.new("test#{i}")
      cdr.start_time = DBI::Timestamp.new(Time.now)
      cdr.caller_aor = "from#{i}@example.org"
      cdr.callee_aor = "to#{i}@example.org"
      cdr.termination = Cdr::CALL_IN_PROGRESS_TERM
      cdrs << cdr
    }
    start_server(State.new(cdrs))
    calls = @service.getActiveCalls
    assert_equal(5, calls.size)
    calls.each_with_index { |call, i|
      assert(call.duration >= 0)
      assert_equal(cdrs[i].caller_aor, call.from)
      assert_equal(cdrs[i].callee_aor, call.to)
    } 
  end  
end
