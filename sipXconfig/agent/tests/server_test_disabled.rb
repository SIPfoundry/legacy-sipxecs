# This test only passes with the lastest SOAP4R binary so
# it is disabled
$:.unshift File.join(File.dirname(__FILE__), "..", "lib")
require 'test/unit'
require 'net/http'
require 'state/acd'
require 'server'
require 'soap/rpc/driver'
require 'main'

class ServerTest < Test::Unit::TestCase
  
  def start_server
    
    # not starting eachtime, port doesn't get freed intime
    return if defined? @@server
    puts "starting"
    
    filename = File.join(File.dirname(__FILE__), "sipxacd.log")
    state = process_file(filename, States::Acd.new)
    httpd = StatsServer.new(state, :Port => 2000)
    
    trap(:INT) do
      httpd.shutdown
    end
    
    @@server = Thread.new do
      httpd.start
    end
    puts "started"    
  end
  
  def setup
    super
    start_server
    @stats = SOAP::RPC::Driver.new('http://localhost:2000/', 'urn:StatsService')
    @stats.wiredump_dev = STDERR if $DEBUG
    @stats.add_method('getBirdArray')
    @stats.add_method('getAgentStats')
    @stats.add_method('getQueueStats')
    @stats.add_method('getCallStats')
    @stats.add_method('getStringCallStats')
    @stats.add_method('getCallHistory', 'fromTime')
    @stats.add_method('getAgentHistory', 'fromTime')
  end
  
  def teardown
  end
  
  # fails SOAP4R in w/1.8.4i
  def test_get_wsdl
    response = Net::HTTP.get_response(URI.parse('http://localhost:2000/wsdl/sipxstats.wsdl'))
    assert_equal('200', response.code)
  end
  
  def test_get_call_string_stats
    actual = @stats.getStringCallStats    
    assert_equal("hello", actual)
  end
  
  def test_get_birds
    actual = @stats.getBirdArray    
    assert_equal("robin", actual[0].species)
    assert_equal("bluejay", actual[1].species)
  end
  
  def test_get_call_stats
    actual = @stats.getCallStats
    assert_kind_of(Array, actual)
    assert_equal(0, actual.size)
  end 
  
  def test_get_agent_stats
    actual = @stats.getAgentStats
    assert_kind_of(Array, actual)
    assert_equal(1, actual.size)
  end 
  
  def test_get_queue_stats
    actual = @stats.getQueueStats
    assert_kind_of(Array, actual)
    assert_equal(1, actual.size)
  end
  
  def test_get_call_history
    actual = @stats.getCallHistory(Time.at(0))
    assert_kind_of(Array, actual)
    assert_equal(3, actual.size)    
  end
  
  def test_get_partial_call_history
    time = Time.parse("2006-03-21T20:48:03.523657Z")
    actual = @stats.getCallHistory(time)
    assert_kind_of(Array, actual)
    assert_equal(2, actual.size)    
  end
  
  
  def test_get_agent_history
    actual = @stats.getAgentHistory(Time.at(0))
    assert_kind_of(Array, actual)
    assert_equal(1, actual.size)    
  end
  
  def test_primitive_read_write
    assert_equal('queue stats here', @stats.getQueueStats)
    assert_equal('call stats here', @stats.getCallStats)
  end
end
