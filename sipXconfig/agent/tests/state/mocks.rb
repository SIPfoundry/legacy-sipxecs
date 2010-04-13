module Mocks

  def Mocks.state_mock
    mock = Object.new    
    class <<mock
      def queue(queue_uri); end      
      def agent(agent_uri); end
      def audit_call(call, now); end
      def audit_agent(agent, queue_uri, queue_info, now); end
    end
    mock    
  end

end