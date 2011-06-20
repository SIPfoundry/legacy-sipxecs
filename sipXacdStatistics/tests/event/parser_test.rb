$:.unshift File.join(File.dirname(__FILE__), "..", "..", "lib")

require 'test/unit'
require 'event/parser'
require 'events'

class ParserTest < Test::Unit::TestCase
  
  def setup
    @p = Events::Parser.new
    @time = Time.parse("2006-03-21T22:28:00Z").to_i
  end
  
  def test_invalid
    assert_nil @p.parse_line('')
  end
  
  def test_unknown
    assert_nil @p.parse_line('XXXXX : 2006-03-21T20:48:57.811291Z : sip:201@cdhcp178.pingtel.com : "206"<sip:206@cdhcp178.pingtel.com>')
  end  
  
  def test_time_to_sec
    time1 = Events.time_to_sec("2006-03-21T22:28:13.134778Z") 
    time2 = Events.time_to_sec("2006-03-21T22:28:23.134778Z") 
    assert_equal(10, time2 - time1)
    
    # to display time in similar format use Time.now.iso8601(6)
  end

  def test_start_acd  
    event = @p.parse_line('start-acd : 2006-03-21T22:28:13.134778Z')
    assert event.kind_of?(AcdEvents::Start)
    assert(120 > event.time - @time)
  end
  
  def test_stop_acd  
    event = @p.parse_line('stop-acd : 2006-03-21T22:29:27.032440Z')
    assert event.kind_of?(AcdEvents::Stop)
    assert(120 > event.time - @time)
  end

  def test_pick_up
    event = @p.parse_line('pick-up : 2006-03-21T20:05:12.848470Z : sip:Boston@cdhcp178.pingtel.com : sip:201@cdhcp178.pingtel.com : "206"<sip:206@cdhcp178.pingtel.com>')
    assert event.kind_of?(AcdEvents::PickUp)
    assert(120 > event.time - @time)
    assert_equal('sip:201@cdhcp178.pingtel.com', event.agent_uri)
    assert_equal('"206"<sip:206@cdhcp178.pingtel.com>', event.call_id) 
  end

  def test_enter_queue
    event = @p.parse_line('enter-queue : 2006-03-21T20:48:57.811291Z : sip:Boston@cdhcp178.pingtel.com : "206"<sip:206@cdhcp178.pingtel.com>')
    assert event.kind_of?(AcdEvents::EnterQueue)
    assert(120 > event.time - @time)
    assert_equal("sip:Boston@cdhcp178.pingtel.com", event.queue_uri)
    assert_equal('"206"<sip:206@cdhcp178.pingtel.com>', event.call_id)
  end
  
  def test_transfer
    event = @p.parse_line('transfer : 2006-03-21T20:48:57.811291Z : sip:201@cdhcp178.pingtel.com : "206"<sip:206@cdhcp178.pingtel.com>')
    assert event.kind_of?(AcdEvents::Transfer)
    assert(120 > event.time - @time)
    assert_equal('sip:201@cdhcp178.pingtel.com', event.agent_uri)    
    assert_equal('"206"<sip:206@cdhcp178.pingtel.com>', event.call_id)    
  end
  
  def test_terminate
    event = @p.parse_line('terminate : 2006-03-21T20:06:04.781573Z : "206"<sip:206@cdhcp178.pingtel.com>')
    assert event.kind_of?(AcdEvents::Terminate)
    assert(120 > event.time - @time)
    assert_equal('"206"<sip:206@cdhcp178.pingtel.com>', event.call_id)
  end
  
  def test_sign_in
    event = @p.parse_line('sign-in-agent : 2006-03-21T20:18:18.204343Z : sip:Boston@cdhcp178.pingtel.com : sip:201@cdhcp178.pingtel.com')
    assert event.kind_of?(AcdEvents::AgentSignIn)
    assert(120 > event.time - @time)
    assert_equal("sip:Boston@cdhcp178.pingtel.com", event.queue_uri)
    assert_equal('sip:201@cdhcp178.pingtel.com', event.agent_uri) 
  end
  
  def test_sign_out
    event = @p.parse_line('sign-out-agent : 2006-03-21T20:19:47.133248Z : sip:Boston@cdhcp178.pingtel.com : sip:201@cdhcp178.pingtel.com')
    assert event.kind_of?(AcdEvents::AgentSignOut)
    assert(120 > event.time - @time)
    assert_equal("sip:Boston@cdhcp178.pingtel.com", event.queue_uri)
    assert_equal('sip:201@cdhcp178.pingtel.com', event.agent_uri) 
  end
  
  
end
