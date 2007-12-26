#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'test/unit'

$:.unshift File.join(File.dirname(__FILE__), '..', '..', 'lib')

require 'utils/cleaner'

class CleanerTest < Test::Unit::TestCase
  def test_missing
    queue = []
    cl = Cleaner.new(0.3, [:retire_long_calls, 8000])
    cl_thread = Thread.new(queue) do |q|
      cl.run(q)
    end
    sleep(1)
    cl.stop
    cl_thread.join
    assert(queue.size > 0) 
    item = queue[0]
    assert(item.kind_of?(Array))
    assert_equal(:retire_long_calls, item[0])
    assert_equal(8000, item[1])  
  end
end

