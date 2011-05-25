#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'test/unit'

$:.unshift(File.join(File.dirname(__FILE__), '..', '..', 'lib'))

require 'db/database_url'
require 'db/cse_reader'

class CseReaderTest < Test::Unit::TestCase
  def test_select_sql
    sql = CseReader.select_sql
    assert_equal("SELECT id, observer, event_seq, event_time, event_type, cseq, " +
      "call_id, from_tag, to_tag, " +
      "from_url, to_url, contact, refer_to, referred_by, failure_status, " +
      "failure_reason, request_uri, reference, caller_internal, callee_route, branch_id, via_count FROM call_state_events " +
      "ORDER BY event_time LIMIT 1500", sql)
    
    sql = CseReader.select_sql(nil, Time.now, nil)
    assert_equal("SELECT id, observer, event_seq, event_time, event_type, cseq, " +
      "call_id, from_tag, to_tag, " +
      "from_url, to_url, contact, refer_to, referred_by, failure_status, " +
      "failure_reason, request_uri, reference, caller_internal, callee_route, branch_id, via_count FROM call_state_events " +
      "WHERE event_time >= ? " +
      "ORDER BY event_time LIMIT 1500", sql)    
    
    sql = CseReader.select_sql(nil, nil, Time.now)
    assert_equal("SELECT id, observer, event_seq, event_time, event_type, cseq, " +
      "call_id, from_tag, to_tag, " +
      "from_url, to_url, contact, refer_to, referred_by, failure_status, " +
      "failure_reason, request_uri, reference, caller_internal, callee_route, branch_id, via_count FROM call_state_events " +
      "WHERE event_time <= ? " +
      "ORDER BY event_time LIMIT 1500", sql)    
    
    sql = CseReader.select_sql(nil, Time.new, Time.new)
    assert_equal("SELECT id, observer, event_seq, event_time, event_type, cseq, " +
      "call_id, from_tag, to_tag, " +
      "from_url, to_url, contact, refer_to, referred_by, failure_status, " +
      "failure_reason, request_uri, reference, caller_internal, callee_route, branch_id, via_count FROM call_state_events " + 
      "WHERE event_time >= ? AND event_time <= ? " +
      "ORDER BY event_time LIMIT 1500", sql)
  end

  def test_select_sql_with_start_id
    sql = CseReader.select_sql(1000)
    assert_equal("SELECT id, observer, event_seq, event_time, event_type, cseq, " +
      "call_id, from_tag, to_tag, " +
      "from_url, to_url, contact, refer_to, referred_by, failure_status, " +
      "failure_reason, request_uri, reference, caller_internal, callee_route, branch_id, via_count FROM call_state_events " +
      "WHERE id > ? " +
      "ORDER BY event_time LIMIT 1500", sql)
    
    sql = CseReader.select_sql(1000, Time.now, nil)
    assert_equal("SELECT id, observer, event_seq, event_time, event_type, cseq, " +
      "call_id, from_tag, to_tag, " +
      "from_url, to_url, contact, refer_to, referred_by, failure_status, " +
      "failure_reason, request_uri, reference, caller_internal, callee_route, branch_id, via_count FROM call_state_events " +
      "WHERE id > ? AND event_time >= ? " +
      "ORDER BY event_time LIMIT 1500", sql)    
    
    sql = CseReader.select_sql(1000, nil, Time.now)
    assert_equal("SELECT id, observer, event_seq, event_time, event_type, cseq, " +
      "call_id, from_tag, to_tag, " +
      "from_url, to_url, contact, refer_to, referred_by, failure_status, " +
      "failure_reason, request_uri, reference, caller_internal, callee_route, branch_id, via_count FROM call_state_events " +
      "WHERE id > ? AND event_time <= ? " +
      "ORDER BY event_time LIMIT 1500", sql)    
    
    sql = CseReader.select_sql(1000, Time.new, Time.new)
    assert_equal("SELECT id, observer, event_seq, event_time, event_type, cseq, " +
      "call_id, from_tag, to_tag, " +
      "from_url, to_url, contact, refer_to, referred_by, failure_status, " +
      "failure_reason, request_uri, reference, caller_internal, callee_route, branch_id, via_count FROM call_state_events " + 
      "WHERE id > ? AND event_time >= ? AND event_time <= ? " +
      "ORDER BY event_time LIMIT 1500", sql)  
  end

  
  def test_delete_sql
    sql = CseReader.delete_sql(Time.new, Time.new)
    assert_equal("DELETE FROM call_state_events " + 
      "WHERE event_time >= ? AND event_time <= ?", sql)    
  end
  
end
