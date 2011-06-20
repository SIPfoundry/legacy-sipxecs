#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'test/unit'

$:.unshift(File.join(File.dirname(__FILE__), '..', '..', 'lib'))

require 'db/database_url'


class DatabaseUrlTest < Test::Unit::TestCase
  
  def test_empty_contruction
    begin
      DatabaseUrl.new({})
      fail "exception expected"
    rescue ArgumentError
    end 
  end

  def test_defaults
    url = DatabaseUrl.new(:username => "anyone")    
    assert_equal('SIPXCDR', url.database)
    assert_equal(5432, url.port)
    assert_equal('localhost', url.host)
    assert_equal('postgresql', url.adapter)
    assert_equal('anyone', url.username)    
  end
  
  def test_equality
    data = {:database => 'a', :port => 1024, :host => 'host', :username => 'abc' }
    
    u1 = DatabaseUrl.new(data)
    u2 = DatabaseUrl.new(data)
    
    data[:database] = 'z'    
    
    u3 = DatabaseUrl.new(data)
    
    data[:database] = nil
    
    u4 = DatabaseUrl.new(data)
    
    data[:database] = 'a'
    data[:username] = 'z'
    
    u5 = DatabaseUrl.new(data) 
    
    assert_equal(u1, u2)
    assert(u1.eql?(u2))
    assert_not_equal(u1, u3)
    assert_not_equal(u1, u4)
    assert_not_equal(u1, u5)
    assert_not_equal(u2, u5)
    assert_not_equal(u4, u3)
  end
  
  def test_to_dbi
    url = DatabaseUrl.new(:database => 'SIPXCDR', :host => 'localhost', :username => 'anyone')
    assert_equal("dbi:Pg:database=SIPXCDR;host=localhost;port=5432", url.to_dbi)
  end
end
