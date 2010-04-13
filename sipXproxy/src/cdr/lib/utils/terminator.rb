#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'monitor'


class Terminator < Monitor

  def initialize(timeout)
    super()
    @timeout = timeout
    @cond = new_cond()
    @stop = false
  end

  # returns true if stopped was called, returns false it there was a timeout
  def wait
    synchronize do
      return true if @stop
      return @cond.wait(@timeout)
    end
  end

  def stop
    synchronize do
      @stop = true
      @cond.signal()
    end
  end
end
