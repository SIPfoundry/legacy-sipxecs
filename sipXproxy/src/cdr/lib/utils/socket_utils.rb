#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

require 'resolv'

require 'utils/sipx_ipsocket'

class SocketUtils
  
  # SocketUtils has only class methods, so don't allow instantiation.
  private_class_method :new

public
  
  def SocketUtils.getaddresses(address)
    if SipxIPSocket.valid_ipaddr?(address)
      return [address]
    end
    result_list = []    
    addr_list = []
    addr_list = Resolv::getaddresses(address)
    # If addr_list is empty try to resolve DNS SRV record
    if addr_list.length == 0
      Resolv::DNS.open {|dns| dns.getresources(address,
                                    Resolv::DNS::Resource::IN::SRV).collect {|r| addr_list << r.target.to_s}}
    end
    # For each entry in the address list try to resolve name recursively
    if addr_list.length != 0
      result = []
      addr_list.each do |addr|
        result = SocketUtils.getaddresses(addr)
        if result.length != 0
          # Record the resolved address if it's not an IP address
          if ! SipxIPSocket.valid_ipaddr?(address)
            if ! result_list.include?([address])
              result_list.concat([address])
            end
          end
          if ! result_list.include?([result])        
            result_list.concat(result)
          end
        end
      end
    end
    return result_list
  end
  
  def SocketUtils.strip_v4_port(addr)
    # Look for string with IPv4 format and a colon with a trailing number
    if /\A(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}):\d+\Z/ =~ addr
      return $1
    else
      return addr
    end
  end

end
