#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

$SOURCE_DIR = File.dirname(__FILE__)    # directory in which this file is located

# system requirements
require 'parsedate'
require File.join($SOURCE_DIR, '..', 'test_helper')

# application requirements
require File.join($SOURCE_DIR, '..', '..', 'call_resolver')


# :TODO: Make it easy to run all the unit tests, possibly via Rakefile, for build loop.
class CallResolverTest < Test::Unit::TestCase
  fixtures :call_state_events, :cdrs
  
  TEST_AOR = 'aor'
  TEST_CONTACT = 'contact'
  TEST_CALL_ID = 'call ID'
  TEST_FROM_TAG = 'f'
  TEST_TO_TAG = 't'
  
  CALL_ID1 = 'call_id1'
  CALL_ID2 = 'call_id2'
  CALL_ID3 = 'call_id3'
  
  TEST_DB1 = 'SIPXCSE_TEST1'
  TEST_DB2 = 'SIPXCSE_TEST2'
  
  LOCALHOST = 'localhost'
  
  public
  
  def setup
    super
    
    # Create the CallResolver, giving it the location of the test config file.
    @resolver = CallResolver.new(File.join($SOURCE_DIR, 'data/callresolver-config'))
  end
  
  # Create a test CSE.  Fill in dummy values for fields we don't care about but
  # have to be filled in because of not-null DB constraints
  def create_test_cse(call_id, event_time)
    CallStateEvent.create(:observer => 'observer',
    :event_seq => 0,
    :event_time => event_time,
    :event_type => CallStateEvent::CALL_REQUEST_TYPE,
    :cseq => 0,
    :call_id => call_id,
    :from_url => 'from_url',
    :to_url => 'to_url')    
  end
  
  def test_merge_events_for_call    
    e1 = CallStateEvent.new(:call_id => CALL_ID1)
    e2 = CallStateEvent.new(:call_id => CALL_ID1)
    
    time = Time.now;
    e3 = CallStateEvent.new(:call_id => CALL_ID2, :event_time => time)
    e4 = CallStateEvent.new(:call_id => CALL_ID2, :event_time => time + 1)
    e5 = CallStateEvent.new(:call_id => CALL_ID2, :event_time => time + 2)
    e6 = CallStateEvent.new(:call_id => CALL_ID2, :event_time => time + 3)
    
    e7 = CallStateEvent.new(:call_id => CALL_ID3)
    
    call1 = [e1, e2]
    call2_part1 = [e3, e5]
    call2_part2 = [e4, e6]
    call2 = [e3, e4, e5, e6]
    call3 = [e7]
    all_calls = [[call1, call2_part1],           # call arrays for first DB
    [call2_part2, call3]]           # call arrays for second DB
    call_map = {}
    @resolver.send(:merge_events_for_call, all_calls, call_map)
    expected_call_map_entries = [call1, call2, call3]
    check_call_map(all_calls, call_map, expected_call_map_entries)
  end
  
  def check_call_map(all_calls, call_map, expected_call_map_entries)
    assert_equal(expected_call_map_entries.size, call_map.size)
    [CALL_ID1, CALL_ID2, CALL_ID3].each_with_index do |call_id, i|
      assert_equal(expected_call_map_entries[i], call_map[call_id])
    end
  end
  
  def new_call_request(event_time, to_tag = nil)
    params = {:event_type => CallStateEvent::CALL_REQUEST_TYPE,
      :event_time => event_time}
    params[:to_tag] = to_tag if to_tag
    CallStateEvent.new(params)
  end
  
  def find_call_request(events)
    @resolver.send(:find_call_request, events)
  end
  
  def test_finish_cdr
    events = load_simple_success_events
    
    # fill in cdr_data with info from the events
    to_tag = 't'
    cdr = Cdr.new
    status = @resolver.send(:finish_cdr, cdr, events, to_tag)
    assert_equal(true, status)
    
    # Check that the CDR is filled in as expected.  It will only be partially
    # filled in because we are testing just one part of the process.
    assert_equal(to_tag, cdr.to_tag, 'Wrong to_tag')
    assert_equal(Time.parse('1990-05-17T19:31:00.000Z'), cdr.connect_time,
                            'Wrong connect_time')
    assert_equal(Time.parse('1990-05-17T19:40:00.000Z'), cdr.end_time,
                            'Wrong end_time')
    assert_equal('sip:bob@2.2.2.2', cdr.callee_contact, 'Wrong callee contact')
    assert_equal(Cdr::CALL_COMPLETED_TERM, cdr.termination, 'Wrong termination code')
    assert_nil(cdr.failure_status)
    assert_nil(cdr.failure_reason)
    
    # Test a failed call.  Check only that the failure info has been filled in
    # properly.  We've checked other info in the case above.
    # This set of events has call request, call setup, call failed.
    call_id = 'testFailed'
    events = @resolver.send(:load_events_with_call_id, call_id)
    check_failed_call(events, to_tag)
    
    # Try again without the call setup event.
    events.delete_if {|event| event.call_setup?}
    check_failed_call(events, to_tag)
  end
  
  def test_finish_cdr_callee_hangs_up
    events = load_simple_success_events_callee_hangs_up
    
    # fill in cdr_data with info from the events
    to_tag = 't'
    cdr = Cdr.new
    status = @resolver.send(:finish_cdr, cdr, events, to_tag)
    assert_equal(true, status)
    
    # Check that the CDR is filled in as expected.  It will only be partially
    # filled in because we are testing just one part of the process.
    assert_equal(to_tag, cdr.to_tag, 'Wrong to_tag')
    assert_equal(Time.parse('1990-05-17T19:41:00.000Z'), cdr.connect_time,
                            'Wrong connect_time')
    assert_equal(Time.parse('1990-05-17T19:50:00.000Z'), cdr.end_time,
                            'Wrong end_time')
    assert_equal('sip:bob@2.2.2.2', cdr.callee_contact, 'Wrong callee contact')
    assert_equal(Cdr::CALL_COMPLETED_TERM, cdr.termination, 'Wrong termination code')
    assert_nil(cdr.failure_status)
    assert_nil(cdr.failure_reason)
  end
  
  # Helper method for test_finish_cdr.  Check that failure info has been filled
  # in properly.
  def check_failed_call(events, to_tag)
    cdr = Cdr.new
    status = @resolver.send(:finish_cdr, cdr, events, to_tag)
    assert_equal(true, status, 'Finishing the CDR failed')
    assert_equal(Cdr::CALL_FAILED_TERM, cdr.termination, 'Wrong termination code')
    assert_equal(499, cdr.failure_status)
    assert_equal("You Can't Always Get What You Want", cdr.failure_reason) 
  end
  
  def test_resolve_call
    ['testSimpleSuccess', 'testComplicatedSuccess', 'testFailed'].each do |call_id|
      events = @resolver.send(:load_events_with_call_id, call_id)
      @resolver.send(:resolve_call, events)
      cdr = Cdr.find_by_call_id(call_id)
      assert_not_nil(cdr, 'CDR was not created')
    end
  end
  
  def test_resolve
    Cdr.delete_all
    start_time = Time.parse('1990-01-1T000:00:00.000Z')
    end_time = Time.parse('2000-12-31T00:00.000Z')
    @resolver.resolve(start_time, end_time)
    assert_equal(4, Cdr.count, 'Wrong number of CDRs')
  end
  
  def test_get_start_time_from_cdr
    Cdr.delete_all
    start_time = @resolver.get_start_end_times
    # Give it one second difference in case the timing is off a bit
    assert((Time.now - (start_time + Configure::SECONDS_PER_DAY)) <= 1)
    
    start_time = Time.parse('1990-01-1T000:00:00.000Z')
    end_time = Time.parse('2000-12-31T00:00.000Z')
    @resolver.resolve(start_time, end_time)
    assert_equal(4, Cdr.count, 'Wrong number of CDRs')
    
    start_time = @resolver.get_start_end_times
    assert_equal(start_time, Time.gm(2000,1,1,0,0,0))      
  end
  
  #-----------------------------------------------------------------------------
  # Helper methods
  
  # load and return events for the simple case
  def load_simple_success_events
    call_id = 'testSimpleSuccess'
    @resolver.send(:load_events_with_call_id, call_id)
  end
  
  # load and return events for the simple case
  def load_simple_success_events_callee_hangs_up
    call_id = 'testSimpleSuccess_CalleeEnd'
    @resolver.send(:load_events_with_call_id, call_id)
  end  
  
end
