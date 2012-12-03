/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.plugin.presence;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.Collection;
import java.util.HashMap;
import java.util.Random;

import org.apache.log4j.Logger;
import org.jivesoftware.openfire.interceptor.PacketRejectedException;
import org.jivesoftware.openfire.muc.MUCRole;
import org.jivesoftware.openfire.muc.MUCRoom;
import org.jivesoftware.openfire.session.Session;
import org.jivesoftware.openfire.user.UserNotFoundException;
import org.jivesoftware.util.NotFoundException;
import org.xmpp.packet.JID;
import org.xmpp.packet.Message;
import org.xmpp.packet.Packet;

public class DefaultMessagePacketInterceptor extends AbstractMessagePacketInterceptor {
    private static Logger log = Logger.getLogger(DefaultMessagePacketInterceptor.class);
    private SipXOpenfirePlugin plugin;
    private int cookedMessageId = new Random().nextInt();
    HashMap<String, String> multiUserChatSubstitutionMessages = new HashMap<String, String>();

    private final static String CALL_DIRECTIVE = "@call";
    private final static String CONF_DIRECTIVE = "@conf";
    private final static String TRANSFER_DIRECTIVE = "@xfer";

    @Override
	public void start(SipXOpenfirePlugin plugin) {
        this.plugin = plugin;
    }

