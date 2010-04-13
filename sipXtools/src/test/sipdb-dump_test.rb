#!/usr/bin/env ruby

# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.

DIR = File.dirname(__FILE__)
$:.unshift File.join(DIR, "..")
require 'test/unit'
require 'stringio'
load 'sipdb-dump'

class SipdbDumpTest < Test::Unit::TestCase

  def test_dump
      out = StringIO.new
      dumper = SipDbDumper.new(out)
      dumper.dump("#{DIR}/registration.xml")
      expected = ["callid=c29c2b63-de009a78-44188331@10.1.1.161",
       "cseq=958",
       "primary=sipxchange.pingtel.com",
       "uri=sip:160@pingtel.com",
       "contact=&lt;sip:160@10.1.1.161;transport=tcp&gt;",
       "contact_host=10.1.1.161",
       "qvalue=",
       "expires=1164082496",
       "expired=false",
       "instance_id=",
       "gruu=",
       "path=",
       "update_number=0x456232bf00000287"]
      assert_equal expected, out.string.strip.split("\t")
  end
  
  def test_contact_host_1
      out = StringIO.new
      dbtime=Time.at(1164082128)
      SipDbDumper.new().dump_contact(out, "contact", "&quot;D K&quot;&lt;sip:154@10.1.1.164;LINEID=7bfd&gt;",dbtime)
      assert_equal "contact=&quot;D K&quot;&lt;sip:154@10.1.1.164;LINEID=7bfd&gt;\tcontact_host=10.1.1.164\t", out.string
  end
  
  def test_contact_host_2
      out = StringIO.new
      dbtime=Time.at(1164082128)
      SipDbDumper.new().dump_contact(out, "contact", "&lt;sip:154@10.1.1.164&gt;",dbtime)
      assert_equal "contact=&lt;sip:154@10.1.1.164&gt;\tcontact_host=10.1.1.164\t", out.string
  end
  
  def test_expired_false
      out = StringIO.new
      dbtime=Time.at(1164082128)
      SipDbDumper.new().dump_expires(out, "expires", "1164082496",dbtime)
      assert_equal "expires=1164082496\texpired=false\t", out.string    
  end
  
  def test_expired_true
      out = StringIO.new
      dbtime=Time.at(1164082500)
      SipDbDumper.new().dump_expires(out, "expires", "1164082496",dbtime)
      assert_equal "expires=1164082496\texpired=true\t", out.string    
  end
  
  def test_cli
    assert Regexp.new('data dumper', Regexp::MULTILINE).match(`#{DIR}/../sipdb-dump --help`)
  end
  
end


