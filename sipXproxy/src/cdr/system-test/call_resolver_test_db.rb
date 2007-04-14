#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'test/unit'


$:.unshift(File.join(File.dirname(__FILE__), '..', 'lib'))

require 'call_resolver'
require 'utils/call_resolver_configure'

class CallResolverTest < Test::Unit::TestCase
  def setup
    @config = CallResolverConfigure.from_file()
  end

  
  def test_resolve()
    resolver = CallResolver.new(@config)
    
    #resolver.resolve(nil, Time.parse("10/26"))
    resolver.resolve(nil, nil)
  end  
  
  def _test_daily
    resolver = CallResolver.new(@config)
    resolver.run_resolver
  end
end
