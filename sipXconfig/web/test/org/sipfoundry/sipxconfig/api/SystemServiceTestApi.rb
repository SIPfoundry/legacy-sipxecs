require 'test/unit'
require 'ConfigServiceDriver.rb'
require 'ApiTestCase.rb'

class SystemServiceTestApi < ApiTestCase
  
  def setup
    super
  
    # create the services that we will use
    @system_service = service(SystemService)
    @test_service = service(TestService)
    
    # reset the user service so we start the test with no users
    reset = ResetServices.new();
    reset.user = true;
    @test_service.resetServices(reset)
  end
  
  def test_getDomain	    
    system_info = @system_service.systemInfo()
    assert_not_nil(system_info.domain.name)
    assert_not_nil(system_info.domain.realm)
  end
end