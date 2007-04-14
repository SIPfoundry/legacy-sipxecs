#!/usr/bin/env ruby

# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.

$:.unshift File.join(File.dirname(__FILE__), "..")
require 'test/unit'

load 'sipxproc'

module MockProcess 
  Response = Struct.new("Response", :body, :value)
  
  def mock(test)
    @test = test
    @expected = []
  end

  def expect_and_return(*data)
    @expected << data
  end
  
  def get_url(url)
    expected = @expected.shift
    @test.assert_equal(expected[0], url)
    fname = File.join(File.dirname(__FILE__), expected[1])
    File.open(fname, 'r') {|file|
      response = Response.new(file.read, nil)
      yield response
    }
  end
  
  def puts(output)
    expected = @expected.shift
    @test.assert_equal(expected[0], output)
  end

  def verify
    @test.assert_equal(0, @expected.length)
  end
end

class SipxProcessTest < Test::Unit::TestCase
    
  def test_status
    setup = SipxProcess.new
    setup.extend MockProcess
    setup.mock(self)
    setup.expect_and_return("/cgi-bin/processmonitor/process.cgi?command=status", "sipxproc_status_output.xml")
    setup.expect_and_return("SIPRegistrar Started")
    setup.expect_and_return("ParkServer Failed")
    setup.expect_and_return("SIPAuthProxy Started")
    setup.expect_and_return("ConfigServer Started")
    setup.process_status
    setup.verify
  end
  
  def test_command
    setup = SipxProcess.new
    setup.extend MockProcess
    setup.mock(self)
    setup.expect_and_return("/cgi-bin/processmonitor/process.cgi?command=restart&process=banana", "sipxproc_empty.txt")
    setup.expect_and_return("/cgi-bin/processmonitor/process.cgi?command=restart&process=apple", "sipxproc_empty.txt")
    setup.process_command('restart', "banana", "apple")
    setup.verify
  end
  
  def test_individual_status
    setup = SipxProcess.new
    setup.extend MockProcess
    setup.mock(self)
    setup.expect_and_return("/cgi-bin/processmonitor/process.cgi?command=status&process=banana", "sipxproc_empty.txt")
    setup.expect_and_return("empty")
    setup.process_command('status', "banana")
    setup.verify  
  end

end
