#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'test/unit'
require 'thread'
require 'dbi'

$:.unshift File.join(File.dirname(__FILE__), '..', 'lib')
require 'state'

$:.unshift File.join(File.dirname(__FILE__), '..', 'test')
require 'test_util'
include TestUtil

class StateTest < Test::Unit::TestCase
  
  class DummyCdr

    attr_reader :counter, :call_id, :callee_aor, :reference
    
    def initialize(call_id, log = nil, callee_aor="sip:221@example.com")
      @counter = 0
      @call_id = call_id
      @callee_aor = callee_aor
    end
    
    def accept(cse)
      @counter += 1
      return self if @counter > 1        
    end
    
    def terminated?
      true
    end
    
    def retire; end
  end
  
  class DummyCse < CallStateEvent
    attr_reader :id, :call_id, :event_time, :event_type
    
    def initialize(call_id, event_time = DBI::Timestamp.new(2000,1,1,1,1,0), event_type = 'R')
      @call_id = call_id
      @event_time = event_time
      @event_type = event_type
      @id = 1
    end    
  end
  
  def test_empty
    q1 = Queue.new
    q2 = Queue.new
    t = Thread.new(State.new(q1, q2)) { | s | s.run }    
    q1.enq(nil)
    t.join
  end
  
  def test_accept
    observer = DummyQueue.new
    cse1 = DummyCse.new('id1')
    cse2 = DummyCse.new('id2')
    
    state = State.new([], observer, DummyCdr )
    
    state.accept(cse1)    
    assert_equal(0, observer.counter)
    
    state.accept(cse2)
    assert_equal(0, observer.counter)
    
    state.accept(cse1)    
    assert_equal(1, observer.counter)
    
    state.accept(cse2)
    assert_equal(2, observer.counter)    
  end
  
  def test_retired
    observer = DummyQueue.new
    cse1 = DummyCse.new('id1')
    
    state = State.new([], observer, DummyCdr )
    state.accept(cse1)    
    state.accept(cse1)    
    assert_equal(1, observer.counter)
    assert_equal(2, observer.last.counter)
    
    state.accept(cse1)
    state.accept(cse1)
    # still only 1 since CDR was retired
    assert_equal(1, observer.counter)
    assert_equal(2, observer.last.counter)
    
    state.flush_retired(0)
    state.accept(cse1)
    state.accept(cse1)
    # we should be notified again
    assert_equal(2, observer.counter)
    assert_equal(2, observer.last.counter)
  end
  
  def test_retired_with_age
    observer = DummyQueue.new
    cse1 = DummyCse.new('id1')
    
    state = State.new([], observer, DummyCdr )
    state.accept(cse1)
    state.accept(cse1)
    assert_equal(1, observer.counter)
    assert_equal(2, observer.last.counter)
    # generation == 2
    
    state.accept(cse1)
    state.accept(cse1)
    # still only 1 since CDR was retired
    assert_equal(1, observer.counter)
    assert_equal(2, observer.last.counter)
    # generation == 4
    
    state.flush_retired(3) # it's only 2 generations old at his point
    state.accept(cse1)
    state.accept(cse1)
    # nothing was flushed - still one
    assert_equal(1, observer.counter)    
  end
  
  class MockCdr
    @@results = []
    
    attr_reader :call_id, :start_time, :callee_aor, :reference
    
    def initialize(call_id, log=nil, callee_aor="sip:221@example.com")
      @call_id = call_id
      @callee_aor = callee_aor
    end
    
    def accept(cse)
      @start_time = cse.event_time  
      self if @@results.shift
    end
    
    def force_finish; end
    def retire; end
    
    def terminated?
      @@results.shift 
    end
    
    def MockCdr.push_results(*arg)
      @@results.push(*arg)
    end
    
    def MockCdr.results(*arg)
      @@results = arg
    end        
  end
  
  def test_failed
    observer = DummyQueue.new
    cse1 = DummyCse.new('id1')
    
    # results of calls to accept and terminated?
    MockCdr.results( true, false )
    state = State.new([], observer, MockCdr )
    
    state.accept(cse1)
    assert_equal(0, observer.counter)
    
    state.flush_failed(1)
    assert_equal(0, observer.counter)
    
    state.flush_failed(0)
    assert_equal(1, observer.counter)
    
    MockCdr.push_results( true, true )
    # now we should have a call in retired list so new events will be ignored
    state.accept(cse1)
    assert_equal(1, observer.counter)
  end
  
  
  def test_failed_and_then_succeeded
    observer = DummyQueue.new
    cse1 = DummyCse.new('id1')
    
    # results of calls to accept and terminated?
    MockCdr.results( true, false, true, true )
    state = State.new([], observer, MockCdr )
    
    state.accept(cse1)
    assert_equal(0, observer.counter)
    
    state.accept(cse1)
    assert_equal(1, observer.counter)
    
    state.flush_failed(0)
    # still only one - nothing flushed
    assert_equal(1, observer.counter)
  end  
  
  
  def test_run
    out_queue = []
    
    cse1 = DummyCse.new('id1')    
    in_queue = [
    cse1, cse1, [:flush_failed, 0]
    ]
    
    # results of calls to accept and terminated?
    MockCdr.results( true, false, true, true )
    state = State.new(in_queue, out_queue, MockCdr )
    state.run
    
    # still only one - nothing flushed, and nil as the second
    assert_equal(2, out_queue.size)
    assert_nil(out_queue[1])
  end
  
  def test_retire_long_calls
    out_queue = []
    cse1 = DummyCse.new('id1', DBI::Timestamp.new(2000,1,1,10,1,1))    
    cse2 = DummyCse.new('id2', DBI::Timestamp.new(2000,1,1,10,2,41)) # cse1 time + 100 seconds
    cse3 = DummyCse.new('id3', DBI::Timestamp.new(2000,1,1,10,4,21)) # cse1 time to 200 seconds   
    in_queue = [
    cse1, cse2, cse3, [:retire_long_calls, 150]
    ]
    MockCdr.results( false, false, false, false, false, false )
    state = State.new(in_queue, out_queue, MockCdr)
    state.run    
    
    # a single CDR will be recorded
    assert_equal(2, out_queue.size)
    assert_nil(out_queue[1])
    
    # and one CDR remains in the state
    assert_equal(2, state.active_cdrs.size)
  end
  
  def test_retire_long_ringing_calls
    out_queue = []
    cse1 = DummyCse.new('id1', DBI::Timestamp.new(2000,1,1,10,1,1))    
    in_queue = [
    cse1, [:retire_long_ringing_calls, 120]
    ]
    MockCdr.results( true, false )
    state = State.new(in_queue, out_queue, MockCdr)
    state.run    
    
    # a single CDR will be recorded
    assert_equal(2, out_queue.size)
    assert_nil(out_queue[1])
    
  end
  
  def test_flush_failed_calls
    out_queue = []
    MockCdr.results(true, false, false)
    state = State.new([], out_queue, MockCdr)
    state.accept(DummyCse.new('id1', DBI::Timestamp.new(2000,1,1,10,1,0)))
    state.accept(DummyCse.new('id2', DBI::Timestamp.new(2000,1,1,10,3,31)))
    assert_equal(0, out_queue.size)
    
    state.flush_failed_calls(160)
    assert_equal(0, out_queue.size)
    
    state.flush_failed_calls(150)
    assert_equal(1, out_queue.size)
  end
end
