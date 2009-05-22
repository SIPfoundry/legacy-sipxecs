require 'test/unit'

require 'rubygems'
require 'rest_client'

class RestServiceTestApi < Test::Unit::TestCase
  USER = 'dummyAdminUserNameForTestingOnly'
  PASS = 'pass'
  API_URL = "http://#{USER}:#{PASS}@localhost:9999/sipxconfig/rest"

  def setup
    RestClient.log='stderr'
  end

  def test_get_phonebooks_xml
    phonebooks = RestClient.get(API_URL + '/phonebook', :accept=>'text/xml')
    p phonebooks
    assert_not_nil(phonebooks)
  end

  def test_get_phonebook_csv
    phonebook = RestClient.get(API_URL + '/phonebook/sales', :accept=>'text/comma-separated-values')
    assert_not_nil(phonebook)
  end

  def test_get_phonebook_xml
    phonebook = RestClient.get(API_URL + '/phonebook/sales', :accept=>'text/xml')
    p phonebook
    assert_not_nil(phonebook)
  end
end