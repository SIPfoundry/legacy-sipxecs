$:.unshift File.join(File.dirname(__FILE__), "..", "lib")
require 'test/unit'
require 'main'
require 'events'
require 'state/acd'

class MainTest < Test::Unit::TestCase
  
  # empty event source
  class EventSource
    def each
    end
  end
  
  def test_empty_source
    es = EventSource.new
    st = States::Acd.new
    assert_same(st, process(es,st))
  end
  
  class TestSource
    def each
      yield AcdEvents::Start.new(1000)
      yield AcdEvents::Stop.new(1500) 
    end  
  end
  
  def test_simple_source  
    es = TestSource.new
    st = States::Acd.new
    assert_same(st, process(es,st))
    assert_equal(500, st.get_running_time(2200))
  end  
  
  def test_array_source
    es = [AcdEvents::Start.new(1200), AcdEvents::Stop.new(1400)]
    st = States::Acd.new
    assert_same(st, process(es,st))
    assert_equal(200, st.get_running_time(2000))
  end
  
  
  def test_process_file
    filename = File.join(File.dirname(__FILE__), "sipxacd.log")
    state = process_file(filename, States::Acd.new)
    assert_instance_of(States::Acd, state)
  end
  
  def test_process_invalid_file
    filename = File.join(File.dirname(__FILE__), "sipxacd_invalid.log")
    acd = process_file(filename, States::Acd.new)
    assert_equal(0, acd.calls.size)
    assert_equal(0, acd.agents.size)
  end
end