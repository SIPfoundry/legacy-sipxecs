#!/usr/bin/env ruby

# Starting with sipx 3.8, editing multiple email address are not supported
# This script will report to stdout all the mailboxes and email address
# that will be affected.  It does not remove the email addresses, so they
# will continue to work until the first edit is made on a user from the 
# sipxconfig web ui.
# See http://track.sipfoundry.org/browse/XCF-1544

require 'find'
require 'rexml/document'

class MailstoreReport

  def report(mailstore, output)
    output.puts "following emails will not be preserved after first user edit:"    
    Find.find(mailstore) do |path|
      if File.basename(path) == 'mailboxprefs.xml'
        File.open(path) do |prefs_stream|
          doc = REXML::Document.new(prefs_stream)
          contacts = doc.get_elements('//notification/contact')
          if (contacts)
            for i in 1..( contacts.length - 1 )
              email = contacts[i].text
              output.puts "mailbox #{path}, email #{email}"
            end          
          end
        end
      end
    end
  end  
end

if __FILE__ == $0
  mailstore = ARGV.empty? ? "@SIPX_VXMLDATADIR@/mailstore" : ARGV[0]
  MailstoreReport.new().report(mailstore, STDOUT)
end
