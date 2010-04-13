require 'test/unit'
require 'ConfigServiceDriver.rb'
require 'ApiTestCase.rb'

# Extend the ParkOrbit by overriding the equality operator so we can check equality by value
class ParkOrbit
  def ==(other)
   (self.name == other.name) &&
    (self.extension == other.extension) &&
     (self.description == other.description) &&
      (self.enabled == other.enabled) &&
       (self.music == other.music)
  end    
end

class ParkOrbitServiceTestApi < ApiTestCase  
  def setup
    super
    
    # create the services that we will use
    @park_orbit_service = service(ParkOrbitService)
    @test_service = service(TestService)
    
    reset = ResetServices.new();
    reset.parkOrbit = true;
    @test_service.resetServices(reset)
  end
  
  def test_createAndGetParkOrbits
    # Create two ParkOrbits
    # ParkOrbit properties are name, extension, description, enabled, music
    expected1 = ParkOrbit.new('orbit1', 'orbit ext1', 'test orbit1', true, 'love supreme');
    @park_orbit_service.addParkOrbit(AddParkOrbit.new(expected1))
    expected2 = ParkOrbit.new('orbit2', 'orbit ext2', 'test orbit2', true, 'walk like a tunisian');
    @park_orbit_service.addParkOrbit(AddParkOrbit.new(expected2))
    
    # Get ParkOrbits and verify that they are right
    getParkOrbitsResponse = @park_orbit_service.getParkOrbits()
    parkOrbits = getParkOrbitsResponse.parkOrbits
    assert_equal(2, parkOrbits.length)
    assert_equal(expected1, parkOrbits[0])	    
    assert_equal(expected2, parkOrbits[1])	    
  end
end
