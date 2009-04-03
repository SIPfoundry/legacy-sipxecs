#!/usr/bin/env ruby

# Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.

require 'test/unit'

DIR = File.dirname(__FILE__)

LOG_FILE="small.log"
CUT_FILE="#{LOG_FILE}.cut"

class SipXCutTest < Test::Unit::TestCase

   def call_script_and_check_result(starttime, stoptime, expected_retcode, expected_contents)
      command="#{DIR}/../sipx-log-cut -i #{DIR}/#{LOG_FILE} -s \"#{starttime}\" -e \"#{stoptime}\" 2>/dev/null"
      system(command)

      assert_equal(expected_retcode, $?.exitstatus)

      cut_file="#{DIR}/#{CUT_FILE}"
      if 1 != expected_retcode
         x=0
         file=File.open(cut_file, "r") 
         file.each_line do |line|
            assert_equal("#{x} - #{expected_contents[x]}", "#{x} - #{line.chomp}")
            x+=1
         end
      end

      File.delete(cut_file)
   end

   def test_SpanInvalid
      call_script_and_check_result("2009-02-17 09:02:14", "2009-02-16 09:02:18", 1, nil)
   end

   def test_SpanTooLate
      expected = []
      call_script_and_check_result("2009-02-17 09:02:10", "2009-02-17 09:02:14", 0, expected)
   end

   def test_SpanOutOf
      expected = [
         "\"2009-02-17T09:02:15.976445Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:15.976563Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:17.976758Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:18.977017Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.543211Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
      ]
      call_script_and_check_result("2009-02-17 09:02:14", "2009-02-17 09:02:18", 0, expected)
   end
  
   def test_SpanWithin
      expected = [
         "\"2009-02-17T09:02:15.976563Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:17.976758Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:18.977017Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.543211Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.977292Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.977412Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.977663Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.977682Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.977734Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.977787Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.978269Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.978298Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.978522Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.998538Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.999084Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.999150Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.999549Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.999599Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.999765Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.999911Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.000039Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
      ]
      call_script_and_check_result("2009-02-17 09:02:16", "2009-02-17 09:02:20", 2, expected)
   end

   def test_SpanCovers
      expected = [
         "\"2009-02-17T09:02:15.976445Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:15.976563Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:17.976758Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:18.977017Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.543211Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.977292Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.977412Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.977663Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.977682Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.977734Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.977787Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.978269Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.978298Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.978522Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.998538Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.999084Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.999150Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.999549Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.999599Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.999765Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:19.999911Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.000039Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.000180Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.000304Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.000325Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.000372Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.000516Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.000584Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.000611Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.000651Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.000682Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.000854Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.000874Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.000960Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.001050Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.001098Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.001686Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.001712Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.002029Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.003401Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.003939Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.004235Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.004565Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.004755Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:22.004833Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:26.004878Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:26.005119Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:26.005570Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:26.005829Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:26.025754Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:26.026052Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:26.026438Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:26.026756Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:26.026798Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:26.026843Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:26.026902Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:26.026965Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:26.026981Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:26.027002Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:26.027033Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:26.027048Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:02:26.027104Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:03:26.907266Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:03:27.017608Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:03:27.027634Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:04:27.037739Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
      ]
      call_script_and_check_result("2009-02-17 09:02:14", "2009-02-17 09:04:28", 0, expected)
   end

   def test_SpanInto
      expected = [
         "\"2009-02-17T09:02:26.027104Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:03:26.907266Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:03:27.017608Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:03:27.027634Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
         "\"2009-02-17T09:04:27.037739Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
      ]
      call_script_and_check_result("2009-02-17 09:02:27", "2009-02-19 05:00:30", 2, expected)
   end

   def test_SpanTooEarly
      expected = []
      call_script_and_check_result("2009-02-17 09:04:28", "2009-02-18 06:30:00", 2, expected)
   end

end
