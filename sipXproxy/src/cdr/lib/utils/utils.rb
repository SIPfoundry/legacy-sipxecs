#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

# system requires
require 'ipaddr'

# application requires
require 'utils/exceptions'
require 'utils/sipx_ipsocket'
require 'utils/socket_utils'

class Utils
  # Utils has only class methods, so don't allow instantiation.
  private_class_method :new
  
  # LATER: Add IPv6 matching
  IPADDRV4 = '(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})'
  DOMAINLABEL = '[a-zA-Z0-9]([a-zA-Z0-9-]*[a-zA-Z0-9])?'
  TOPLABEL = '[a-zA-Z]([a-zA-Z0-9]|-)*[a-zA-Z0-9]'  
  HOSTNAME = "(#{DOMAINLABEL}\\.)*#{TOPLABEL}"    
  HOST = "(#{IPADDRV4}|#{HOSTNAME})"
  HOSTPORT = "#{HOST}(:\\d+)?"
  CONTACT = "(\\<?.*:(.+@)?#{HOSTPORT}).*\\>?"
  EMBEDDEDQUOTES = '(\\\".*\\\")'
  
  #patterns
  SIPURI = Regexp.new(".*:(.+@)?(#{HOST})")  
  QUOTEDSTRING = Regexp.new("(\".*\")\s*(\\<.*:.*)")
  
  
  # Take a contact string like "Jorma Kaukonen"<sip:187@10.1.1.170:1234>;tag=1c32681
  # and extract just the host part, in this case "10.1.1.170".  The "@" is optional,
  # could be just "sip:10.1.1.170" for example.
  def Utils.contact_host(contact)
    # Use regular expression to extract the hostport part
    sip_uri = if contact =~ QUOTEDSTRING
      $2
    else
      contact
    end
    if sip_uri =~ SIPURI
      $2
    else   
      raise(BadContactException.new(contact),
            "Bad contact, can't extract address: \"#{contact}\"")
    end
  end
  
  # Take a string like "Jorma Kaukonen"<sip:187@10.1.1.170:1234>;tag=1c32681
  # and extract just the user part, in this case "187". 
  def Utils.contact_user(contact)
    # Use regular expression to extract the user part
    if contact =~ QUOTEDSTRING
      sip_uri = $2
    else
      sip_uri = contact
    end
    sipstr = sip_uri.to_s
    tail = sipstr.index('@')
    start = sipstr.index(':')
    if start && tail
       contact = sipstr[start+1...tail]
    end
    contact
  end
  
  # Look for the char in the str.  If found, then remove that char and everything
  # after it.  Return the str.
  def Utils.remove_part_of_str_beginning_with_char(str, char)
    char_index = str.index(char)
    if char_index
      str = str[0...char_index]
    end
    str
  end
  
  # Return just the AOR part of a SIP From or To header, stripping the tag if
  # one is present.
  # If there is no tag then raise BadSipHeaderException if is_tag_required is true.
  # :LATER: Use regex here, much simpler
  def Utils.get_aor_from_header(header, is_tag_required = true)
    aor = nil
    # Strip quoted display name 
    if header =~ QUOTEDSTRING
      sip_uri = $2
    else
      sip_uri = header
    end
    # Remember how long the display name was we chopped off and add that
    # back on when getting the semicolon position
    add_pos = header.length - sip_uri.length      
    
    # find the semicolon and tag
    semi = sip_uri.index(';tag')
    
    # extract the AOR
    if semi
      # Add the length we originally stripped off
      semi += add_pos
      aor = header[0, semi]
    else
      if is_tag_required
        raise(BadSipHeaderException. new(header),
              'Tag missing from SIP header',
        caller)
      end
      aor = header
    end
    
    # put '>' at the end to balance '<' at the start, if necessary
    if aor[0] == ?< and aor[-1] != ?>
      aor << '>'
    end
    
    aor
  end
  
  # Given a contact URL with params, like <sip:101@10.1.20.3:5100;play=https%3A%2F%2Flocalhost>,
  # remove the params, which are preceded by semicolons.  Usually there is a '>' at the end
  # matching a '<' at the beginning.  If so then leave it in place.
  # :LATER: Use regex here, much simpler
  def Utils.contact_without_params(contact)
    if contact =~ QUOTEDSTRING
      sip_uri = $2
    else
      sip_uri = contact
    end
    add_pos = contact.length - sip_uri.length
    
    semi_index = sip_uri.index(';')
    if semi_index
      semi_index += add_pos
      gt_index = contact.index('>')
      contact = contact[0...semi_index]
      contact << '>' if gt_index
    end
    contact
  end
  
  # Raise a CallResolverException.  Include the stack trace.
  def Utils.raise_exception(msg, klass = CallResolverException)
    raise(klass, msg, caller)
  end
  
  # Given an events array, return a string that displays one event per line,
  # with preceding newlines.  Used for debugging.
  def Utils.events_to_s(events)
    events.inject('') {|str, event| str + "\n" + event.to_s}
  end
  
end
