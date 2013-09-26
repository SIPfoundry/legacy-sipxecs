require 'xsd/qname'

# {urn:StatsService}ArrayOfBird
class ArrayOfBird < ::Array
  @@schema_type = "Bird"
  @@schema_ns = "urn:StatsService"
  @@schema_element = [["item", ["Bird", XSD::QName.new(nil, "item")]]]
end

# {urn:StatsService}Bird
class Bird
  @@schema_type = "Bird"
  @@schema_ns = "urn:StatsService"
  @@schema_element = [["species", ["SOAP::SOAPString", XSD::QName.new(nil, "species")]]]

  attr_accessor :species

  def initialize(species = nil)
    @species = species
  end
end

# {urn:StatsService}ArrayOfString
class ArrayOfString < ::Array
  @@schema_type = "string"
  @@schema_ns = "http://www.w3.org/2001/XMLSchema"
  @@schema_element = [["item", ["String", XSD::QName.new(nil, "item")]]]
end

# {urn:StatsService}ArrayOfCallStats
class ArrayOfCallStats < ::Array
  @@schema_type = "CallStats"
  @@schema_ns = "urn:StatsService"
  @@schema_element = [["item", ["CallStats", XSD::QName.new(nil, "item")]]]
end

# {urn:StatsService}CallStats
class CallStats
  @@schema_type = "CallStats"
  @@schema_ns = "urn:StatsService"
  @@schema_element = [["from", ["SOAP::SOAPString", XSD::QName.new(nil, "from")]], ["state", ["SOAP::SOAPString", XSD::QName.new(nil, "state")]], ["agent_uri", ["SOAP::SOAPString", XSD::QName.new(nil, "agent_uri")]], ["queue_uri", ["SOAP::SOAPString", XSD::QName.new(nil, "queue_uri")]], ["queue_wait_time", ["SOAP::SOAPLong", XSD::QName.new(nil, "queue_wait_time")]], ["total_wait_time", ["SOAP::SOAPLong", XSD::QName.new(nil, "total_wait_time")]], ["processing_time", ["SOAP::SOAPLong", XSD::QName.new(nil, "processing_time")]]]

  attr_accessor :from
  attr_accessor :state
  attr_accessor :agent_uri
  attr_accessor :queue_uri
  attr_accessor :queue_wait_time
  attr_accessor :total_wait_time
  attr_accessor :processing_time

  def initialize(from = nil, state = nil, agent_uri = nil, queue_uri = nil, queue_wait_time = nil, total_wait_time = nil, processing_time = nil)
    @from = from
    @state = state
    @agent_uri = agent_uri
    @queue_uri = queue_uri
    @queue_wait_time = queue_wait_time
    @total_wait_time = total_wait_time
    @processing_time = processing_time
  end
end

# {urn:StatsService}ArrayOfAgentStats
class ArrayOfAgentStats < ::Array
  @@schema_type = "AgentStats"
  @@schema_ns = "urn:StatsService"
  @@schema_element = [["item", ["AgentStats", XSD::QName.new(nil, "item")]]]
end

# {urn:StatsService}AgentStats
class AgentStats
  @@schema_type = "AgentStats"
  @@schema_ns = "urn:StatsService"
  @@schema_element = [["agent_uri", ["SOAP::SOAPString", XSD::QName.new(nil, "agent_uri")]], ["state", ["SOAP::SOAPString", XSD::QName.new(nil, "state")]], ["current_state_time", ["SOAP::SOAPLong", XSD::QName.new(nil, "current_state_time")]], ["queues", ["ArrayOfString", XSD::QName.new(nil, "queues")]]]

  attr_accessor :agent_uri
  attr_accessor :state
  attr_accessor :current_state_time
  attr_accessor :queues

  def initialize(agent_uri = nil, state = nil, current_state_time = nil, queues = nil)
    @agent_uri = agent_uri
    @state = state
    @current_state_time = current_state_time
    @queues = queues
  end
end

# {urn:StatsService}ArrayOfQueueStats
class ArrayOfQueueStats < ::Array
  @@schema_type = "QueueStats"
  @@schema_ns = "urn:StatsService"
  @@schema_element = [["item", ["QueueStats", XSD::QName.new(nil, "item")]]]
end

