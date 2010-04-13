#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

module TestUtil
  
  class DummyQueue
    attr_reader :counter, :last
    
    def initialize()
      @counter = 0
    end
    
    def <<(x)
      @last = x 
      @counter += 1
    end
    
    alias enq <<    
  end
  
  class NullLog
    def error(*args)
    end
    
    def debug(*args)
    end
  end
  
  # creates simple mocks from klass and argument hash
  def TestUtil.create_mock(klass, args)
    o = klass.new
    
    if(args)
      args.each { |field, value| 
        setter = (field.to_s + "=").to_sym
        o.send( setter, value ) 
      }
    end    
    return o
  end
    
end
