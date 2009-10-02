package org.sipfoundry.openfire.plugin.presence;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.Collection;

import org.apache.log4j.Logger;
import org.jivesoftware.openfire.XMPPServer;
import org.jivesoftware.openfire.interceptor.PacketInterceptor;
import org.jivesoftware.openfire.interceptor.PacketRejectedException;
import org.jivesoftware.openfire.muc.MUCRole;
import org.jivesoftware.openfire.muc.MUCRoom;
import org.jivesoftware.openfire.session.Session;
import org.jivesoftware.openfire.PacketDeliverer; 
import org.restlet.Client;
import org.restlet.data.Method;
import org.restlet.data.Protocol;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.Representation;
import org.xmpp.packet.Message;
import org.xmpp.packet.Packet;
import org.xmpp.packet.JID;


public class MessagePacketInterceptor implements PacketInterceptor {
    private static Logger log = Logger.getLogger("org.sipfoundry");
    private SipXOpenfirePlugin plugin;

    MessagePacketInterceptor(SipXOpenfirePlugin plugin) {
        this.plugin = plugin;
    }
 
    public void interceptPacket(Packet packet, Session session, boolean incoming,
            boolean processed) throws PacketRejectedException {
        try {
            if (packet instanceof Message) {
                Message message = (Message) packet;
                if (message.getType() == Message.Type.chat) {
                    processChatMessage(message, incoming, processed);
                }
                else if (message.getType() == Message.Type.groupchat) {
                    processGroupChatMessage(message, incoming, processed);                    
                }
            }
        } catch (PacketRejectedException e) {
            throw new PacketRejectedException(e);
        } catch (Exception e) {
            log.debug("Caught: '" + e.getMessage());
            e.printStackTrace(System.err);
        }
    }

    String buildRestCallCommand(String callerNumber, String calledNumber) {
        String restCallCommand =  "https://"+ plugin.getSipXopenfireConfig().getSipXrestIpAddress()
                + ":" + plugin.getSipXopenfireConfig().getSipXrestHttpsPort() + "/callcontroller/"
                + callerNumber + "/" + calledNumber;
        log.debug("rest call command is: " + restCallCommand);
        return restCallCommand;
    }

    String buildRestCallCommand(String agentId, String caller, String calledNumber) {
        String restCallCommand =  "https://" + plugin.getSipXopenfireConfig().getSipXrestIpAddress()
                + ":" + plugin.getSipXopenfireConfig().getSipXrestHttpsPort() + "/callcontroller/"
                + caller + "/" + calledNumber
                + "?agent=" + agentId;
        return restCallCommand;
    }

    String buildRestConferenceCommand(String agentId, String caller, String calledNumber, String conferencePin) {
        String restCallCommand =  "https://" + plugin.getSipXopenfireConfig().getSipXrestIpAddress()
                + ":" + plugin.getSipXopenfireConfig().getSipXrestHttpsPort() + "/callcontroller/"
                + caller + "/" + calledNumber
                + "?agent=" + agentId;
        if (conferencePin != null && conferencePin.length() > 0 ){
            restCallCommand += "&confpin=" + conferencePin;
        }
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
    
    private void processChatMessage(Message message, boolean incoming, boolean processed) throws Exception{
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

    private void processGroupChatMessage(Message message, boolean incoming, boolean processed) throws Exception{
        log.debug("Chat message in:" + message + " incoming=" + incoming + "; processed=" + processed);
        
        String chatText = message.getBody();
        if (chatText != null) {
            if (incoming && !processed) {
                if (chatText.startsWith("@conf")) {
                    // check if the message is in the user->chat room direction.
                    log.debug("conference command detected: " + chatText);
                    log.debug("from : " + message.getFrom());
                    log.debug("from node: " + message.getFrom().toBareJID());
                    log.debug("to : " + message.getTo());
                    log.debug("to node: " + message.getTo().toBareJID());
                    try{
                        // next check whether or not the chat room is associated with
                        // a conference bridge
                        String domain = message.getTo().getDomain();
                        String subdomain = domain.substring(0, domain.indexOf('.'));
                        String roomName = message.getTo().getNode();
                        MUCRoom chatRoom;
                        if ((chatRoom = plugin.getChatRoom(subdomain, roomName)) != null) {
                            // check if the chat room is associated with a conference bridge
                            String conferenceName;
                            if ((conferenceName = plugin.getConferenceName(subdomain, roomName)) != null) {
                                // verify that the command issuer has the privilege to start the conference and 
                                // has a SIP ID.
                                String commandRequester = message.getFrom().toBareJID();
                                Collection<String> owners = chatRoom.getOwners();
                                String commandRequesterSipId = plugin.getSipId(commandRequester); 
                                if (commandRequesterSipId != null && owners.contains(commandRequester)) {
                                    // INVITE into the conference every room occupant that 
                                    // has an associated SIP ID.
                                    String conferencePin = plugin.getConferencePin(subdomain, roomName);
                                    for (MUCRole occupant : chatRoom.getOccupants()) {
                                        if (occupant.getRole() != MUCRole.Role.none) {
                                            String occupantJID = occupant.getUserAddress().toBareJID();
                                            String occupantSipId = plugin.getSipId(occupantJID);
                                            if ( occupantSipId != null) {
                                                String restCallCommand = buildRestConferenceCommand(
                                                        commandRequesterSipId, occupantSipId, conferenceName, conferencePin);
                                                sendRestRequest(restCallCommand);                                                    
                                            }
                                        }
                                    }
                                }
                                else{
                                    // Not an owner; send back a message saying that command is not allowed
                                    log.debug(commandRequesterSipId + "is not the owner of MUC room " + subdomain + ":" + roomName);
                                    JID from = message.getFrom();
                                    JID to   = message.getTo();
                                    message.setTo(from);
                                    message.setFrom(to);
                                    message.setBody("NOT ALLOWED: Only the owners of the " + roomName + " chatroom are allowed to perform this operation");
                                    throw new PacketRejectedException(commandRequesterSipId + " is not the owner of MUC room " + subdomain + ":" + roomName);
                                }
                            }
                            log.debug("MUC room " + subdomain + ":" + roomName + " does not have an associated conference");
                        }
                        else{
                            log.debug("MUC room " + subdomain + ":" + roomName + " not found");
                        }
                    } catch (Exception ex) {
                        log.debug("caught: " + ex.getMessage() + " while processing room chat message to " + message.getTo());
                    }
                }                    
            }
        }
    }    

}
