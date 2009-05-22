require 'test/unit'

require 'rubygems'
require 'rest_client'

class RestServiceTestApi < Test::Unit::TestCase
  USER = 'dummyAdminUserNameForTestingOnly'
  PASS = 'pass'
  API_URL = "http://#{USER}:#{PASS}@localhost:9999/sipxconfig/rest"

  def setup
    super
  end

  def test_phoneBookService
    RestClient.log='stderr'
    phonebook = RestClient.get(API_URL + '/phonebook/sales', :accept=>'text/comma-separated-values')
    assert_not_nil(phonebook)
  end
end