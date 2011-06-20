$:.unshift File.join(File.dirname(__FILE__), "..", "..", "lib")
require 'test/unit'
require 'state/history'

include States

class DummyItem
  attr_reader :time
  
  def initialize(time_i)
    @time = Time.at(time_i)
  end
end

class AcdTest < Test::Unit::TestCase
  
  def test_history_empty
    h = History.new
    assert(h.get_history(Time.at(0)).empty?)    
  end
  
  def test_history
    h = History.new(30, 0)
    h.add(DummyItem.new(10), 11)
    h.add(DummyItem.new(20), 21)
    h.add(DummyItem.new(20), 22)
    h.add(DummyItem.new(30), 31)
    assert_equal(4, h.get_history(Time.at(0)).size)    
    assert_equal(3, h.get_history(Time.at(15)).size)    
    assert_equal(3, h.get_history(Time.at(20)).size)    
    assert_equal(1, h.get_history(Time.at(30)).size)    
    assert_equal(0, h.get_history(Time.at(31)).size)        
  end
  
  def test_trim_based_on_time
    h = History.new(20, 0)
    h.add(DummyItem.new(10), 11)
    h.add(DummyItem.new(20), 21)
    assert_equal(2, h.get_history(Time.at(0)).size)    
    h.add(DummyItem.new(21), 40)
    assert_equal(2, h.get_history(Time.at(0)).size)
    h.add(DummyItem.new(21), 60)
    assert_equal(1, h.get_history(Time.at(0)).size)    
  end
  
  def test_trim_based_on_size
    h = History.new(20, 3)
    h.add(DummyItem.new(10), 11)
    h.add(DummyItem.new(20), 21)
    assert_equal(2, h.get_history(Time.at(0)).size)
    h.add(DummyItem.new(21), 40)
    assert_equal(3, h.get_history(Time.at(0)).size)
    h.add(DummyItem.new(25), 60)
    assert_equal(3, h.get_history(Time.at(0)).size)
  end

  def test_default_memory
    # default memory is 90 minutes
    h = History.new(30, 0)
    time1 = Time.at(1000)
    time2 = time1 + 30 * 60
    h.add(DummyItem.new(time1.to_i), time1.to_i)    
    h.add(DummyItem.new(time2.to_i), time2.to_i + 1)
    assert_equal(1, h.get_history(Time.at(0)).size)    
  end
  
  def test_out_of_order
    h = History.new
    h.add(DummyItem.new(10), 11)
    
    begin
      h.add(DummyItem.new(5), 21)
      fail "Should have thrown an error"
    rescue ItemOutOfOrderError
      puts
    end  
  end
end
