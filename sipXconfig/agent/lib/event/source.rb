require 'rubygems'
require 'file/tail'

module Events
  
  class ArrayReader 
    def initialize(lines, from = 0, to = lines.size)
      @lines = lines
      @index = from
      @max = to
    end
    
    def get_line
      while @index < @max
        line = @lines[@index]
        @index += 1
        yield line
      end      
    end
    
    def close; end    
  end
  
  class FileReader    
    def initialize(path)
      @file = File.open(path, "r")      
    end
    
    def get_line
      while line = @file.gets
          yield line.chomp if line
      end              
    end
    
    def close
      @file.close
    end    
  end
  
  class TailReader

# do HTTP request

    def initialize(path, interval=10)
      @path = path
      @interrupted = false
      @interval = interval
    end

    def interrupt
      @interrupted = true
    end
    
    def wait_for_new_file
    
      # Only seems to be nec. when run from rake
      # application doesn't need it
      trap("INT") { 
        @interrupted = true 
      }

      # If this server starts before acd server, file will not exists yet
      # so ...wait-for-i-i-i-i-t ....
      # puts "checking in #{@path} exists..."      # TODO:LOG
      while (!@interrupted && ! FileTest.exists?(@path))
        # puts "#{@path} does not exist, waiting"  # TODO:LOG
        sleep @interval
      end      
    end
    
    def get_line
      wait_for_new_file

      # NOTE: tail file implementation will send `EOFError' messages
      # to console when ruby in debug mode      
      @file = File::Tail::Logfile.open(@path) do |log|
        log.tail { |line| 
           yield line.chomp if line
        }
      end
    end
    
    def close
      @file.close if defined? @file
    end    
  end
    
  class Source
    def initialize(reader, parser)
      @reader = reader
      @parser = parser      
    end
    
    def each
      @reader.get_line do | line |
        $stderr.puts line if $DEBUG      
        event = @parser.parse_line(line)
        yield event if event
      end
      @reader.close
      nil
    end
  end
  
end
