$:.unshift File.join(File.dirname(__FILE__), "..", "lib")
require 'test/unit'
require 'events'
require 'state'

class StateTest < Test::Unit::TestCase
  
  class FakeState < States::State
    attr_reader :mark
    
    def initialize
      @mark = 0
    end
    
    def accept_Bongo(event)
      @mark += 1
    end
    
    def accept_AgentSignIn(event)
      @mark += 1
    end
    
    def accept_Event(event)
      @mark += 1
    end
    
    def accept_Object(event)
      # this should never be called
      @mark += 1
    end
  end
  
  class Bongo
  end
  
  class Kuku
  end
  
  class SomeEvent < Event
  end
  
  def test_duration
    assert_equal(10, States.duration(5,15,30))
    assert_equal(25, States.duration(5,nil,30))
    assert_equal(0, States.duration(nil,nil,15))
    assert_equal(0, States.duration(nil,20,30))
  end
  
  def test_accept  
    state = FakeState.new
    assert_equal(0, state.mark)
    state.accept(Bongo.new)
    assert_equal(1, state.mark)
    state.accept(Kuku.new)
    assert_equal(1, state.mark)
    state.accept(AcdEvents::AgentSignIn.new(1000, "abc", "m_queue"))
    assert_equal(2, state.mark)
    state.accept(SomeEvent.new(2222))
    assert_equal(3, state.mark)
    state.accept(Object.new)
    assert_equal(3, state.mark)    
  end
  
end