# {urn:StatsService}QueueStats
class QueueStats
  @@schema_type = "QueueStats"
  @@schema_ns = "urn:StatsService"
  @@schema_element = [["queue_uri", ["SOAP::SOAPString", XSD::QName.new(nil, "queue_uri")]], ["waiting_calls", ["SOAP::SOAPLong", XSD::QName.new(nil, "waiting_calls")]], ["avg_wait_time", ["SOAP::SOAPLong", XSD::QName.new(nil, "avg_wait_time")]], ["max_wait_time", ["SOAP::SOAPLong", XSD::QName.new(nil, "max_wait_time")]], ["abandoned_calls", ["SOAP::SOAPLong", XSD::QName.new(nil, "abandoned_calls")]], ["avg_abandoned_time", ["SOAP::SOAPLong", XSD::QName.new(nil, "avg_abandoned_time")]], ["max_abandoned_time", ["SOAP::SOAPLong", XSD::QName.new(nil, "max_abandoned_time")]], ["processed_calls", ["SOAP::SOAPLong", XSD::QName.new(nil, "processed_calls")]], ["avg_processing_time", ["SOAP::SOAPLong", XSD::QName.new(nil, "avg_processing_time")]], ["max_processing_time", ["SOAP::SOAPLong", XSD::QName.new(nil, "max_processing_time")]], ["idle_agents", ["SOAP::SOAPInt", XSD::QName.new(nil, "idle_agents")]], ["busy_agents", ["SOAP::SOAPInt", XSD::QName.new(nil, "busy_agents")]]]

  attr_accessor :queue_uri
  attr_accessor :waiting_calls
  attr_accessor :avg_wait_time
  attr_accessor :max_wait_time
  attr_accessor :abandoned_calls
  attr_accessor :avg_abandoned_time
  attr_accessor :max_abandoned_time
  attr_accessor :processed_calls
  attr_accessor :avg_processing_time
  attr_accessor :max_processing_time
  attr_accessor :idle_agents
  attr_accessor :busy_agents

  def initialize(queue_uri = nil, waiting_calls = nil, avg_wait_time = nil, max_wait_time = nil, abandoned_calls = nil, avg_abandoned_time = nil, max_abandoned_time = nil, processed_calls = nil, avg_processing_time = nil, max_processing_time = nil, idle_agents = nil, busy_agents = nil)
    @queue_uri = queue_uri
    @waiting_calls = waiting_calls
    @avg_wait_time = avg_wait_time
    @max_wait_time = max_wait_time
    @abandoned_calls = abandoned_calls
    @avg_abandoned_time = avg_abandoned_time
    @max_abandoned_time = max_abandoned_time
    @processed_calls = processed_calls
    @avg_processing_time = avg_processing_time
    @max_processing_time = max_processing_time
    @idle_agents = idle_agents
    @busy_agents = busy_agents
  end
end

# {urn:StatsService}ArrayOfCallAudit
class ArrayOfCallAudit < ::Array
  @@schema_type = "CallAudit"
  @@schema_ns = "urn:StatsService"
  @@schema_element = [["item", ["CallAudit", XSD::QName.new(nil, "item")]]]
end

# {urn:StatsService}CallAudit
class CallAudit
  @@schema_type = "CallAudit"
  @@schema_ns = "urn:StatsService"
  @@schema_element = [["from", ["SOAP::SOAPString", XSD::QName.new(nil, "from")]], ["state", ["SOAP::SOAPString", XSD::QName.new(nil, "state")]], ["agent_uri", ["SOAP::SOAPString", XSD::QName.new(nil, "agent_uri")]], ["queue_uri", ["SOAP::SOAPString", XSD::QName.new(nil, "queue_uri")]], ["enter_time", ["SOAP::SOAPDateTime", XSD::QName.new(nil, "enter_time")]], ["pick_up_time", ["SOAP::SOAPDateTime", XSD::QName.new(nil, "pick_up_time")]], ["terminate_time", ["SOAP::SOAPDateTime", XSD::QName.new(nil, "terminate_time")]]]

  attr_accessor :from
  attr_accessor :state
  attr_accessor :agent_uri
  attr_accessor :queue_uri
  attr_accessor :enter_time
  attr_accessor :pick_up_time
  attr_accessor :terminate_time

  def initialize(from = nil, state = nil, agent_uri = nil, queue_uri = nil, enter_time = nil, pick_up_time = nil, terminate_time = nil)
    @from = from
    @state = state
    @agent_uri = agent_uri
    @queue_uri = queue_uri
    @enter_time = enter_time
    @pick_up_time = pick_up_time
    @terminate_time = terminate_time
  end
end

# {urn:StatsService}ArrayOfAgentAudit
class ArrayOfAgentAudit < ::Array
  @@schema_type = "AgentAudit"
  @@schema_ns = "urn:StatsService"
  @@schema_element = [["item", ["AgentAudit", XSD::QName.new(nil, "item")]]]
end

# {urn:StatsService}AgentAudit
class AgentAudit
  @@schema_type = "AgentAudit"
  @@schema_ns = "urn:StatsService"
  @@schema_element = [["agent_uri", ["SOAP::SOAPString", XSD::QName.new(nil, "agent_uri")]], ["queue_uri", ["SOAP::SOAPString", XSD::QName.new(nil, "queue_uri")]], ["sign_in_time", ["SOAP::SOAPDateTime", XSD::QName.new(nil, "sign_in_time")]], ["sign_out_time", ["SOAP::SOAPDateTime", XSD::QName.new(nil, "sign_out_time")]]]

  attr_accessor :agent_uri
  attr_accessor :queue_uri
  attr_accessor :sign_in_time
  attr_accessor :sign_out_time

  def initialize(agent_uri = nil, queue_uri = nil, sign_in_time = nil, sign_out_time = nil)
    @agent_uri = agent_uri
    @queue_uri = queue_uri
    @sign_in_time = sign_in_time
    @sign_out_time = sign_out_time
  end
end
