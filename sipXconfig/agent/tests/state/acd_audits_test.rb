$:.unshift File.join(File.dirname(__FILE__), "..", "..", "lib")
require 'test/unit'
require 'state/agent'
require 'state/call'
require 'state/acd_audits'

include AcdStats

class AcdAuditsTest < Test::Unit::TestCase

  def setup
    @state_mock = Object.new    
    class <<@state_mock
      def queue(queue_uri); end      
      def agent(agent_uri); end      
    end
  end
      
  def test_agent_audit
    agent = States::Agent.new("agent")
    qi = States::QueueInfo.new(10)
    qi.sign_out_time = 100
    agent_audit = AgentAudit.new(agent, "queue", qi)
    assert_equal("agent", agent_audit.agent_uri)
    assert_equal("queue", agent_audit.queue_uri)
    # 100 seconds is Jan 1 00:01:40 1970
    assert_equal(10, agent_audit.sign_in_time.sec)
    assert_equal(40, agent_audit.sign_out_time.sec)
  end
  
  def test_call_audit
    call = States::Call.new("my call")
    call.accept(AcdEvents::EnterQueue.new(1100, "some queue", "my call"))
    call.accept(AcdEvents::EnterQueue.new(1200, "my queue", "my call"))
    call.accept(AcdEvents::PickUp.new(1300, "my queue", "my agent", "my call"))
    terminate = AcdEvents::Terminate.new(1400, "my call")
    terminate.state = @state_mock
    call.accept(terminate)
    call_audit = CallAudit.new(call)

    assert_equal("my call", call_audit.from)
    assert_equal("my queue", call_audit.queue_uri)
    assert_equal("my agent", call_audit.agent_uri)
    assert_equal(States::Call::TERMINATE, call_audit.state)
    assert_equal(Time.at(1100), call_audit.enter_time)    
    assert_equal(Time.at(1300), call_audit.pick_up_time)    
    assert_equal(Time.at(1400), call_audit.terminate_time)    
  end    
  
end
