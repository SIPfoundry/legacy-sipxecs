#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

$:.unshift(File.join(File.dirname(__FILE__), '..', '..', 'lib'))
require 'call_direction/call_direction_plugin'

require 'utils/configure'
require 'cdr'

$:.unshift File.join(File.dirname(__FILE__), '..', '..', 'test')
require 'test_util'
include TestUtil

class CallDirectionPluginTest < Test::Unit::TestCase 
  def setup
    # Create the CallResolver, giving it the location of the test config file.
    @plugin = CallDirectionPlugin.new(nil, nil, ["1.1.1.1"], NullLog.new)
  end

  def test_set_call_direction_config
    # disabled by default
    config = Configure.new
    assert(!CallDirectionPlugin.call_direction?(config))    
  end
  
  def test_call_direction
    # Set up a single test gateway

    cdr = Cdr.new("123")
    cdr.caller_contact = "sip:alice@example.com"
    cdr.callee_contact = "sip:bob@example.com"
    @plugin.set_call_direction(cdr)    
    assert_equal(CallDirectionPlugin::INTRANETWORK, cdr.call_direction)
  end
  
  def test_incoming
    # Set up a single test gateway

    cdr = Cdr.new("123")
    cdr.caller_contact = "sip:alice@1.1.1.1"
    cdr.callee_contact = "sip:bob@example.com"
    @plugin.set_call_direction(cdr)    
    assert_equal(CallDirectionPlugin::INCOMING, cdr.call_direction)
  end

  def test_call_outgoing
    # Set up a single test gateway

    cdr = Cdr.new("123")
    cdr.caller_contact = "sip:alice@example.com"
    cdr.callee_contact = "sip:bob@1.1.1.1"
    @plugin.set_call_direction(cdr)    
    assert_equal(CallDirectionPlugin::OUTGOING, cdr.call_direction)
  end
  
  def test_call_direction_with_null_contact
    cdr = Cdr.new("123")
    cdr.caller_contact = "sip:alice@example.com"
    cdr.callee_contact = nil
    @plugin.set_call_direction(cdr)    
    assert_equal(CallDirectionPlugin::INTRANETWORK, cdr.call_direction)
  end
  
  # FXIME: not sure if this is the right test - compare with 3.6 test
  def test_domain_name_contact
    @plugin = CallDirectionPlugin.new(nil, nil, ["example.com"], NullLog.new)
    cdr = Cdr.new("123")
    cdr.caller_contact = "sip:alice@example.com"
    cdr.callee_contact = "sip:bob@1.1.1.1"
    @plugin.set_call_direction(cdr)    
    assert_equal(CallDirectionPlugin::INCOMING, cdr.call_direction)
  end          
    
end
