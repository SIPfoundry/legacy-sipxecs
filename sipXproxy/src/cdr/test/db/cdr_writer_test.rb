#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'test/unit'
require 'thread'

$:.unshift(File.join(File.dirname(__FILE__), '..', '..', 'lib'))

require 'db/cdr_writer'

class CdrWriterTest < Test::Unit::TestCase
  
  def test_insert_sql
    sql = CdrWriter.insert_sql
    assert_equal("INSERT INTO cdrs ( call_id, from_tag, to_tag, caller_aor, callee_aor, " +
      "start_time, connect_time, end_time, " +
      "termination, failure_status, failure_reason, " +
      "call_direction, reference, caller_contact, callee_contact, caller_internal, callee_route ) " +
      "VALUES ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )", sql)
  end
end
