require 'test/unit'
require 'ConfigServiceDriver.rb'
require 'ApiTestCase.rb'

# Extend the ConferenceBridge by overriding the equality operator so we can check equality by value.
# For the purpose of this test, we don't check the serviceUri because it is read-only.  A generic
# equality operator would check that property.
class ConferenceBridge
  def ==(other)
   (self.name == other.name) &&
    (self.description == other.description) &&
     (self.enabled == other.enabled) &&
      (self.host == other.host) &&
       (self.port == other.port) &&
        (self.conferences == other.conferences)
  end    
end

# Extend the Conference by overriding the equality operator so we can check equality by value.
class Conference
  def ==(other)
   (self.name == other.name) &&
    (self.extension == other.extension) &&
     (self.description == other.description) &&
      (self.enabled == other.enabled)
  end    
end

class ConferenceBridgeServiceTestApi < ApiTestCase  
  def setup
    super
    
    # create the services that we will use
    @conference_bridge_service = service(ConferenceBridgeService)
    @test_service = service(TestService)
    
    reset = ResetServices.new();
    reset.conferenceBridge = true;
    @test_service.resetServices(reset)
  end
  
  def test_createAndGetConferenceBridge
    # Create a Conference
    # Conference properties are name, extension, description, enabled
    conf = Conference.new('conf', 'conf ext', 'global cooling conference', true)
  
    # Create a ConferenceBridge.
    # ConferenceBridge properties are name, description, enabled, host, port, serviceUri, conferences.
    # The serviceUri is read-only so don't fill it in.
    bridge = ConferenceBridge.new('brooklyn bridge', 'for sale today', true, 'bigcomputer',
                                  9999, nil, [conf]);
    @conference_bridge_service.addConferenceBridge(AddConferenceBridge.new(bridge))
    
    # Call getConferenceBridges and verify the result
    getConferenceBridgesResponse = @conference_bridge_service.getConferenceBridges()
    conferenceBridges = getConferenceBridgesResponse.conferenceBridges
    assert_equal(1, conferenceBridges.length)
    assert_equal(1, conferenceBridges[0].conferences.length)
    assert_equal(bridge, conferenceBridges[0])	    
  end
end
