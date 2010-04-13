#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'test/unit'

$:.unshift File.join(File.dirname(__FILE__), '..', '..', 'lib')

require 'utils/utils'
require 'utils/exceptions'


class UtilsTest < Test::Unit::TestCase

  def test_get_get_aor_from_header

    # test get_aor_from_header with a good input
    header = 'From: Alice <sip:alice@atlanta.com>;tag=1928301774'
    assert_equal('From: Alice <sip:alice@atlanta.com>',
                 Utils.get_aor_from_header(header))
                 
    header = 'From: \"Alice:in;Wonder;land\" <sip:alice@atlanta.com>;tag=1928301774'
    assert_equal('From: \"Alice:in;Wonder;land\" <sip:alice@atlanta.com>',
                 Utils.get_aor_from_header(header))                 
    
    # another good input
    header = '<sip:100@pingtel.com;user=phone>;tag=5551212'
    assert_equal('<sip:100@pingtel.com;user=phone>', Utils.get_aor_from_header(header))
    
    # Test get_aor_from_header with a bad input: there is no tag.
    # Must raise an exception.
    assert_raise(BadSipHeaderException) do
      Utils.get_aor_from_header('From: Alice <sip:alice@atlanta.com>')
    end
    
    # Test get_aor_from_header with a bad input: there is no tag.
    # Must not raise an exception because we tell it that a tag is not required.
    assert_nothing_thrown do
      Utils.get_aor_from_header('From: Alice <sip:alice@atlanta.com>', false)
    end
  end
  
  def test_contact_host
    ip_contacts = ["\"Jorma; \\\"The sip: Man\\\" Kaukonen\"<sip:187@10.1.1.170:1234>;tag=\"1c32681\"",
                   "\"Jorma:Kaukonen\"sip:187@10.1.1.170;tag=1c32681",
                   "Joe <sip:187@10.1.1.170:1234>",                   
                   "<sip:187@10.1.1.170:1234>",
                   "sip:187@10.1.1.170:1234",
                   "sip:187@10.1.1.170",
                   "sip:10.1.1.170"
                   ]
    ip_contacts.each do |contact|
      assert_equal("10.1.1.170", Utils.contact_host(contact))
    end
      
    name_contacts = ["\"Jorma%20Kaukonen\"<sip:187@test-example.com:1234>;tag=1c32681",
                     "\"Jorma \\\"The Man\\\" Kaukonen\"<sip:187@test-example.com>;tag=1c32681",
                     "\"Jorma;:/,Kaukonen\"sip:187@test-example.com;tag=1c32681"
                    ]
    name_contacts.each do |contact|
      assert_equal("test-example.com", Utils.contact_host(contact))                       
    end
    assert_equal("test.exampl-1.exa2mple.com", Utils.contact_host("\"Joe\'s Place\" <sips:345@test.exampl-1.exa2mple.com:34>"))
  end

  def test_remove_part_of_str_beginning_with_char
    str = "a.b.c"
    assert_equal("a", Utils.remove_part_of_str_beginning_with_char(str, "."))
    assert_equal(str, Utils.remove_part_of_str_beginning_with_char(str, "^"))    
  end

  def test_contact_without_params
    contact_with_no_params = '<sip:100@10.1.20.3:5100>'
    assert_equal(contact_with_no_params, Utils.contact_without_params(contact_with_no_params))
    
    contact_with_one_long_param = '<sip:100@10.1.20.3:5100;play=https%3A%2F%2Flocalhost%3A8091%2Fcgi-bin%2Fvoicemail%2Fmediaserver.cgi%3Faction%3Dautoattendant%26name%3Daa_41>'
    assert_equal('<sip:100@10.1.20.3:5100>', Utils.contact_without_params(contact_with_one_long_param))
    
    contact_with_lots_of_params = '<sip:alice@example.com;one;more;for;you;nineteen;for;me>'
    assert_equal('<sip:alice@example.com>', Utils.contact_without_params(contact_with_lots_of_params))
    
    contact_with_no_greater_than_sign_at_end = 'sip:alice@example.com;one;more;for;you;nineteen;for;me'
    assert_equal('sip:alice@example.com', Utils.contact_without_params(contact_with_no_greater_than_sign_at_end))
  end

end
