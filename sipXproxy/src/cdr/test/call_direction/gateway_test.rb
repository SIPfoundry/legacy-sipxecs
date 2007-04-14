#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'test/unit'

# set up the load path
$:.unshift(File.join(File.dirname(__FILE__), '..', '..', 'lib'))


require 'call_direction/gateway'

class GatewayTest < Test::Unit::TestCase
  
  def test_load_gateways    
    # Load it
    gateways = Gateway.find_all
    
    # FIXME add some test to it
    # assert(gateways.size > 0)
    # puts gateways[0].ip_addresses
  end
end
