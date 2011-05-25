# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.

module States  
    
  # Base class for all _State_ objects.
  # Provides for event routing: states can implement _accept_EventType_ methods that will be
  # automatically called when _EventType_ event (or one of its sublasses) is passed to accept method.
  class State
    def accept(event)
      klass = event.class
      while klass != Object
        # strip module names from class
        class_name = klass.to_s.gsub(/.+::/, '')
        method_name = "accept_#{class_name}"
        if respond_to?(method_name)
          send(method_name, event)
          break
        end
        klass = klass.superclass
      end
    end
  end
  
  # Calculates the relative duration - now is always provided.
  # _start_ and _stop_ are not +nil+ if respectively start and stop happened already
  def States.duration(start, stop, now)
    case 
    when start && stop: stop - start   # we already completed
    when start: now - start   # we are in the process
    else 0  # did not even started
    end  
  end  
  
end
