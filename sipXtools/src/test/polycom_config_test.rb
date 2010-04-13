#!/usr/bin/env ruby

# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.

DIR = File.dirname(__FILE__)
$:.unshift File.join(DIR, "..")
require 'test/unit'
load 'polycom-config'

class PolycomConfigTest < Test::Unit::TestCase

  def test_multiarg
    config = PolycomConfig.new()
    config.flatten("#{DIR}/polycom-config.cfg", "#{DIR}/polycom-config.cfg")
  end

  def test_flatten
    config = PolycomConfig.new()
    def config.puts(msg)
      @console = '' if ! defined? @console
      @console += msg
      @console += "\n"
    end
    def config.console
      @console
    end
    config.flatten(File.join(DIR, "polycom-config.cfg"))
    expected = <<EXPECTED
/phone1/reg/reg.1.displayName=bluejay
/phone1/reg/reg.1.type=private
/phone1/reg/reg.1.address=bluejay@example.com
/phone1/reg/reg.1.label=BlueJay
/phone1/call/donotdisturb/call.donotdisturb.perReg=0
EXPECTED
   assert_equal expected, config.console
  end
  
end
