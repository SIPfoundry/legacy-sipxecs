#!/usr/bin/env ruby

SRCDIR = File.join(File.dirname(__FILE__), "..")
$:.unshift SRCDIR

require 'test/unit'
require 'stringio'
require 'sipx-upgrade-3.6-mailstore-report'

class MailstoreReportTest < Test::Unit::TestCase

  def test_report
    actual = StringIO.new
    Dir.chdir("#{SRCDIR}/test") {
      MailstoreReport.new.report("mailstore-3.6", actual)
      expected = <<EOF
following emails will not be preserved after first user edit:
mailbox mailstore-3.6/160/mailboxprefs.xml, email douglas@hubler.us
EOF
      assert_equal(expected, actual.string)
    }
  end
  
end
