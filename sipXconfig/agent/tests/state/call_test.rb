$:.unshift File.join(File.dirname(__FILE__), "..", "..", "lib")
require 'test/unit'
require 'state/call'
require 'state/acd'
require 'state/acd_stats'

require 'tests/state/mocks'

class CallTest < Test::Unit::TestCase
  
  def setup
    @state_mock = Mocks::state_mock
  end
  
  def test_calls_state
    a = States::Calls.new
    assert_equal(0, a.calls.size)
    a.accept(AcdEvents::EnterQueue.new(1000, "my_queue", "my_call"))
    assert_equal(1, a.calls.size)
    terminate = AcdEvents::Terminate.new(1000, "my_call")
    terminate.state = @state_mock
    a.accept(terminate)
    assert_equal(0, a.calls.size)    
  end
  
  def test_calls_state_enter_queue
    a = States::Calls.new
    assert_equal(0, a.calls.size)
    a.accept(AcdEvents::EnterQueue.new(1000, "some queue", "my_call"))    
    assert_equal(1, a.calls.size)
    terminate = AcdEvents::Terminate.new(1200, "my_call")
    terminate.state = @state_mock
    a.accept(terminate)
    assert_equal(0, a.calls.size)    
  end
  
  def test_calls_state_more_calls
    a = States::Calls.new
    assert_equal(0, a.calls.size)
    a.accept(AcdEvents::EnterQueue.new(1000, "some queue", "call 1"))
    call = a.calls["call 1"]
    assert_not_nil(call)
    
    a.accept(AcdEvents::EnterQueue.new(1000, "some queue", "call 2"))    
    a.accept(AcdEvents::EnterQueue.new(1000, "some queue", "call 3"))    
    a.accept(AcdEvents::EnterQueue.new(1000, "some queue", "call 4"))    
    assert_equal(4, a.calls.size)
    a.accept(AcdEvents::EnterQueue.new(1100, "some queue", "call 1"))
    assert_same(call, a.calls["call 1"])
    
    assert_equal(4, a.calls.size)
    
    stats = a.stats(nil, 2000)
    assert_equal(4, stats.size)
    stats.each { | stat | assert_kind_of(AcdStats::Call, stat) }        
    
    terminate = AcdEvents::Terminate.new(1200, "call 2")
    terminate.state = @state_mock
    a.accept(terminate)
    
    terminate = AcdEvents::Terminate.new(1200, "call 3")
    terminate.state = @state_mock
    a.accept(terminate)
    
    assert_equal(2, a.calls.size)    
  end
  
  def test_call
    call = States::Call.new("my call")
    assert_equal(0, call.processing_time(1000))
    assert_equal(0, call.total_wait_time(1000))
    
    call.accept(AcdEvents::EnterQueue.new(1100, "some queue", "my call"))
    assert_equal(0, call.processing_time(1110))
    assert_equal(10, call.total_wait_time(1110))
    assert_equal("some queue", call.queue_uri)
    assert_nil(call.agent_uri)
    assert_equal(States::Call::WAITING, call.state)
    
    stats = AcdStats::Call.new.init(nil, call, 2000)
    assert_equal(0, stats.processing_time)
    assert_equal(900, stats.total_wait_time)
    assert_equal("my call", stats.from)
    assert_equal(States::Call::WAITING, stats.state)    
    
    call.accept(AcdEvents::EnterQueue.new(1200, "my queue", "my call"))
    assert_equal(0, call.processing_time(1210))
    assert_equal(110, call.total_wait_time(1210))
    assert_equal("my queue", call.queue_uri)
    assert_nil(call.agent_uri)
    assert_equal(States::Call::WAITING, call.state)
    
    call.accept(AcdEvents::PickUp.new(1300, "my queue", "my agent", "my call"))
    assert_equal(10, call.processing_time(1310))
    assert_equal(200, call.total_wait_time(1310))
    assert_equal("my queue", call.queue_uri)
    assert_equal("my agent", call.agent_uri)
    assert_equal(States::Call::IN_PROGRESS, call.state)
    
    stats = AcdStats::Call.new.init(nil, call, 2000)
    assert_equal(700, stats.processing_time)
    assert_equal(200, stats.total_wait_time)
    assert_equal("my call", stats.from)
    assert_equal(States::Call::IN_PROGRESS, stats.state)    
    
    terminate = AcdEvents::Terminate.new(1400, "my call")
    terminate.state = @state_mock
    call.accept(terminate)
    assert_equal(100, call.processing_time(1310))
    assert_equal(200, call.total_wait_time(1310))
    assert_equal("my queue", call.queue_uri)
    assert_equal("my agent", call.agent_uri)
    assert_equal(States::Call::TERMINATE, call.state)
  end
  
  
  def test_call_overflow_no_waiting
    call = States::Call.new("my call")
    assert_equal(0, call.processing_time(1000))
    assert_equal(0, call.total_wait_time(1000))
    
    call.accept(AcdEvents::EnterQueue.new(1100, "some queue", "my call"))
    assert_equal(0, call.processing_time(1110))
    assert_equal(10, call.total_wait_time(1110))
    assert_equal("some queue", call.queue_uri)
    assert_nil(call.agent_uri)
    assert_equal(States::Call::WAITING, call.state)
    
    stats = AcdStats::Call.new.init(nil, call, 2000)
    assert_equal(0, stats.processing_time)
    assert_equal(900, stats.total_wait_time)
    assert_equal("my call", stats.from)
    assert_equal(States::Call::WAITING, stats.state)    
    
    # no Enter Queue call - call is picked up by queue from another agent    
    call.accept(AcdEvents::PickUp.new(1300, "my queue", "my agent", "my call"))
    assert_equal(10, call.processing_time(1310))
    assert_equal(200, call.total_wait_time(1310))
    assert_equal("my queue", call.queue_uri)
    assert_equal("my agent", call.agent_uri)
    assert_equal(States::Call::IN_PROGRESS, call.state)
    
    stats = AcdStats::Call.new.init(nil, call, 2000)
    assert_equal(700, stats.processing_time)
    assert_equal(200, stats.total_wait_time)
    assert_equal("my call", stats.from)
    assert_equal(States::Call::IN_PROGRESS, stats.state)    
    
    terminate = AcdEvents::Terminate.new(1400, "my call")
    terminate.state = @state_mock
    call.accept(terminate)
    assert_equal(100, call.processing_time(1310))
    assert_equal(200, call.total_wait_time(1310))
    assert_equal("my queue", call.queue_uri)
    assert_equal("my agent", call.agent_uri)
    assert_equal(States::Call::TERMINATE, call.state)
  end
  
  
  def test_call_unanswered
    call = States::Call.new("my call")
    assert_equal(0, call.processing_time(1000))
    assert_equal(0, call.total_wait_time(1000))
    
    call.accept(AcdEvents::EnterQueue.new(1200, "my queue", "my call"))
    assert_equal(0, call.processing_time(1210))
    assert_equal(10, call.total_wait_time(1210))
    assert_equal("my queue", call.queue_uri)
    assert_nil(call.agent_uri)
    assert_equal(States::Call::WAITING, call.state)
    
    event = AcdEvents::Terminate.new(1400, "my call")
    event.state = @state_mock
    call.accept(event)
    assert_equal(0, call.processing_time(1310))
    assert_equal(110, call.total_wait_time(1310))
    assert_equal("my queue", call.queue_uri)
    assert_nil(call.agent_uri)
    assert_equal(States::Call::UNANSWERED, call.state)
  end
  
  def test_call_transfer
    acd = States::Acd.new
    acd.accept(AcdEvents::AgentSignIn.new(1000, "queue", "agent1"))
    acd.accept(AcdEvents::AgentSignIn.new(1000, "queue", "agent2"))
    acd.accept(AcdEvents::EnterQueue.new(1200, "queue", "call"))    
    call = acd.calls.first
    assert_equal(States::Call::WAITING, call.state)    
    acd.accept(AcdEvents::PickUp.new(1300, "queue", "agent1", "call"))
    
    assert_equal(States::Agent::BUSY, acd.agent("agent1").state)
    assert_equal(States::Agent::IDLE, acd.agent("agent2").state)    
    assert_equal(States::Call::IN_PROGRESS, call.state)    
    assert_equal("agent1", call.agent_uri)    
    assert_equal(50, call.processing_time(1350))
    
    acd.accept(AcdEvents::Transfer.new(1400, "agent2", "call"))
    assert_equal(States::Agent::IDLE, acd.agent("agent1").state)
    assert_equal(States::Agent::BUSY, acd.agent("agent2").state)
    assert_equal(States::Call::IN_PROGRESS, call.state)    
    assert_equal("agent2", call.agent_uri)
    assert_equal(150, call.processing_time(1450))    
  end
end
