#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'test/unit'

$:.unshift(File.join(File.dirname(__FILE__), '..', 'lib'))

require 'call_state_event'
require 'utils/exceptions'

class CallStateEventTest < Test::Unit::TestCase
  
  def test_caller_aor
    cse = CallStateEvent.new
    cse.from_url = 'sip:alice@example.com'
    assert_raise(BadSipHeaderException) { cse.caller_aor }
    cse.from_url = 'sip:alice@example.com;tag=f'
    assert_equal('sip:alice@example.com', cse.caller_aor)
  end

  def test_callee_aor
    cse = CallStateEvent.new
    cse.event_type = CallStateEvent::CALL_SETUP_TYPE
    cse.to_url = 'sip:bob@example.com'
    assert_raise(BadSipHeaderException) { cse.callee_aor }
    cse.to_url = 'sip:bob@example.com;tag=t'
    assert_equal('sip:bob@example.com', cse.callee_aor)

    cse.event_type = CallStateEvent::CALL_REQUEST_TYPE
    cse.to_url = 'sip:bob@example.com'
    assert_equal('sip:bob@example.com', cse.callee_aor)
    cse.to_url = 'sip:bob@example.com;tag=t'
    assert_equal('sip:bob@example.com', cse.callee_aor)
  end
  
end
