require 'test/unit'

require 'rubygems'
require 'rest_client'

class RestServiceTestApi < Test::Unit::TestCase
  USER = 'dummyAdminUserNameForTestingOnly'
  PASS = 'pass'
  API_URL = "http://#{USER}:#{PASS}@localhost:9999/sipxconfig/rest"

  def setup
    #RestClient.log='stderr'
  end

  def test_get_phonebooks_xml
    phonebooks = RestClient.get(API_URL + '/phonebook', :accept=>'text/xml')
    assert_not_nil(phonebooks)
  end

  def test_get_phonebook_csv
    phonebook = RestClient.get(API_URL + '/phonebook/sales', :accept=>'text/comma-separated-values')
    assert_not_nil(phonebook)
  end

  def test_get_phonebook_xml
    phonebook = RestClient.get(API_URL + '/phonebook/sales', :accept=>'text/xml')
    assert_not_nil(phonebook)
  end

  def test_change_pin
    # change PIN: 1235 -> 4321
    uri1 = "http://userpintest:1235@localhost:9999/sipxconfig/rest/voicemail/pin/4321"
    RestClient.put(uri1, '+')
    # change PIN back: 4321 -> 1235
    uri2 = "http://userpintest:4321@localhost:9999/sipxconfig/rest/voicemail/pin/1235"
    RestClient.put(uri2, '+')
  end
end