package org.sipfoundry.openfire.plugin.presence;

import java.io.BufferedReader;
import java.io.InputStreamReader;

import org.apache.log4j.Logger;
import org.jivesoftware.openfire.interceptor.PacketInterceptor;
import org.restlet.Client;
import org.restlet.data.Method;
import org.restlet.data.Protocol;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.xmpp.packet.Message;
import org.jivesoftware.openfire.XMPPServer;
import org.xmpp.packet.Packet;
import org.jivesoftware.openfire.session.Session;
import org.jivesoftware.openfire.interceptor.PacketRejectedException;

public class MessagePacketInterceptor implements PacketInterceptor {
    private static Logger log = Logger.getLogger("org.sipfoundry");
    private SipXOpenfirePlugin plugin;

    MessagePacketInterceptor(SipXOpenfirePlugin plugin) {
        this.plugin = plugin;
    }

    public void interceptPacket(Packet packet, Session session, boolean incoming,
            boolean processed) throws PacketRejectedException {
        try {
            log.debug("intercept Packet " + packet);
            if (packet instanceof Message) {
                Message message = (Message) packet;
                if (message.getType() == Message.Type.chat) {
                    String chatText = message.getBody();
                    if (chatText != null) {
                        if (incoming && processed) {
                            log.debug("message is: " + chatText);
                            if (chatText.startsWith("@call") || chatText.startsWith("@xfer")) {
                                // build the URI.
                                // First find out who is sending the message. This will be the
                                // caller
                                log.debug("from : " + message.getFrom());
                                log.debug("from node: " + message.getFrom().toBareJID());
                                String fromSipId = plugin.getSipId(message.getFrom().toBareJID());
                                String toSipId = plugin.getSipId(message.getTo().toBareJID());
                                log.debug(fromSipId + ":" + toSipId);

                                if (chatText.startsWith("@call")) {
                                    if (fromSipId != null) {
                                        // check if there is a phone number after @call
                                        String[] result = chatText.split("\\s");
                                        String numberToCall;
                                        if (result.length >= 2) {
                                            // TODO: turn this next block of code into a function
                                            String username = result[1]
                                                    + "@"
                                                    + XMPPServer.getInstance().getServerInfo()
                                                            .getXMPPDomain();
                                            log.debug("call username is " + username);
                                            
                                            // Assume that the target of the call is an XMPP id.
                                            // Try to resolve this to a sip id.
                                            String calledSipId = null;
                                            try {
                                                calledSipId = plugin.getSipId(username);
                                            } catch (Exception e) {
                                                log.debug("caught: " + e.getMessage());
                                            }

                                            // The called ID is not an XMPP id. Then this must be
                                            // a SIP id directly specified.
                                            if (calledSipId != null) {
                                                numberToCall = calledSipId.substring(0, fromSipId
                                                        .indexOf('@'));
                                            } else {
                                                numberToCall = result[1];
                                            }
                                        } else {
                                            numberToCall = toSipId.substring(0, fromSipId
                                                    .indexOf('@'));
                                        }

                                        String restCallCommand = buildRestCallCommand(fromSipId,numberToCall);
                                        sendRestRequest(restCallCommand);
                                    }
                                } else if (chatText.startsWith("@xfer")) {
                                    if (toSipId != null ) {
                                        // check if there is a phone number after @call
                                        String[] result = chatText.split("\\s");
                                        String numberToCall;
                                        if (result.length >= 2) {
                                            // figure out what that number is...
                                            // First, check if it is the name part of a JID on the
                                            // system
                                            // that is mapped to a SIP ID.
                                            String username = result[1]
                                                    + "@"
                                                    + XMPPServer.getInstance().getServerInfo()
                                                            .getXMPPDomain();
                                            log.debug("xfer username is " + username);
                                            String calledSipId = null;
                                            try {
                                                calledSipId = plugin.getSipId(username);
                                            } catch (Exception e) {
                                                log.debug("caught: " + e.getMessage());
                                            }

                                            if (calledSipId != null) {
                                                numberToCall = calledSipId.substring(0, fromSipId
                                                        .indexOf('@'));
                                            } else {
                                                numberToCall = result[1];
                                            }
                                        } else {
                                            numberToCall = toSipId.substring(0, fromSipId
                                                    .indexOf('@'));
                                        }
                                        String agentId = fromSipId;
                                        String restCallCommand = buildRestCallCommand(fromSipId,toSipId, numberToCall);
                                        sendRestRequest(restCallCommand);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } catch (Exception e) {
            log.debug("Caughtx: '" + e.getMessage());
            e.printStackTrace(System.err);
        }
    }

    String buildRestCallCommand(String callerSipUri,  String calledNumber) {
        String restCallCommand = "https://" + plugin.getCallControllerConfig().getIpAddress() + ":" +
            plugin.getCallControllerConfig().getHttpPort() + "/callcontroller/"
                + callerSipUri.substring(0, callerSipUri.indexOf('@')) + "/"
                + calledNumber;
        log.debug("rest call command is: " + restCallCommand);
        return restCallCommand;
    }
    
    String buildRestCallCommand(String agentId, String callerSipUri, String calledNumber) {
        String restCallCommand = "https://" + plugin.getCallControllerConfig().getIpAddress() + ":" +
        plugin.getCallControllerConfig().getHttpPort() + "/callcontroller/"
        + callerSipUri.substring(0, callerSipUri.indexOf('@')) + "/"
        + calledNumber + "?agent=" + agentId;
        return restCallCommand;
    }

    void sendRestRequest(String url) {
        try {
            Request request = new Request(Method.POST, url);  

           // Handle it using an HTTP client connector  
            Client client = new Client(Protocol.HTTPS);  
            Response response = client.handle(request);  

            if ( response.getStatus().getCode() / 100  != 2) {
                log.error("sending the request to initiate the call failed code = " + response.getStatus().getCode());
            }
        } catch (Exception err) {
            log.error("rest caught: ", err);
           
        }
    }
}
