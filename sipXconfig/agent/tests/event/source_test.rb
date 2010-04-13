$:.unshift File.join(File.dirname(__FILE__), "..", "..", "lib")

require 'test/unit'
require 'event/source'
require 'tempfile'

class EventSourceTest < Test::Unit::TestCase
  
  class DummyReader
    def initialize(max)
      @max = max
      @i = 0
    end
    
    def get_line
      while @i < @max
        @i += 1
        yield "bongo"
      end
    end
    
    def close; end
  end
  
  
  class DummyParser
    def parse_line(line)
      if line == "bongo"
        return 1
      end
    end
  end
  
  LEN = 5
  
  def test_source
    reader = DummyReader.new(LEN)
    parser = DummyParser.new
    
    source = Events::Source.new(reader, parser)    
    i = 0
    source.each {|event| i += event }
    assert_equal(LEN, i)    
  end

  def test_array_reade
    reader = Events::ArrayReader.new(["bongo", "bongo"])
    parser = DummyParser.new
    
    source = Events::Source.new(reader, parser)    
    i = 0
    source.each {|event| i += event }
    assert_equal(2, i)    
  end
  
end

class TailReaderTest < Test::Unit::TestCase

  def test_wait_for_new_file
    tmp = Tempfile.new('test_wait_for_new_file')
    f = tmp.path
    tmp.delete
    tail = Events::TailReader.new(f, 0.5)

    create_file = Thread.new do
      puts "create file in 1 sec"
      sleep(1)
      puts "creating file"
      File.open(f, File::RDWR|File::CREAT)
    end
 
    reaped = false
    reaper = Thread.new do
      puts "reap unit test in 2 sec"
      sleep(2)
      reaped = true
      tail.interrupt
    end

    puts "waiting for #{f} to be created..."
    tail.wait_for_new_file
    assert ! reaped
  end

end
