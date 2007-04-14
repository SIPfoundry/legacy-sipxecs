#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'test/unit'

$:.unshift(File.join(File.dirname(__FILE__), '..', '..', 'lib'))

require 'db/database_url'
require 'db/cse_reader'

class CseReaderTest < Test::Unit::TestCase
  
  class MockQueue
    attr_reader :i
    
    def initialize
      @i = 0
    end
    
    def enq(cse)    
      if @i % 500 == 0
        p cse.event_time
      end
      @i += 1
    end
    
    alias :<< :enq
  end
  
  # this one connects to real database for now
  def test_DB
    url = DatabaseUrl.new()
    p url
    q = MockQueue.new
    reader = CseReader.new(url)
    reader.run(q, nil, nil)
    p q.i    
  end
end
