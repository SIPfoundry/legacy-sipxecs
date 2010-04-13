#!/usr/bin/env ruby

require 'test/unit'
require 'tempfile'
require 'fileutils'

SRCDIR = File.join(File.dirname(__FILE__), "..")
$:.unshift SRCDIR
require 'sipx-upgrade-3.6-mailstore'

class MailstoreMigrateTest < Test::Unit::TestCase

  def test_migrate
    migrate = MailstoreMigrate.new   
    mailstore_tmp = File.join(Dir.tmpdir, 'mailstore-3.6.tmp')
    FileUtils.rm_r(mailstore_tmp) if File.exists?(mailstore_tmp)
    Dir.chdir("#{SRCDIR}/test") {    
      FileUtils.cp_r('mailstore-3.6', mailstore_tmp)
      assert ! File.exists?(mailstore_tmp + "/160/saved/00050627-00.wav")
      assert File.exists?(mailstore_tmp + "/160/bbb/00050627-00.wav")    
      migrate.migrate(mailstore_tmp)      
      assert File.exists?(mailstore_tmp + "/160/saved/00050627-00.wav")
      assert ! File.exists?(mailstore_tmp + "/160/bbb/00050627-00.wav")    
    }
  end
  
end
