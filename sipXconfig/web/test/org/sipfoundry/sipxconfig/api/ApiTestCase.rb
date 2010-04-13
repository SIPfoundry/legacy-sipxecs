class ApiTestCase < Test::Unit::TestCase
  SERVICE_ROOT = 'http://localhost:9999/sipxconfig/services/'

  DUMMY_USER = "dummyAdminUserNameForTestingOnly"
  DUMMY_PASSWORD = "dummyAdminUserPasswordCanBeAnything"

  def setup
    super
  
    # Hash of SOAP services, indexed by class
    @services = Hash.new
  end

  # Return the service for the given service_class, creating it on demand.
  # Note that the name of the service class is also the last part of the service URL.
  def service(service_class)
    # return the named service if we have already created it
    service = @services[service_class]
    return service if service
    
    # doesn't exist yet, so create it
    service_url = SERVICE_ROOT + service_class.name
    @services[service_class] = service = service_class.new(service_url)
    
    # when debugging, dump service messages to stdout
	service.wiredump_dev = STDOUT if $DEBUG
	
	# set up HTTP basic authentication
    service.options["protocol.http.basic_auth"] << [service_url, DUMMY_USER, DUMMY_PASSWORD]
    
    service
  end
  
  # Every subclass of TestCase must have at least one test
  def test_nothing
  end
end
