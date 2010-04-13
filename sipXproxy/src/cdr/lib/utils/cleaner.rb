#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'utils/terminator'


# inserts special house keeping events to state queue
# at the moment the only even it sends it 'retire_long_calls'
class Cleaner
  
  def initialize(interval, action)
    @terminator = Terminator.new(interval)
    @action = action
  end

  def run(queue)
    until (@terminator.wait) do
      queue << @action
    end
  end

  def stop
    @terminator.stop
  end
end
