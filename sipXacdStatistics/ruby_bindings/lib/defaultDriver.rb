require 'default.rb'

require 'soap/rpc/driver'

class AcdStatsService < ::SOAP::RPC::Driver
  DefaultEndpointUrl = "http://localhost:2000/"
  MappingRegistry = ::SOAP::Mapping::Registry.new

  MappingRegistry.set(
    ArrayOfBird,
    ::SOAP::SOAPArray,
    ::SOAP::Mapping::Registry::TypedArrayFactory,
    { :type => XSD::QName.new("urn:StatsService", "Bird") }
  )
  MappingRegistry.set(
    ArrayOfCallStats,
    ::SOAP::SOAPArray,
    ::SOAP::Mapping::Registry::TypedArrayFactory,
    { :type => XSD::QName.new("urn:StatsService", "CallStats") }
  )
  MappingRegistry.set(
    ArrayOfAgentStats,
    ::SOAP::SOAPArray,
    ::SOAP::Mapping::Registry::TypedArrayFactory,
    { :type => XSD::QName.new("urn:StatsService", "AgentStats") }
  )
  MappingRegistry.set(
    ArrayOfQueueStats,
    ::SOAP::SOAPArray,
    ::SOAP::Mapping::Registry::TypedArrayFactory,
    { :type => XSD::QName.new("urn:StatsService", "QueueStats") }
  )
  MappingRegistry.set(
    ArrayOfCallAudit,
    ::SOAP::SOAPArray,
    ::SOAP::Mapping::Registry::TypedArrayFactory,
    { :type => XSD::QName.new("urn:StatsService", "CallAudit") }
  )
  MappingRegistry.set(
    ArrayOfAgentAudit,
    ::SOAP::SOAPArray,
    ::SOAP::Mapping::Registry::TypedArrayFactory,
    { :type => XSD::QName.new("urn:StatsService", "AgentAudit") }
  )
  MappingRegistry.set(
    Bird,
    ::SOAP::SOAPStruct,
    ::SOAP::Mapping::Registry::TypedStructFactory,
    { :type => XSD::QName.new("urn:StatsService", "Bird") }
  )
  MappingRegistry.set(
    CallStats,
    ::SOAP::SOAPStruct,
    ::SOAP::Mapping::Registry::TypedStructFactory,
    { :type => XSD::QName.new("urn:StatsService", "CallStats") }
  )
  MappingRegistry.set(
    AgentStats,
    ::SOAP::SOAPStruct,
    ::SOAP::Mapping::Registry::TypedStructFactory,
    { :type => XSD::QName.new("urn:StatsService", "AgentStats") }
  )
  MappingRegistry.set(
    QueueStats,
    ::SOAP::SOAPStruct,
    ::SOAP::Mapping::Registry::TypedStructFactory,
    { :type => XSD::QName.new("urn:StatsService", "QueueStats") }
  )
  MappingRegistry.set(
    CallAudit,
    ::SOAP::SOAPStruct,
    ::SOAP::Mapping::Registry::TypedStructFactory,
    { :type => XSD::QName.new("urn:StatsService", "CallAudit") }
  )
  MappingRegistry.set(
    AgentAudit,
    ::SOAP::SOAPStruct,
    ::SOAP::Mapping::Registry::TypedStructFactory,
    { :type => XSD::QName.new("urn:StatsService", "AgentAudit") }
  )

  Methods = [
    [ XSD::QName.new("urn:StatsService", "getBirdArray"),
      "urn:StatsService#getBirdArray",
      "getBirdArray",
      [ ["retval", "getBirdArrayResponse", ["Bird[]", "urn:StatsService", "Bird"]] ],
      { :request_style =>  :rpc, :request_use =>  :encoded,
        :response_style => :rpc, :response_use => :encoded }
    ],
    [ XSD::QName.new("urn:StatsService", "getCallStats"),
      "urn:StatsService#getCallStats",
      "getCallStats",
      [ ["retval", "getCallStatsResponse", ["CallStats[]", "urn:StatsService", "CallStats"]] ],
      { :request_style =>  :rpc, :request_use =>  :encoded,
        :response_style => :rpc, :response_use => :encoded }
    ],
    [ XSD::QName.new("urn:StatsService", "getAgentStats"),
      "urn:StatsService#getAgentStats",
      "getAgentStats",
      [ ["retval", "getAgentStatsResponse", ["AgentStats[]", "urn:StatsService", "AgentStats"]] ],
      { :request_style =>  :rpc, :request_use =>  :encoded,
        :response_style => :rpc, :response_use => :encoded }
    ],
    [ XSD::QName.new("urn:StatsService", "getQueueStats"),
      "urn:StatsService#getQueueStats",
      "getQueueStats",
      [ ["retval", "getQueueStatsResponse", ["QueueStats[]", "urn:StatsService", "QueueStats"]] ],
      { :request_style =>  :rpc, :request_use =>  :encoded,
        :response_style => :rpc, :response_use => :encoded }
    ],
    [ XSD::QName.new("urn:StatsService", "getCallHistory"),
      "urn:StatsService#getCallHistory",
      "getCallHistory",
      [ ["in", "fromTime", ["::SOAP::SOAPDateTime"]],
        ["retval", "callHistory", ["CallAudit[]", "urn:StatsService", "CallAudit"]] ],
      { :request_style =>  :rpc, :request_use =>  :encoded,
        :response_style => :rpc, :response_use => :encoded }
    ],
    [ XSD::QName.new("urn:StatsService", "getAgentHistory"),
      "urn:StatsService#getCallHistory",
      "getAgentHistory",
      [ ["in", "fromTime", ["::SOAP::SOAPDateTime"]],
        ["retval", "agentHistory", ["AgentAudit[]", "urn:StatsService", "AgentAudit"]] ],
      { :request_style =>  :rpc, :request_use =>  :encoded,
        :response_style => :rpc, :response_use => :encoded }
    ]
  ]

  def initialize(endpoint_url = nil)
    endpoint_url ||= DefaultEndpointUrl
    super(endpoint_url, nil)
    self.mapping_registry = MappingRegistry
    init_methods
  end

private

  def init_methods
    Methods.each do |definitions|
      opt = definitions.last
      if opt[:request_style] == :document
        add_document_operation(*definitions)
      else
        add_rpc_operation(*definitions)
        qname = definitions[0]
        name = definitions[2]
        if qname.name != name and qname.name.capitalize == name.capitalize
          ::SOAP::Mapping.define_singleton_method(self, qname.name) do |*arg|
            __send__(name, *arg)
          end
        end
      end
    end
  end
end

