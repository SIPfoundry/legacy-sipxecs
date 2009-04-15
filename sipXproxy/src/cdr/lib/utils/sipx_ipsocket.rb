# This file is intentionally lacking the standard SIPfoundry copyright header,
# because the code below is really part of Ruby. This is a hack, hopefully temporary.
# Copied these methods from the Ruby file ipaddr.rb, which has the code for
# these methods in the class IPSocket, but inexplicably hides them so they can't be used.
# Add "ipaddr" to the method names for clarity.

class SipxIPSocket
  
  # SipxIPSocket has only class methods, so don't allow instantiation.
  private_class_method :new

public
 
  def SipxIPSocket.valid_v4_ipaddr?(addr)
    if /\A(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})\Z/ =~ addr
      return $~.captures.all? {|i| i.to_i < 256}
    end
    return false
  end

  def SipxIPSocket.valid_v6_ipaddr?(addr)
    # IPv6 (normal)
    return true if /\A[\dA-Fa-f]{1,4}(:[\dA-Fa-f]{1,4})*\Z/ =~ addr
    return true if /\A[\dA-Fa-f]{1,4}(:[\dA-Fa-f]{1,4})*::([\dA-Fa-f]{1,4}(:[\dA-Fa-f]{1,4})*)?\Z/ =~ addr
    return true if /\A::([\dA-Fa-f]{1,4}(:[\dA-Fa-f]{1,4})*)?\Z/ =~ addr
    # IPv6 (IPv4 compat)
    return true if /\A[\dA-Fa-f]{1,4}(:[\dA-Fa-f]{1,4})*:/ =~ addr && valid_v4?($')
    return true if /\A[\dA-Fa-f]{1,4}(:[\dA-Fa-f]{1,4})*::([\dA-Fa-f]{1,4}(:[\dA-Fa-f]{1,4})*:)?/ =~ addr && valid_v4?($')
    return true if /\A::([\dA-Fa-f]{1,4}(:[\dA-Fa-f]{1,4})*:)?/ =~ addr && valid_v4?($')
     false
  end

  def SipxIPSocket.valid_ipaddr?(addr)
    valid_v4_ipaddr?(addr) || valid_v6_ipaddr?(addr)
  end

end
