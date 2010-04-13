# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.

module States
  
  # keeps track of the wait time for calls terminated in last 30 minutes
  class TerminatedCalls
    
    # information about a single call, agregates termination time_stamp and wait_time for a call
    class Info
      attr_reader :timestamp, :wait_time
      def initialize(timestamp, wait_time)
        @timestamp = timestamp
        @wait_time = wait_time
      end
    end
    
    # memory is interval (in seconds) of how long we will keep the
    # by default it is set to 30 minutes
    def initialize(memory = 30 * 60)
      @calls = []
      @memory = memory
    end
    
    # returns the triple (total_time, number_of_calls, max_time)
    # it's going to be used to compute the average waiting time
    def total_count_max(now)
      oldest_counted = now - @memory
      old_calls_index = 0      
      results = @calls.inject([0, 0, 0]) do |totals, info|
        if oldest_counted < info.timestamp
          time = info.wait_time
          totals[0] += time
          totals[1] += 1
          totals[2] = time if totals[2] < time
        else
          old_calls_index += 1                    
        end        
        totals
      end
      # trim the calls table
      @calls.slice!(0, old_calls_index)
      return results
    end
    
    def add_call(timestamp, wait_time)
      trim(timestamp)
      @calls << Info.new(timestamp, wait_time)
    end
    
    # remove all the calls data that is older than memory
    private
    def trim(now)
      index = 0
      @calls.each_index do |index|
        break if now - @calls[index].timestamp <= @memory
      end
      @calls.slice!(0, index)
    end
  end
  
end
