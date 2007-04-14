#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

$:.unshift(File.join(File.dirname(__FILE__), "..", ".."))

require 'test/test_helper'
require 'parsedate'
require 'call_resolver'
require 'utils/configure'

require 'app/models/call_state_event'
require 'app/models/cdr'


# :TODO: Make it easy to run all the unit tests, possibly via Rakefile, for build loop.
class CallResolverPurgeTest < Test::Unit::TestCase

  def setup  
    init_db
    # Create the CallResolver, giving it the location of the test config file.
    @resolver = CallResolver.new(File.join($SOURCE_DIR, 'data/callresolver-config'))
  end
  
  def test_purge_all_old_calls
    # Make sure we have a clean database at the start of each test
    assert(clean_db)
      
    make_call("sip:200@example.com", "sip:201@example.com", "id-1", 10)    # 10 days old       
    make_call("sip:201@example.com", "sip:200@example.com", "id-2", 11)    # 11 days old
    make_call("sip:200@example.com", "sip:201@example.com", "id-3", 12)    # 12 days old          
    make_call("sip:203@example.com", "sip:205@example.com", "id-4", 40)    # 40 days old
    make_call("sip:204@example.com", "sip:206@example.com", "id-5", 36)    # 36 days old 
    
    # Resolve - expect to find 5 resolved calls
    resolve(5)
    
    # Purge records older than 9 days
    purge(9,9)
    
    assert_equal(CallStateEvent.count,0, 'Wrong number of call state events')
    assert_equal(Cdr.count, 0, 'Wrong number of CDRs')
  end
  
  def test_purge_edge_case
    # Make sure we have a clean database at the start of each test
    assert(clean_db)
      
    make_call("sip:200@example.com", "sip:201@example.com", "id-1", 10)    # 10 days old       
    make_call("sip:201@example.com", "sip:202@example.com", "id-2", 11)    # 11 days old
    
    # Resolve - expect to find 2 resolved calls
    resolve(2)
    
    # Purge records older than  or as old as 11 days
    purge(11,11)
    
    assert_equal(CallStateEvent.count, 3, 'Wrong number of call state events')
    assert_equal(Cdr.count, 1, 'Wrong number of CDRs')
  end  
  
  def test_purge_remove_cse_separately
    # Make sure we have a clean database at the start of each test
    assert(clean_db)
  
    make_call("sip:200@example.com", "sip:201@example.com", "id-1", 1)    # 1 day old       
    make_call("sip:201@example.com", "sip:200@example.com", "id-2", 2)    # 2 days old
    make_call("sip:200@example.com", "sip:201@example.com", "id-3", 10)   # 10 days old          
    # 40 days old with two parties that are not used in newer calls
    make_call("sip:203@example.com", "sip:205@example.com", "id-4", 40) 
    # 36 days old with one party that are not used in newer calls
    make_call("sip:204@example.com", "sip:206@example.com", "id-5", 36)     
    
    # Resolve - expect to find 5 resolved calls
    resolve(5)
    
    # Purge CSE records older than 5 days, CDRs older than 35
    purge(5,35)
    
    assert_equal(CallStateEvent.count,6, 'Wrong number of call state events')
    assert_equal(Cdr.count, 3, 'Wrong number of CDRs')
  end  
  
  #-----------------------------------------------------------------------------
  # Helper methods
  def resolve(expected)
    start_time = Time.parse('1990-01-1T000:00:00.000Z')
    end_time = Time.parse('2030-12-31T00:00.000Z')
    @resolver.resolve(start_time, end_time)
    assert_equal(expected, Cdr.count, 'Wrong number of CDRs')
  end

  def purge(age_cse, age_cdr)
    cse_start_time = Time.now - (Configure::SECONDS_PER_DAY * age_cse)
    cdr_start_time = Time.now - (Configure::SECONDS_PER_DAY * age_cdr)
    @resolver.send(:purge, cse_start_time, cdr_start_time)
  end  
  
  def make_call(from, to, id, days_old)
    cse = CallStateEvent.new
    # Adjust the time a bit so that all records are slightly older 
    # than 'now - age'
    cse.cseq = 1
    cse.event_time = Time.now - (Configure::SECONDS_PER_DAY * days_old) - 50
    cse.contact = from
    cse.from_url = from + "; tag=f"
    cse.to_url = to    
    cse.event_seq = (@cseq += 1)
    cse.event_type = 'R'
    cse.call_id = id
     
    cse.refer_to = ""
    cse.referred_by = ""
    cse.observer = "TestObserver"   
    cse.failure_reason = "No reason"
    cse.failure_status = ""
    cse.from_tag = "f"
    cse.to_tag = "t"  
    
    cse.save
 
    cse = CallStateEvent.new
    # Adjust the time a bit so that all records are slightly older 
    # than 'now - age'    
    cse.cseq = 2
    cse.event_time = Time.now - (Configure::SECONDS_PER_DAY * days_old) - 30
    cse.contact = to
    cse.from_url = from + "; tag=f"
    cse.to_url = to    
    cse.event_seq = (@cseq += 1)
    cse.event_type = 'S'
    cse.call_id = id
    
    cse.refer_to = ""
    cse.referred_by = ""
    cse.observer = "TestObserver"   
    cse.failure_reason = "No reason"
    cse.failure_status = ""
    cse.from_tag = "f"
    cse.to_tag = "t"      
    
    cse.save

    cse = CallStateEvent.new    
    # Adjust the time a bit so that all records are slightly older 
    # than 'now - age'       
    cse.cseq = 3
    cse.event_time = Time.now - (Configure::SECONDS_PER_DAY * days_old) - 10
    cse.contact = from
    cse.from_url = from + "; tag=f"
    cse.to_url = to    
    cse.event_seq = (@cseq += 1)
    cse.event_type = 'E'
    cse.call_id = id    
    
    cse.refer_to = ""
    cse.referred_by = ""
    cse.observer = "TestObserver"   
    cse.failure_reason = "No reason"
    cse.failure_status = ""
    cse.from_tag = "f"
    cse.to_tag = "t"      
    
    cse.save
  end
  
  def clean_db
    clean = true
    
    clean = false if (CallStateEvent.count != 0)
    clean = false if (Cdr.count != 0)
    clean
  end
  
  def init_db
    # Connect to the database
    # :TODO: read these parameters from the Call Resolver config file
    ActiveRecord::Base.establish_connection(
        :adapter  => "postgresql",
        :host     => "localhost",
        :username => "postgres",
        :database => "SIPXCDR")
    
    # Clear database
    CallStateEvent.delete_all
    Cdr.delete_all
    
    @cseq = 0
  end  
  
end
