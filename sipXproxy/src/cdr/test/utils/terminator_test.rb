#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'test/unit'

$:.unshift File.join(File.dirname(__FILE__), '..', '..', 'lib')

require 'utils/terminator'

class TerminatorTest < Test::Unit::TestCase
  def test_stop
    t = Terminator.new(30)
    t.stop()
    assert(t.wait)
  end


  def test_timeout
    term = Terminator.new(0.3)
    wait_result = nil
    test_thread = Thread.new(term) do |t| 
      wait_result = t.wait()
    end
    test_thread.join()
    assert_not_nil(wait_result)
    assert(!wait_result)
  end
  
  def test_stop_while_waiting
    term = Terminator.new(0.5)
    wait_result = nil
    test_thread = Thread.new(term) do |t| 
      # wait wi
      wait_result = t.wait()
    end
    sleep(0.1)
    term.stop()
    test_thread.join()
    assert_not_nil(wait_result)
    assert(wait_result)
  end
end
