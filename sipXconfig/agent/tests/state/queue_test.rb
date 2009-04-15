$:.unshift File.join(File.dirname(__FILE__), "..", "..", "lib")
require 'test/unit'
require 'state/queue'

class QueueTest < Test::Unit::TestCase
  def test_queues
    q = States::Queues.new
    assert(q.queues.empty?)
    
    q.accept(AcdEvents::EnterQueue.new(1200, "my_queue", "my_call"))
    assert_equal(1, q.queues.size)
    queue = q.queues["my_queue"]
    assert_not_nil(queue)
    
    q.accept(AcdEvents::AgentSignIn.new(1300, "my_queue", "my_agent"))
    assert_equal(1, q.queues.size)
    assert_same(queue, q.queues["my_queue"])
    
    q.accept(AcdEvents::AgentSignIn.new(1300, "my_queue", "my_agent"))
    assert_equal(1, q.queues.size)
    
    q.accept(AcdEvents::AgentSignIn.new(1300, "another_queue", "my_agent"))
    assert_equal(2, q.queues.size)
    assert_not_same(queue, q.queues["another_queue"])    
  end
  
  def test_queue_with_agents
    q = States::Queue.new("my_queue")
    q.accept(AcdEvents::AgentSignIn.new(1300, "my_queue", "my_agent"))
    assert_equal(1, q.agents.size)
    
    q.accept(AcdEvents::AgentSignIn.new(1350, "my_queue", "another_agent"))    
    assert_equal(2, q.agents.size)
    
    q.accept(AcdEvents::AgentSignOut.new(1400, "my_queue", "my_agent"))
    assert_equal(1, q.agents.size)
    
    q.accept(AcdEvents::AgentSignOut.new(1450, "my_queue", "another_agent"))
    assert_equal(0, q.agents.size)
  end
  
  def test_get_busy_idle_agents_empty
    q = States::Queue.new("my_queue")
    busy, idle = q.busy_idle_agents([])
    assert_equal(0, busy)
    assert_equal(0, idle)
  end
  
  
  def test_get_busy_idle_agents
    a1 = States::Agent.new("agent1")
    a1.accept(AcdEvents::AgentSignIn.new(1000, "my_queue", "my_agent"))
    
    a2 = States::Agent.new("agent1")
    
    q = States::Queue.new("my_queue")
    busy, idle = q.busy_idle_agents([a1, a2])
    assert_equal(0, busy)
    assert_equal(1, idle)
    
    a2.accept(AcdEvents::AgentSignIn.new(1000, "my_queue", "my_agent"))
    busy, idle = q.busy_idle_agents([a1, a2])
    assert_equal(0, busy)
    assert_equal(2, idle)
    
    a2.accept(AcdEvents::PickUp.new(1000, "my_queue", "my_agent", "call_id"))
    busy, idle = q.busy_idle_agents([a1, a2])
    assert_equal(1, busy)
    assert_equal(1, idle)
    
    a2.accept(AcdEvents::Terminate.new(1000, "call_id"))
    busy, idle = q.busy_idle_agents([a1, a2])
    assert_equal(0, busy)
    assert_equal(2, idle)
  end

  def test_avg_wait_time_empty
    q = States::Queue.new("my queue")
    avg, count, max = q.waiting_calls([], 1000)
    assert_equal(0, avg)    
    assert_equal(0, avg)    
    assert_equal(0, max)    
  end

  def test_avg_abandonded_time_empty
    q = States::Queue.new("my queue")
    assert_equal(0, q.abandoned_calls(1000)[0])    
  end
  
  def test_average_wait_time
    call1 = States::Call.new("my call 1")
    call1.accept(AcdEvents::EnterQueue.new(1000, "my queue", "my call 1"))    
    
    call2 = States::Call.new("my call 1")
    call2.accept(AcdEvents::EnterQueue.new(1200, "my queue", "my call 2"))

    q = States::Queue.new("my queue")
    assert_equal(1, q.waiting_calls([call1], 2000)[0])
    assert_equal(1000, q.waiting_calls([call1], 2000)[1])    
    assert_equal(900, q.waiting_calls([call1, call2], 2000)[1])        
    assert_equal(1000, q.waiting_calls([call1, call2], 2000)[2])    
  end


  def test_queue_stats
    acd = States::Acd.new
    
    events = [
      AcdEvents::EnterQueue.new(1000, "my queue", "call 1"),
      AcdEvents::EnterQueue.new(1200, "my queue", "call 2"),
      AcdEvents::AgentSignIn.new(1300, "my queue", "agent 1"),
      AcdEvents::AgentSignIn.new(1350, "my queue", "agent 2")
    ]
    
    events.each { |e| acd.accept(e) }
    
    stats = acd.queue_stats(2000)
    assert_equal(1, stats.size)
    
    assert_equal(2, stats[0].waiting_calls)
    assert_equal(900, stats[0].avg_wait_time)
    assert_equal(1000, stats[0].max_wait_time)
    
    assert_equal(0, stats[0].avg_abandoned_time)
    assert_equal(0, stats[0].max_abandoned_time)
    assert_equal(0, stats[0].abandoned_calls)
    
    assert_equal(0, stats[0].avg_processing_time)
    assert_equal(0, stats[0].max_processing_time)
    assert_equal(0, stats[0].processed_calls)
        
    assert_equal(2, stats[0].idle_agents)
    assert_equal(0, stats[0].busy_agents)
    
    
    events = [
      AcdEvents::PickUp.new(2000, "my queue", "agent 1", "call 1")
    ]
    events.each { |e| acd.accept(e) }
    
    stats = acd.queue_stats(2100)
    assert_equal(1, stats[0].idle_agents)
    assert_equal(1, stats[0].busy_agents)
    # call1 - wait time (2000 - 1000) = 1000
    # call2 - wait time (2100 - 1200) = 900

    assert_equal(1, stats[0].waiting_calls)
    assert_equal(950, stats[0].avg_wait_time)
    assert_equal(1000, stats[0].max_wait_time)
    
    assert_equal(0, stats[0].avg_abandoned_time)
    assert_equal(0, stats[0].max_abandoned_time)
    assert_equal(0, stats[0].abandoned_calls)
    
    assert_equal(0, stats[0].avg_processing_time)
    assert_equal(0, stats[0].max_processing_time)
    assert_equal(0, stats[0].processed_calls)    
    
    events = [
      AcdEvents::Terminate.new(2200, "call 1")
    ]
    events.each { |e| acd.accept(e) }
    stats = acd.queue_stats(2300)
    assert_equal(2, stats[0].idle_agents)
    assert_equal(0, stats[0].busy_agents)
    
    # call1 - wait time (2000 - 1000) = 1000
    # call2 - wait time (2300 - 1200) = 1100
    assert_equal(1, stats[0].waiting_calls)
    assert_equal(1050, stats[0].avg_wait_time)
    assert_equal(1100, stats[0].max_wait_time)
    
    assert_equal(0, stats[0].abandoned_calls)
    assert_equal(0, stats[0].avg_abandoned_time)
    assert_equal(0, stats[0].max_abandoned_time)
    
    assert_equal(1, stats[0].processed_calls)    
    assert_equal(200, stats[0].avg_processing_time)
    assert_equal(200, stats[0].max_processing_time)
        
    # terminate call that has not been picked up
    events = [
      AcdEvents::Terminate.new(2300, "call 2")
    ]
    events.each { |e| acd.accept(e) }
    stats = acd.queue_stats(2400)
    
    assert_equal(0, stats[0].waiting_calls)
    assert_equal(1050, stats[0].avg_wait_time)
    assert_equal(1100, stats[0].max_wait_time)

    assert_equal(1, stats[0].abandoned_calls)
    assert_equal(1100, stats[0].avg_abandoned_time)    
    assert_equal(1100, stats[0].max_abandoned_time)    

    assert_equal(1, stats[0].processed_calls)    
    assert_equal(200, stats[0].avg_processing_time)
    assert_equal(200, stats[0].max_processing_time)
  end
end
