package org.sipfoundry.openfire.plugin.presence;

import java.io.BufferedReader;
import java.io.InputStreamReader;

import org.apache.log4j.Logger;
import org.jivesoftware.openfire.XMPPServer;
import org.jivesoftware.openfire.interceptor.PacketInterceptor;
import org.jivesoftware.openfire.interceptor.PacketRejectedException;
import org.jivesoftware.openfire.session.Session;
import org.restlet.Client;
import org.restlet.data.Method;
import org.restlet.data.Protocol;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.Representation;
import org.xmpp.packet.Message;
import org.xmpp.packet.Packet;

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
                                if (fromSipId == null) {
                                    log.debug("fromSipId is null for " +message.getFrom().toBareJID() + 
                                            " user does not have an associated SIP URL -- not handling call");
                                    return;
                                }
                                String toSipId = plugin.getSipId(message.getTo().toBareJID());
                                log.debug(fromSipId + ":" + toSipId);

                                if (chatText.startsWith("@call")) {

                                    // check if there is a phone number after @call
                                    String[] result = chatText.split("\\s");
                                    String numberToCall;
                                    if (result.length >= 2) {
                                        // TODO: turn this next block of code into a function
                                        String username = null;
                                        if ( result[1].indexOf("@") == -1 ) {
                                            username = result[1]
                                                + "@"
                                                + XMPPServer.getInstance().getServerInfo()
                                                        .getXMPPDomain();
                                        } else {
                                            username = result[1];
                                        }
                                        log.debug("call username is " + username);

                                        // Assume that the target of the call is an XMPP id.
                                        // Try to resolve this to a sip id.
                                        String calledSipId = null;
                                        try {
                                            calledSipId = plugin.getSipId(username);
                                        } catch (Exception e) {
                                            log.debug("caught: " + e.getMessage());
                                        }
                                        log.debug("calledSipId " + calledSipId);

                                        // The called ID is not an XMPP id. Then this must be
                                        // a SIP id directly specified.
                                        if (calledSipId != null) {
                                            numberToCall = calledSipId;
                                        } else {
                                            numberToCall = username;
                                        }
                                    } else {
                                        if ( toSipId != null ) {
                                            numberToCall = toSipId;
                                        } else {
                                            log.debug("no SIP ID associated with user " + 
                                                    message.getTo().toBareJID());
                                            return;
                                        }
                                    }

                                    String restCallCommand = buildRestCallCommand(fromSipId,
                                            numberToCall);
                                    sendRestRequest(restCallCommand);

                                } else if (chatText.startsWith("@xfer")) {
                                    if (toSipId != null) {
                                        // check if there is a phone number after @call
                                        String[] result = chatText.split("\\s");
                                        String numberToCall;
                                        if (result.length >= 2) {
                                            // figure out what that number is...
                                            // First, check if it is the name part of a JID on the
                                            // system
                                            // that is mapped to a SIP ID.
                                            String username = null;
                                            if ( result[1].indexOf("@") == -1 ) {
                                                username = result[1]
                                                    + "@"
                                                    + XMPPServer.getInstance().getServerInfo()
                                                            .getXMPPDomain();
                                            } else {
                                                username = result[1];
                                            }
                                            
                                           
                                            log.debug("xfer username is " + username);
                                            String calledSipId = null;
                                            // Assume that the target of the call is an XMPP id.
                                            // Try to resolve this to a sip id.
                                            try {
                                                calledSipId = plugin.getSipId(username);
                                            } catch (Exception e) {
                                                log.debug("caught: " + e.getMessage());
                                            }
                                            log.debug("calledSipId " + calledSipId);

                                            // The called ID is not an XMPP id. Then this must be
                                            // a SIP id directly specified.
                                            if (calledSipId != null) {
                                                numberToCall = calledSipId;
                                            } else {
                                                numberToCall = username;
                                            }
                                            
                                            
                                        } else {
                                            if ( toSipId != null ) {
                                                numberToCall = toSipId;
                                            } else {
                                                log.debug("no SIP ID associated with user " + 
                                                        message.getTo().toBareJID());
                                                return;
                                            }
                                        }
                                       
                                        String restCallCommand = buildRestCallCommand(fromSipId,
                                                toSipId, numberToCall);
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

    String buildRestCallCommand(String callerNumber, String calledNumber) {
        String restCallCommand =  "https://"+ plugin.getCallControllerConfig().getIpAddress()
                + ":" + plugin.getCallControllerConfig().getHttpPort() + "/callcontroller/"
                + callerNumber + "/" + calledNumber;
        log.debug("rest call command is: " + restCallCommand);
        return restCallCommand;
    }

    String buildRestCallCommand(String agentId, String caller, String calledNumber) {
        String restCallCommand =  "https://" + plugin.getCallControllerConfig().getIpAddress()
                + ":" + plugin.getCallControllerConfig().getHttpPort() + "/callcontroller/"
                + caller + "/" + calledNumber
                + "?agent=" + agentId;
        return restCallCommand;
    }

    
    /*
     * TODO : Convert this to use the REST client. Does not seem to work.
     */
    void sendRestRequest(String url) {

        try {
            String command = "curl -k -X POST " + url;
            log.debug(command);
            String line;
            Process p = Runtime.getRuntime().exec(command);
            log.debug(command);
            BufferedReader input = new BufferedReader(new InputStreamReader(p.getErrorStream()));
            while ((line = input.readLine()) != null) {
                log.debug("curl:" + line);
            }
            input.close();
        } catch (Exception err) {
            log.debug("rest caught: " + err.getMessage());
            err.printStackTrace();
        }

    }
}
