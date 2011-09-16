$:.unshift File.join(File.dirname(__FILE__), "..", "lib")
require 'sipxexample'
require 'test/unit'

class ExampleTest < Test::Unit::TestCase
  def test_hello
    assert_equal(0, Sipx::Example.new.hello)
  end 
end
