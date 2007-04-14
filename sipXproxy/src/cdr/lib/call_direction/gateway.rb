#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

# System requires
require 'dbi'
require 'ipaddr'

# Application requires.  Assume that the load path includes this file's dir.
require 'utils/exceptions'
require 'utils/sipx_ipsocket'
require 'utils/socket_utils'
require 'utils/utils'

SIPXCONFIG_DATABASE = 'SIPXCONFIG'


# The Gateway class models a gateway defined in the sipXconfig database. Gateways in the
# database can be given addresses that are either domain names or IP addresses.
# Resolve domain names to IP addresses so that the address will be in a
# canonical form for matching. Assume that the gateway address will not change
# while this process is running, so we can cache the DNS lookup.
class Gateway
  public
  
  def initialize(name, address)
    @name = name
    @address = address
  end
  
  # Find ip_addresses for this gateway and log the errors
  def ip_addresses(log)
    addresses = extract_ip_addresses
    if !@address
      log.error("Gateway #{@name} does not have IP address")
    elsif addresses.length == 0
      log.error(%Q/Unable to resolve the domain name "#{gateway.address}" \
        for the gateway "#{gateway.name}. \
        This gateway will not be used when computing call direction./)
    else
      log.debug("#{self} addresses: #{addresses.to_s}")
    end      
    return addresses
  end
  
  def to_s
    "Gateway: #{@name}, address: #{address}"
  end
  
  # If the gateway address is a domain name, then resolve it to an IP addr.
  # Return nil if cannot resolve, return array of addresses if can resolve.
  def extract_ip_addresses
    return unless @address
    
    # Strip a possible port number off the IPv4 address
    addr = SocketUtils.strip_v4_port(@address)
    
    # return address if it's a valid IP address    
    return [addr] if SipxIPSocket.valid_ipaddr?(addr)
    
    # Strip a possible port number from domain name
    addr = $1 if /\A(.+):\d+\Z/ =~ addr
    
    # The gateway address is not an IP address, so it must be a domain name.
    # Try to resolve it.
    addrs = SocketUtils.getaddresses(addr)
  end
  
  class << self
    def find_all
      begin
        gateways = []
        # connect to the MySQL server
        dbh = DBI.connect("dbi:Pg:#{SIPXCONFIG_DATABASE}:localhost", "postgres")
        # get server version string and display it
        sth = dbh.execute("SELECT name, address FROM gateway")
        sth.each do |row|
          gateways << Gateway.new(row[0], row[1])
        end
        sth.finish
        return gateways
      rescue DBI::DatabaseError => e
        puts "An error occurred"
        puts "Error code: #{e.err}"
        puts "Error message: #{e.errstr}"
      ensure
        # disconnect from server
        dbh.disconnect if dbh
      end    
    end
    
    # Find the gateways.  For gateways configured with domain names, resolve the
    # names to IP addresses so we have a canonical format for address matching.
    # Return array of resolved IP addresses.
    # FIXME: we should not be connecting to SIPXCONFIG database, it's better to use SOAP or somehow find out from proxy
    def addresses
      # Build a gateway IP list
      Gateway.find_all.inject([]) do | addresses, gateway |
        ip_addresses = gateway.ip_addresses
        addresses.concat(ip_addresses) if ip_addresses
      end
    end
    
  end
  
end
