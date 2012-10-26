/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.openfire.sqa;

import static org.apache.commons.lang.StringUtils.substringBefore;

import ietf.params.xml.ns.dialog_info.DialogInfo;

import java.io.StringReader;
import java.util.Map;

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBException;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.jivesoftware.openfire.XMPPServer;
import org.jivesoftware.openfire.user.UserNotFoundException;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.commons.util.UnfortunateLackOfSpringSupportFactory;
import org.sipfoundry.sqaclient.SQAEvent;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.xmpp.packet.JID;
import org.xmpp.packet.Message;
import org.xmpp.packet.Presence;


public class SqaEventHandler implements Runnable {
    private final JAXBContext m_context;
    private final SQAEvent m_event;
    private final XMPPServer m_server = XMPPServer.getInstance();
    private static final Logger logger = LoggerFactory.getLogger(SqaEventHandler.class);
    ValidUsers m_users = UnfortunateLackOfSpringSupportFactory.getValidUsers();
    Map<String, SipPresenceBean> m_presenceCache;

    public SqaEventHandler(SQAEvent event, JAXBContext context, Map<String, SipPresenceBean> presenceCache) {
        m_event = event;
        m_context = context;
        m_presenceCache = presenceCache;
    }

    @Override
    public void run() {
        String data = m_event.getData();
        StringReader reader = new StringReader(data);
        logger.debug("Dialog Event Data: "+data);
        try {
            //Remove any unwanted trailing characters
            data = substringBefore(data, "</dialog-info>").concat("</dialog-info>\n");
            DialogInfo dialogInfo = (DialogInfo) m_context.createUnmarshaller().unmarshal(new StringReader(data));
            logger.debug("dialog-info: "+dialogInfo.toString());

            SipEventBean bean = new SipEventBean(dialogInfo);
            String sipId = bean.getCallerId();
            String targetSipId = bean.getCalleeId();
            logger.debug("Target SIP Id (callee) "+targetSipId);
            String observerId = bean.getObserverId();
            User user = m_users.getUser(sipId);
            User targetUser = targetSipId != null ? m_users.getUser(targetSipId) : null;
            User observerUser = m_users.getUser(observerId);
            String targetJid = targetUser != null ? targetUser.getJid() : null;
            String userJid = user != null ? user.getJid() : null;
            User observerPartyUser = targetUser;
            String observerPartyId = targetSipId;

            if (!StringUtils.equals(sipId, observerId)) {
                observerPartyUser = user;
                observerPartyId = sipId;
            }

            if (observerUser != null) {
                boolean trying = bean.isTrying();
                boolean ringing = bean.isRinging();
                boolean confirmed = bean.isConfirmed();
                boolean terminated = bean.isTerminated();
                JID observerJID = m_server.createJID(observerUser.getJid(), null);

                logger.debug("Initiator: " + userJid);
                logger.debug("Recipient: " + targetJid);
                logger.debug("Observer status: trying: " + trying + " confirmed: " + confirmed + " terminated: " + terminated + " ringing: " + ringing);

                if (observerJID != null) {
                    logger.debug("Observer JID (IM Entity that sent the message): " + observerJID.getNode());
                    org.jivesoftware.openfire.user.User ofObserverUser = m_server.getUserManager().getUser(observerJID.getNode());
                    Presence presence = m_server.getPresenceManager().getPresence(ofObserverUser);

                    String body = null;
                    if (ringing) {
                        if (StringUtils.equals(observerId, targetSipId)) {
                            body = Utils.getDisplayName(user, sipId) + " calls you: your phone is ringing";
                        }
                    } else if (confirmed) {
                        if (StringUtils.equals(observerId, targetSipId)) {
                            body = "Call established with: " + Utils.getDisplayName(user, sipId);
                        }
                        SipPresenceBean previousPresenceBean = m_presenceCache.get(observerJID.getNode());
                        //if cache is not cleared, than this is a new incomming call, do not broadcast on the call status again
                        if (previousPresenceBean == null && presence != null) {
                            logger.debug("ObserverPartyUser " + observerPartyUser + " observer party id " + observerPartyId);
                            String presenceMessage = Utils.generateXmppStatusMessageWithSipState(observerUser, observerPartyUser, presence, observerPartyId);
                            //presence status is about to be changed - save current presence
                            if (presenceMessage != null) {
                                m_presenceCache.put(observerJID.getNode(), new SipPresenceBean(presence.getStatus(), observerPartyId));
                                Utils.setPresenceStatus(ofObserverUser, presence, presenceMessage);
                            }
                        }
                    } else if (terminated) {
                        if (StringUtils.equals(observerId, targetSipId)) {
                            body = "Call with: " + Utils.getDisplayName(user, sipId) + " is terminated";
                        }
                        SipPresenceBean previousPresenceBean = m_presenceCache.get(observerJID.getNode());
                        if(previousPresenceBean != null) {
                            Utils.setPresenceStatus(ofObserverUser, presence, previousPresenceBean.getStatusMessage());
                            m_presenceCache.remove(observerJID.getNode());
                        }
                    }
                    if (body != null && targetJid != null) {
                        JID targetJID = m_server.createJID(targetJid, null);
                        if (targetJID != null) {
                            JID mybuddyJID = m_server.createJID(m_users.getImBotName(), null);
                            sendMessage(mybuddyJID, targetJID, body);
                        }
                    }
                }
            }
        } catch (JAXBException e) {
            logger.error("Cannot parse packet: data ", e);
        } catch (UserNotFoundException e) {
            logger.error("Cannot parse packet: data ", e);
        } finally {
            IOUtils.closeQuietly(reader);
        }
    }

    private void sendMessage(JID from, JID to, String body) {
        Message message = new Message();
        message.setFrom(from);
        message.setTo(to);
        message.setBody(body);
        message.setType(Message.Type.chat);
        m_server.getMessageRouter().route(message);
    }
}

