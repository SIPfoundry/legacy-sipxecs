require 'test/unit'
require 'ConfigServiceDriver.rb'
require 'ApiTestCase.rb'

class DeviceDiscoveryServiceTestApi < ApiTestCase

  def setup
    super

    # create the services that we will use
    @discovery_service = service(DeviceDiscoveryService)
    @test_service = service(TestService)

    reset = ResetServices.new();
    reset.discovery = true;
    @test_service.resetServices(reset)

  end

  def test_addRetrieveDevice
    expected = DiscoveredDevice.new();
    expected.macAddress = '000011112222';
    expected.ipAddress = '100.100.100.100';
    expected.vendor = 'testVendor';

    expectedList = ArrayOfDiscoveredDevices.new([expected]);
    @discovery_service.updateDiscoveredDevicesList(UpdateDiscoveredDevicesList.new(expectedList));

    actualList = @discovery_service.retrieveDiscoveredDevicesList();
    actualArray = actualList.retrievedDevices;
    assert_equal(1,actualArray.length);
    actual = actualArray[0];
    assert_equal('000011112222',actual.macAddress);
    assert_equal('100.100.100.100',actual.ipAddress);
    assert_equal('testVendor',actual.vendor);

  end

end
