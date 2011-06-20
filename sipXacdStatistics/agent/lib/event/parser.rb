require 'events'
require 'time'

module Events
  
  NAME_TO_CLASS = { 
    'start-acd' => AcdEvents::Start, 
    'stop-acd' => AcdEvents::Stop,
    'enter-queue' => AcdEvents::EnterQueue,
    'terminate' => AcdEvents::Terminate,
    'pick-up' => AcdEvents::PickUp,
    'transfer' => AcdEvents::Transfer,
    'sign-in-agent' => AcdEvents::AgentSignIn,
    'sign-out-agent' => AcdEvents::AgentSignOut
  }
  
  def Events.time_to_sec(time)
    Time.parse(time).to_i
  end
  
  # Extracts event from textual representation. 
  # Creates the event class based on the name of the event, 
  # recalculates the time and passes the remaining parameters to event constructor.
  class Parser
    def parse_line(line)
      fields = line.split(" : ")
      if klass = NAME_TO_CLASS[fields[0]]
        begin 
          time = Events.time_to_sec(fields[1])
          params = fields.slice(2, fields.length)
          klass.new(time, *params) 
        rescue StandardError => error
          # just log - do not interrupt parsing because of the old format errors
          $stderr.puts "#{klass} : #{error}"
        end  
      end
    end    
  end
end
