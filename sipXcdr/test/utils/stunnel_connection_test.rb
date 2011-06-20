#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'stringio'
require 'test/unit'

$:.unshift(File.join(File.dirname(__FILE__), '..', '..', 'lib'))

# application requires
require 'utils/stunnel_connection'

class StunnelConnectionTest < Test::Unit::TestCase

  def test_generate_stunnel_config_file
    hostString = 'test.example.com:5433, test2.example.com:5434, localhost'
    c = Configure.new(CallResolverConfigure::CSE_HOSTS => hostString)
    config = CallResolverConfigure.new(c)
    sc = StunnelConnection.new(config)
    
    scf = StringIO.open
    sc.generate_stunnel_config_file(scf)

    scf = StringIO.new(scf.string)
    generated = scf.readlines()

    expected_name = File.join(File.dirname(__FILE__), 'stunnel-config')
    expected = File.open(expected_name).readlines()

    expected.each_index { |i|
      assert_equal(expected[i], generated[i])
    }
  end
  
  def _test_open
    hostString = 'widelo:5433, localhost'
    c = Configure.new(CallResolverConfigure::CSE_HOSTS => hostString,
      'SIP_CALLRESOLVER_LOG_LEVEL' => 'DEBUG')
    config = CallResolverConfigure.new(c)
    sc = StunnelConnection.new(config)

    sc.open
    sleep 10
    sc.close
  end
end
