# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.

module States
  
  class ItemOutOfOrderError < StandardError
    def initialize(item)
      super("Item out of order: #{item}")
    end
  end
  
  class History
    # keep at most 90 minutes history
    # This should roughly be greater than 2x the cron frequency in /etc/cron.d/sipxconfig-report
    DEFAULT_MEMORY = 90 * 60
    
    # no sense throwing away old events if memory consumption is negligible.
    # this is mainly useful when system has been down for more than 90 minutes and common complaint 
    # is that users see NO history.  If report utilities (such as sipxconfig-report) keep reading 
    # within the DEFAULT_MEMORY time window, this is value is not important.
    DEFAULT_KEEP_AT_LEAST = 1000
    
    def initialize(memory = DEFAULT_MEMORY, min_size = DEFAULT_KEEP_AT_LEAST)
      @memory = memory
      @min_size = min_size
      @history = []
    end
    
    def add(item, now)
      if !@history.empty?    
        raise ItemOutOfOrderError.new(item) if item.time < @history.last.time              
        trim(now)
      end        
      @history << item
    end
    
    def get_history(from_time)
      i = newer_index(from_time)
      @history.slice(i..@history.size)
    end    
    
    def trim(now)      
      from_time = Time.at(now - @memory)
      i = newer_index(from_time)
      over_quota = @history.size - (@min_size - 1)
      remove_from = [i, over_quota].min
      @history.slice!(0, remove_from) if remove_from > 0
    end
    
    private 
    def newer_index(from_time)
      @history.each_index do |i|
        return i if @history[i].time >= from_time
      end
      return @history.size
    end
  end
end
