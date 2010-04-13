$:.unshift File.join(File.dirname(__FILE__), "..", "..", "lib")
require 'test/unit'
require 'state/acd'

class AcdTest < Test::Unit::TestCase
  
  def test_acd_state
    acd = States::Acd.new
    assert_equal(0, acd.get_running_time(900))
    acd.accept(AcdEvents::Start.new(1000))
    assert_equal(50, acd.get_running_time(1050))
    acd.accept(AcdEvents::Stop.new(1200))
    assert_equal(200, acd.get_running_time(1350))
  end
  
  def test_acd_state_restart
    acd = States::Acd.new
    acd.accept(AcdEvents::EnterQueue.new(800, "my queue", "call1"))
    assert_equal(0, acd.get_running_time(900))
    acd.accept(AcdEvents::Start.new(1000))
    acd.accept(AcdEvents::EnterQueue.new(1201, "my queue", "call1"))
    acd.accept(AcdEvents::EnterQueue.new(1202, "my queue", "call2"))
    acd.accept(AcdEvents::EnterQueue.new(1203, "my queue", "call3"))    
    assert_equal(3, acd.calls.size)
    acd.accept(AcdEvents::Stop.new(1300))
    acd.accept(AcdEvents::Start.new(1000))
    assert_equal(0, acd.calls.size)
  end    
  
  def test_queue_stats_empty
    acd = States::Acd.new
    actual = acd.queue_stats(2000)
    assert_kind_of(Array, actual)
    assert_equal(0, actual.size)
  end
  
  def test_calls_stats_empty
    acd = States::Acd.new
    actual = acd.agent_stats(2000)
    assert_kind_of(Array, actual)
    assert_equal(0, actual.size)
  end
  
  def test_agents_stats_empty
    acd = States::Acd.new
    actual = acd.call_stats(2000)
    assert_kind_of(Array, actual)
    assert_equal(0, actual.size)
  end
  
  def test_agent_history_empty
    acd = States::Acd.new
    actual = acd.agent_history(Time.at(0))
    assert_kind_of(Array, actual)
    assert_equal(0, actual.size)
  end
  
  def test_call_history_empty
    acd = States::Acd.new
    actual = acd.call_history(Time.at(0))
    assert_kind_of(Array, actual)
    assert_equal(0, actual.size)
  end  
end
