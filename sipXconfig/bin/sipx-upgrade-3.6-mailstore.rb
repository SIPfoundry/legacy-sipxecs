#!/usr/bin/env ruby

# Starting with sipx 3.8, custom vm folders are not allowed
# so this script will move all voicemail to respective 'saved' folder
# See http://track.sipfoundry.org/browse/XCF-1544

require 'find'
require 'fileutils'

class MailstoreMigrate

  def migrate(mailstore)
    puts "migrating mailstore #{mailstore}"
    Dir.chdir(mailstore) do
      Dir.glob('*') do |mailbox|
        if FileTest.directory?(mailbox)
          migrate_mailbox(mailbox)
        end
      end      
    end 
  end
  
  def migrate_mailbox(mailbox)
    puts "migrating mailbox #{mailbox}"
    Dir.chdir(mailbox) do
      Dir.glob('*') do |folder|
        if FileTest.directory?(folder)
	      if folder !~ /^(inbox|deleted|saved|[.]+?)$/
            puts "migrating folder #{folder}"
	        Dir.foreach(folder) do |vmfile|
	          if ! FileTest.directory?(vmfile)
  	            fname = File.basename(vmfile)
	            src = Dir.pwd + "/#{folder}/#{fname}"
	            dest = Dir.pwd + "/saved/#{fname}"
	            puts "moving #{src} to #{dest}"
	            FileUtils.mv(src, dest)
	          end
	        end	        
	      end
	    end
      end
    end
  end
end

if __FILE__ == $0
  mailstore = ARGV.empty? ? "@SIPX_VXMLDATADIR@/mailstore" : ARGV[0]
  MailstoreMigrate.new().migrate(mailstore)
end
