$:.unshift File.join(File.dirname(__FILE__), "..", "..", "lib")
require 'test/unit'
require 'state/terminated_calls'

class TerminatedCallsTest < Test::Unit::TestCase
  
  def setup
    @tc = States::TerminatedCalls.new
  end
  
  
  def test_empty
    total, count, max = @tc.total_count_max(1000)
    assert_equal(0, total)
    assert_equal(0, count)
    assert_equal(0, max)
  end
  
  def test_single
    @tc.add_call(1000, 125)
    total, count, max = @tc.total_count_max(1001)
    assert_equal(125, total)
    assert_equal(1, count)
    assert_equal(125, max)    
    total, count = @tc.total_count_max(1100)
    assert_equal(125, total)
    assert_equal(1, count)    
  end
  
  def test_simple
    @tc.add_call(1000, 1)
    @tc.add_call(1001, 3)
    @tc.add_call(1002, 2)
    total, count, max = @tc.total_count_max(1003)
    assert_equal(6, total)
    assert_equal(3, count)
    assert_equal(3, max)
  end
  
  def test_forget_default_interval
    @tc.add_call(1000, 1)
    
    interval = 30 * 60;
    
    @tc.add_call(1001 + interval, 2)
    @tc.add_call(1002 + interval, 3)
    
    total, count = @tc.total_count_max(1003 + interval)
    assert_equal(5, total)
    assert_equal(2, count)      
  end
  
  def test_forget_custom_interval
    interval = 60;
    tc = States::TerminatedCalls.new(interval)
    tc.add_call(1000, 1)
    
    
    tc.add_call(1001 + interval, 2)
    tc.add_call(1002 + interval, 3)
    
    total, count = tc.total_count_max(1003 + interval)
    assert_equal(5, total)
    assert_equal(2, count)      
  end
  
end
