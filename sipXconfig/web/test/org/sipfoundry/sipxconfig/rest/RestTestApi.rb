require 'test/unit'

require 'rubygems'
require 'rest_client'

class RestServiceTestApi < Test::Unit::TestCase
  ADMIN = 'dummyAdminUserNameForTestingOnly'
  PASS = 'pass'
  API_URL_ADMIN = "http://#{ADMIN}:#{PASS}@localhost:9999/sipxconfig/rest"
  API_URL_USER = "http://restuser:1235@localhost:9999/sipxconfig/rest/my"

  def setup
    #RestClient.log='stderr'
  end

  def test_get_phonebooks_xml
    phonebooks = RestClient.get(API_URL_ADMIN + '/phonebook', :accept=>'text/xml')
    assert_not_nil(phonebooks)
  end

  def test_get_phonebook_csv
    phonebook = RestClient.get(API_URL_ADMIN + '/phonebook/sales', :accept=>'text/comma-separated-values')
    assert_not_nil(phonebook)
  end

  def test_get_phonebook_xml
    phonebook = RestClient.get(API_URL_ADMIN + '/phonebook/sales', :accept=>'text/xml')
    assert_not_nil(phonebook)
  end

  def test_change_pin
    # change PIN: 1235 -> 4321
    uri1 = API_URL_USER + '/voicemail/pin/4321'
    RestClient.put(uri1, '+')
    # change PIN back: 4321 -> 1235
    uri2 = 'http://restuser:4321@localhost:9999/sipxconfig/rest/my/voicemail/pin/1235'
    RestClient.put(uri2, '+')
  end

  def test_forwarding_xml
    call_sequence_xml = <<-XML
    <call-sequence>
      <rings>
        <ring>
          <expiration>30</expiration>
          <type>If no response</type>
          <position>0</position>
          <enabled>true</enabled>
          <number>200</number>
        </ring>
        <ring>
          <expiration>30</expiration>
          <type>At the same time</type>
          <position>1</position>
          <enabled>true</enabled>
          <number>201</number>
        </ring>
      </rings>
      <withVoicemail>false</withVoicemail>
    </call-sequence>
    XML

    uri = API_URL_USER + '/forward'
    result = RestClient.put(uri, call_sequence_xml, :content_type=>'text/xml')
    print "After put:\n", result, "\n"

    result = RestClient.get(uri, :accept=>'text/xml')
    print "After get:\n", result, "\n"
    assert_equal call_sequence_xml.gsub(/\s+/, ''), result.gsub(/\s+/, '')
  end

  def test_forwarding_json
    call_sequence_json = <<-JSON
    {"call-sequence": {
      "rings": [
        {
          "expiration": 30,
          "type": "If no response",
          "position": 0,
          "enabled": true,
          "number": "200"
        },
        {
          "expiration": 15,
          "type": "At the same time",
          "position": 1,
          "enabled": false,
          "number": "201"
        }
      ],
      "withVoicemail": false
    }}
    JSON
    uri = API_URL_USER + '/forward'

    uri = API_URL_USER + '/forward'
    result = RestClient.put(uri, call_sequence_json, :content_type=>'application/json')
    assert_not_nil result
    print "After put:\n", result, "\n"

    result = RestClient.get(uri, :accept=>'application/json')
    print "After get:\n", result, "\n"
    assert_equal call_sequence_json.gsub(/\s+/, ''), result.gsub(/\s+/, '')
  end
end
