#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
##############################################################################

# system requirements
require 'socket'

# SipConnection opens a connection to a host:port
# and sends UDP messages to that destination

class SipConnection
  def initialize(hostName, hostPort)
    @sock = UDPSocket.open
    @sock.connect(hostName, hostPort)
  end

  def send(message)
    @sock.send(message, 0)
  end
end

# SipMessage constructs a SIP message depending on the actual
# passed in method

class SipMessage
  def initialize(seq, method, callId, from, to, referTo, referredBy, failure)
    case method
     when "OK" then @msg = "SIP/2.0 200 OK\r\n"
     when "Trying" then @msg = "SIP/2.0 100 Trying\r\n"
     when "Ringing" then @msg = "SIP/2.0 180 Ringing\r\n"
     else @msg = method + " " + to + " SIP/2.0\r\n"
    end

    @msg += "Via: SIP/2.0/TCP " + $ipAddress + ":47796;" \
            "branch=z9hG4bK-cd1a7003e\r\n"
    @msg += "To: " + to + "\r\n"
    @msg += "From: <" + from + ">;tag=d78c6098\r\n"
    @msg += "Call-ID: " + callId + "\r\n"
    @msg += "Cseq: " + seq + " " + method + "\r\n"
    @msg += "Max-Forwards: 20\r\n"
    @msg += "User-Agent: " + $userAgent + "\r\n"
    @msg += "Contact: " + from + "\r\n";
    @msg += "Date: Wed, 22 Mar 2006 21:57:37 GMT\r\n"
    @msg += "Content-Length: 0\r\n\r\n"
  end

  def to_s
    return @msg
  end
end

# SipCall makes a SIP call sending SIP messages to the proxy
# and auth proxy servers

class SipCall
  def initialize(proxy, authproxy, callId, toParty, fromParty)
    @proxy = proxy
    @authproxy = authproxy
    @inviteMessage = SipMessage.new("1", "INVITE", callId, \
                                    fromParty, toParty, "", "", 0)
    @tryingMessage = SipMessage.new("1", "Trying", callId, fromParty, \
                                    toParty, "", "", 0)
    @ringingMessage = SipMessage.new("1", "Trying", callId, \
                                     fromParty, toParty, "", "", 0)
    @okMessage = SipMessage.new("1", "OK", callId, fromParty, \
                                toParty, "", "", 0)
    @ackMessage = SipMessage.new("1", "ACK", callId, fromParty, \
                                 toParty, "", "", 0)
    @byeMessage = SipMessage.new("2", "BYE", callId, fromParty, \
                                 toParty, "", "", 0)
    @okByeMessage = SipMessage.new("2", "OK", callId, fromParty, \
                                   toParty, "", "", 0)
  end

  def makeSuccessful(duration)
    @proxy.send(@inviteMessage.to_s)
    sleep 0.1
    @authproxy.send(@tryingMessage.to_s)
    sleep 0.1
    @authproxy.send(@ringingMessage.to_s)
    sleep 0.1
    @authproxy.send(@okMessage.to_s)
    sleep 0.1
    @authproxy.send(@ackMessage.to_s)
    sleep duration
    @authproxy.send(@byeMessage.to_s)
    sleep 0.1
    @authproxy.send(@okByeMessage.to_s)
  end

  def makeFailed(duration)
  end
end
