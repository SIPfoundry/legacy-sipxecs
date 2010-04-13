$:.unshift File.join(File.dirname(__FILE__), "..", "..", "lib")
require 'test/unit'
require 'state/agent'

require 'tests/state/mocks'

class AgentTest < Test::Unit::TestCase
  
  def setup
    @state_mock = Mocks::state_mock
  end
  
  def test_agents_state
    a = States::Agents.new
    assert_equal(0, a.agents.size)    
    a.accept(AcdEvents::AgentSignIn.new(1000, "my_queue", "my_agent"))
    assert_equal(1, a.agents.size)
    assert_equal(1, a.agents["my_agent"].queues.size)
    sign_out = AcdEvents::AgentSignOut.new(2000, "my_queue", "my_agent")
    sign_out.state = @state_mock    
    a.accept(sign_out)
    assert_equal(0, a.agents.size)
  end
  
  def test_agents_state_more_queues
    a = States::Agents.new
    assert_equal(0, a.agents.size)    
    a.accept(AcdEvents::AgentSignIn.new(1000, "queue1", "my_agent"))
    agent = a.agents["my_agent"]
    assert_not_nil(agent)
    
    a.accept(AcdEvents::AgentSignIn.new(1000, "queue2", "my_agent"))
    a.accept(AcdEvents::AgentSignIn.new(1000, "queue2", "another_agent"))
    assert_equal(2, a.agents.size)
    assert_equal(2, a.agents["my_agent"].queues.size)
    
    stats = a.stats(nil, 2000)
    assert_equal(2, stats.size)
    stats.each { | stat | assert_kind_of(AcdStats::Agent, stat) }
    
    sign_out = AcdEvents::AgentSignOut.new(2000, "queue1", "my_agent")
    sign_out.state = @state_mock
    a.accept(sign_out)    
    assert_same(agent, a.agents["my_agent"])
    assert_not_same(agent, a.agents["another_agent"])
    
    assert_equal(2, a.agents.size)
    assert_equal(1, a.agents["my_agent"].queues.size)
    assert_equal("queue2", a.agents["my_agent"].queues[0])
    sign_out = AcdEvents::AgentSignOut.new(2000, "queue2", "another_agent")
    sign_out.state = @state_mock
    a.accept(sign_out)    
    assert_equal(1, a.agents.size)
    assert_equal(1, a.agents["my_agent"].queues.size)
    assert_equal("queue2", a.agents["my_agent"].queues[0])
    sign_out = AcdEvents::AgentSignOut.new(2000, "queue2", "my_agent")
    sign_out.state = @state_mock
    a.accept(sign_out)    
    assert_equal(0, a.agents.size)    
  end
  
  def test_agent_state
    a = States::Agent.new("my_agent")
    assert_equal(0, a.current_state_time(100))
    
    a.accept(AcdEvents::AgentSignIn.new(1000, "my_queue", "my_agent"))
    assert(States::Agent::IDLE, a.state)
    assert_equal(500, a.current_state_time(1500))
    
    a.accept(AcdEvents::PickUp.new(1200, "my_queue", "my_agent", "call_id"))
    assert(States::Agent::BUSY, a.state)
    assert_equal(600, a.current_state_time(1800))
    
    a.accept(AcdEvents::Terminate.new(1300, "call_id"))
    assert(States::Agent::IDLE, a.state)
    assert_equal(100, a.current_state_time(1400))
  end
  
  def test_agent_history
    ah = States::AgentHistory.new
    actual = ah.get_history(Time.at(0))
    assert_equal(0, actual.size)
    
    qi = States::QueueInfo.new(50)
    qi.sign_out_time = 100
    ah.add(States::Agent::new("a"), "queue", qi, 100)
    actual = ah.get_history(Time.at(0))
    assert_equal(1, actual.size)    
  end
end