    @Override
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
        String restCallCommand =  "http://"+ plugin.getSipXopenfireConfig().getSipXrestIpAddress()
                + ":" + plugin.getSipXopenfireConfig().getSipXrestHttpPort() + "/callcontroller/"
                + callerNumber + "/" + calledNumber + "?timeout=30&isForwardingAllowed=true";
        log.debug("rest call command is: " + restCallCommand);
        return restCallCommand;
    }

    String buildRestCallCommand(String agentId, String caller, String calledNumber) {
        String restCallCommand =  "http://" + plugin.getSipXopenfireConfig().getSipXrestIpAddress()
                + ":" + plugin.getSipXopenfireConfig().getSipXrestHttpPort() + "/callcontroller/"
                + caller + "/" + calledNumber
                + "?agent=" + agentId + "&timeout=30&isForwardingAllowed=true";
        return restCallCommand;
    }

    String buildRestConferenceCommand(String agentId, String caller, String calledNumber, String conferencePin) {
        String restCallCommand =  "http://" + plugin.getSipXopenfireConfig().getSipXrestIpAddress()
                + ":" + plugin.getSipXopenfireConfig().getSipXrestHttpPort() + "/callcontroller/"
                + caller + "/" + calledNumber
                + "?agent=" + agentId + "&timeout=30&isForwardingAllowed=true";
        if (conferencePin != null && conferencePin.length() > 0 ){
            restCallCommand += "&confpin=" + conferencePin;
        }
        return restCallCommand;
    }



    /*
     * TODO : Convert this to use the REST client. Does not seem to work.
     */
    private static void sendRestRequest(String url) {

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
            if (incoming && !processed) {
                log.debug("message is: " + chatText);
                if (chatText.startsWith(CALL_DIRECTIVE) || chatText.startsWith(TRANSFER_DIRECTIVE)) {
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

                    if (chatText.startsWith(CALL_DIRECTIVE)) {

                        // check if there is an expression after @call
                        String expression = chatText.substring( CALL_DIRECTIVE.length() );
                        expression = expression.trim();
                        String numberToCall;
                        if (expression.length() > 0) {
                            // a name was specified - try to map it to a SIP ID that we can call
                            numberToCall = mapArbitraryNameToSipEndpoint( expression );
                            reply( message, plugin.getLocalizer().localize("attemptingcall.prompt") + " " + numberToCall);
                        } else {
                            // number to call was not specified, assume that the other
                            // end of the chat session is the party to call.
                            if ( toSipId != null ) {
                                numberToCall = toSipId;
                                changeMessageBody( message, plugin.getXmppDisplayName(message.getFrom().getNode()) + " " + plugin.getLocalizer().localize("callingyou.prompt") );
                            } else {
                                log.debug("no SIP ID associated with user " +
                                        message.getTo().toBareJID());
                                reply( message, plugin.getLocalizer().localize("commandfailed.prompt") +
                                                " - " +
                                                message.getTo().getNode() +
                                                " " +
                                                plugin.getLocalizer().localize("notassociatedwithsip.prompt") );
                                return;
                            }
                        }

                        String restCallCommand = buildRestCallCommand(fromSipId,
                                numberToCall);
                        sendRestRequest(restCallCommand);
                    } else if (chatText.startsWith(TRANSFER_DIRECTIVE)) {
                        if (toSipId != null) {
                            // check if there is an expression after @xfer
                            String expression = chatText.substring( TRANSFER_DIRECTIVE.length() );
                            expression = expression.trim();
                            String numberToCall;
                            if (expression.length() > 0) {
                                numberToCall = mapArbitraryNameToSipEndpoint( expression );
                                log.debug("xfer username is " + numberToCall);
                                String restCallCommand = buildRestCallCommand(fromSipId,
                                        toSipId, numberToCall);
                                sendRestRequest(restCallCommand);
                                changeMessageBody( message, plugin.getXmppDisplayName(message.getFrom().getNode()) +
                                                            " " +
                                                            plugin.getLocalizer().localize("isreferring.prompt") );
                            }
                            else{
                                reply( message, plugin.getLocalizer().localize("commandfailed.prompt") +
                                                " - " +
                                                plugin.getLocalizer().localize("xfertargetreq.prompt") );
                            }
                        }
                        else{
                            reply( message, plugin.getLocalizer().localize("commandfailed.prompt") +
                                            " - " +
                                            message.getTo().getNode() +
                                            " " +
                                            plugin.getLocalizer().localize("notassociatedwithsip.prompt"));
                        }
                    }
                }

            }
        }
    }

    private void processGroupChatMessage(Message message, boolean incoming, boolean processed) throws Exception
    {
        String chatText = message.getBody();
        if (chatText != null) {
            if (incoming && !processed) {
                if (chatText.startsWith(CONF_DIRECTIVE) ) {
                    if( message.getID() == null )
                    {
                        // Message IDs are optional be we need them to
                        // track data related to @conf directives.  If we
                        // find that a given message does not have an ID,
                        // we make one up and add it to the message.
                        message.setID(Integer.toString(cookedMessageId++));
                    }

                    log.debug("message is: " + chatText);
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
                            try
                            {
                                conferenceName = plugin.getConferenceName(subdomain, roomName);
                                // verify that the command issuer has the privilege to start the conference and
                                // has a SIP ID.
                                JID commandRequester = message.getFrom();
                                Collection<JID> owners = chatRoom.getOwners();
                                String commandRequesterSipId = plugin.getSipId(commandRequester.toBareJID());
                                if (commandRequesterSipId != null && owners.contains(commandRequester)) {
                                    String conferencePin = plugin.getConferencePin(subdomain, roomName);
                                    // Check who is to be invited to the conference.  If the @conf directive
                                    // is not followed by anything then everyone in the room will get invited.
                                    // If the directive is followed by an expression, only the SIP user mapping to that
                                    // expression will be invited.
                                    String expression = chatText.substring( CONF_DIRECTIVE.length() );
                                    expression = expression.trim();
                                    if (expression.length() > 0) {
                                        String numberToCall;
                                        numberToCall = mapArbitraryNameToSipEndpoint( chatRoom, expression );
                                        log.debug("@conf username is " + numberToCall);
                                        String restCallCommand = buildRestConferenceCommand(
                                                commandRequesterSipId, numberToCall, conferenceName, conferencePin);
                                        sendRestRequest(restCallCommand);
                                        reply( message, plugin.getLocalizer().localize("tryingtoinvite.prompt") +
                                                		" " +
                                                		numberToCall +
                                                		" " +
                                                		plugin.getLocalizer().localize("tothe.prompt") +
                                                        " " +
                                                        roomName +
                                                        " " +
                                                        plugin.getLocalizer().localize("audioconference.prompt"));
                                        throw new PacketRejectedException();
                                    }
                                    else{
                                        for (MUCRole occupant : chatRoom.getOccupants()) {
                                            if (occupant.getRole() != MUCRole.Role.none) {
                                                String occupantJID = occupant.getUserAddress().toBareJID();
                                                log.info( "occupantJID " + occupantJID);
                                                String mapKey = message.getID() + occupantJID;
                                                try{
                                                    String occupantSipId = plugin.getSipId(occupantJID);
                                                    if ( occupantSipId != null) {
                                                        String restCallCommand = buildRestConferenceCommand(
                                                                commandRequesterSipId, occupantSipId, conferenceName, conferencePin);
                                                        sendRestRequest(restCallCommand);
                                                        multiUserChatSubstitutionMessages.put(mapKey, plugin.getLocalizer().localize("invitetojoin.prompt") +
                                                                                                      " " +
                                                                                                      roomName +
                                                                                                      " " +
                                                                                                      plugin.getLocalizer().localize("audioconference.prompt") +
                                                                                                      " - " +
                                                                                                      plugin.getLocalizer().localize("willringshortly.prompt") );
                                                    }
                                                    else{
                                                        throw new UserNotFoundException("could not find SIP id for JID " + occupantJID );
                                                    }
                                                }
                                                catch( Exception ex ){
                                                    log.warn( "processGroupChatMessage " + ex + ": skipping user");
                                                    multiUserChatSubstitutionMessages.put(mapKey, message.getFrom().getNode() +
                                                                                            " " +
                                                                                            plugin.getLocalizer().localize("nosipaccount.prompt"));
                                                }
                                            }
                                        }
                                    }
                                }
                                else{
                                    // Not an owner; send back a message saying that command is not allowed
                                    log.debug(commandRequesterSipId + " is not the owner of MUC room " + subdomain + ":" + roomName);
                                    reply( message, plugin.getLocalizer().localize("notallowed.prompt") +
                                                    " - " +
                                                    plugin.getLocalizer().localize("onlyowners.prompt") +
                                                    " " +
                                                    roomName +
                                                    " " +
                                                    plugin.getLocalizer().localize("allowedtoperformoperation.prompt") );
                                    throw new PacketRejectedException(commandRequesterSipId + " is not the owner of MUC room " + subdomain + ":" + roomName);
                                }
                            }
                            catch( NotFoundException e )
                            {
                                // MUC not associated with audio conference room
                                log.debug("MUC room " + subdomain + ":" + roomName + " does not have an associated conference");
                                reply( message, plugin.getLocalizer().localize("commandfailed.prompt") +
                                        " - " +
                                        plugin.getLocalizer().localize("noaudioconf.prompt") );
                                throw new PacketRejectedException("MUC room " + subdomain + ":" + roomName + " does not have an associated conference");
                            }
                        }
                        else{
                            log.debug("MUC room " + subdomain + ":" + roomName + " not found");
                        }
                    } catch (Exception ex) {
                        log.debug("caught: " + ex.getMessage() + " while processing room chat message to " + message.getTo());
                    }
                }
            }
            else if (!incoming && !processed) {
                // check if we have an output message substitution stored for this message Id and destination.
                if( message.getID() != null ){
                    String mapKey =  message.getID() + message.getTo().toBareJID();
                    String messageSubstitution = multiUserChatSubstitutionMessages.get(mapKey);
                    if( messageSubstitution != null ){
                        changeMessageBody(message, messageSubstitution);
                        multiUserChatSubstitutionMessages.remove(mapKey);
                    }
                }
            }
        }
    }

    /*
     * Tries to convert a supplied name to a dialable SIP URI.  The conversion routine will look for the following:
     * #1- routine assumes that the name is the node of a JID and tries to map it to a SIP Id; if that fails
     * #2- routine assumes that the name is an XMPP display name and tries to map it to a JID then #1 is attempted; if that fails
     * #3- assume that the name is the userpart of the SIP URI.  Append the SIP domain to it
     */
    private String mapArbitraryNameToSipEndpoint( String name )
    {
        String sipEndpoint = null;
        // check if the supplied name has a domain part
        String[] result = name.split("@");
        if ( result.length == 1 ){
            // no domain name specified - try to map name to something we know how to dial
            // First, check if the name is an XMPP Display Name name that we can map to a SIP ID
            try{
                sipEndpoint = plugin.getSipIdFromXmppDisplayName(name);
            }
            catch( UserNotFoundException ex ){
                try{
                    // name was not an XMPP display name - check if it was an XMPP name
                    sipEndpoint = plugin.getSipId(name);
                }
                catch( UserNotFoundException ex2 ){
                    // name was not an XMPP username either - assume it is a SIP Id
                    sipEndpoint = name;
                }
            }
        }
        else{
            // the name contains a domain - if the user went through the trouble of typing
            // in a domain, chances are that this is a dialable URI from sipXecs.  Use the name as is.
            sipEndpoint = name;
        }
        log.debug("mapArbitraryNameToSipEndpoint " + name + " to " + sipEndpoint);
        return sipEndpoint;
    }

    /*
     * Tries to convert a supplied name to a dialable SIP URI.  The conversion routine will look for the following:
     * #1- routine assumes that the name is chatroom nickname (aka nick, alias or handle ) and tries to map it to a SIP Id; if that fails
     * #2- routine assumes that the name is the node of a JID and tries to map it to a SIP Id; if that fails
     * #3- routine assumes that the name is an XMPP display name and tries to map it to a JID then #1 is attempted; if that fails
     * #4- assume that the name is the userpart of the SIP URI.  Append the SIP domain to it
     */
    private String mapArbitraryNameToSipEndpoint( MUCRoom room, String name ){
        String sipEndpoint;
        try{
            MUCRole occupant = room.getOccupant( name );
            String occupantJID = occupant.getUserAddress().toBareJID();
            sipEndpoint = plugin.getSipId(occupantJID);
        }
        catch( Exception ex ){
            sipEndpoint = mapArbitraryNameToSipEndpoint( name );
        }
        return sipEndpoint;
    }

    // turns a message into a reply.  Note that this operation destroys the original message (i.e. the original
    // message will never be delivered to the original destination).
    private static void reply( Message message, String replyText ){
        JID from = message.getFrom();
        JID to   = message.getTo();
        message.setTo(from);
        message.setFrom(to);
        changeMessageBody(message, replyText);
    }

    private static void changeMessageBody( Message message, String newBodyText ){
        // the message can carry the chat text in two places:
        // #1 -[mandatory]- the message's body element carries the vanilla version of the chat text
        // #2 -[optional]- the message's html extension carries a style-enhanced version of the chat text
        // IM Clients that can handle html will render #2 in priority if present so both need to be modified.
        // Given that we do not want to fully parse and recreate the HTML, we simply remove the extension from the message.
        // This means that the message bodies generated by this message will not be formatted (i.e. will be plaintext)

        // address #1
        message.setBody(newBodyText);
        // address #2
        message.deleteExtension( "html", "http://jabber.org/protocol/xhtml-im");
    }
}
