require 'test/unit'

require 'rubygems'
require 'rest_client'

class RestServiceTestApi < Test::Unit::TestCase
  ADMIN = 'dummyAdminUserNameForTestingOnly'
  PASS = 'pass'
  API_URL_ADMIN = "http://#{ADMIN}:#{PASS}@localhost:9999/sipxconfig/rest"

  def test_add_phones_xml
    new_phone_xml = <<-XML
    <phones>
      <phone>
        <serialNumber>000000c0ffee</serialNumber>
        <model>kphoneStandard</model>"
        <description>Phone 1 - YES</description>
      </phone>
      <phone>
        <serialNumber>0004f21ed0d3</serialNumber>
        <model>noSuchPhoneModel</model>"
        <description>NO - invalid model</description>
      </phone>
      <phone>
        <serialNumber>111000c0ffee</serialNumber>
        <model>kphoneStandard</model>"
        <description>Phone 2 - YES</description>
      </phone>
    </phones>
    XML
    phones = RestClient.post(API_URL_ADMIN + '/phone', new_phone_xml)
    assert_not_nil(phones)
  end
end
