$:.unshift File.join(File.dirname(__FILE__), "..", "lib")
require 'test/unit'
require 'events'

class EventsTest < Test::Unit::TestCase
  
  def test_acd_start
    event = AcdEvents::Start.new(1000)
    assert_equal(1000, event.time)
  end
  
  def test_call_enter_queue
    event = AcdEvents::EnterQueue.new(2000,  "test_queue", "my_call")
    assert_equal(2000, event.time)
    assert_equal("my_call", event.call_id)
    assert_equal("my_call", event.from)
    assert_equal("test_queue", event.queue_uri)
    
    event = AcdEvents::EnterQueue.new(2000,  "test_queue", "my_call", "122333")
    assert_equal(2000, event.time)
    assert_equal("122333", event.call_id)
    assert_equal("my_call", event.from)
    assert_equal("test_queue", event.queue_uri)
  end
  
  def test_call_terminate
    event = AcdEvents::Terminate.new(2000, "my_call")
    assert_equal(2000, event.time)
    assert_equal("my_call", event.from)
    assert_equal("my_call", event.call_id)

    event = AcdEvents::Terminate.new(2000, "my_call", "122333")
    assert_equal(2000, event.time)
    assert_equal("my_call", event.from)
    assert_equal("122333", event.call_id)
  end  
  
  def test_call_pick_up
    event = AcdEvents::PickUp.new(2000, "my-queue", "my_agent", "my_call")
    assert_equal(2000, event.time)
    assert_equal("my_call", event.call_id)
    assert_equal("my_call", event.from)
    assert_equal("my_agent", event.agent_uri)
    
    event = AcdEvents::PickUp.new(2000, "my-queue", "my_agent", "my_call", "122333")
    assert_equal(2000, event.time)
    assert_equal("122333", event.call_id)
    assert_equal("my_call", event.from)
    assert_equal("my_agent", event.agent_uri)
  end    
  
  def test_agent_sign_in
    event = AcdEvents::AgentSignIn.new(3000, "test_queue", "my_agent")
    assert_equal(3000, event.time)
    assert_equal("my_agent", event.agent_uri)
    assert_equal("test_queue", event.queue_uri)
  end
  
  def test_agent_sign_out
    event = AcdEvents::AgentSignOut.new(4000, "test_queue", "my_agent")
    assert_equal(4000, event.time)
    assert_equal("my_agent", event.agent_uri)
    assert_equal("test_queue", event.queue_uri)
  end
  
end