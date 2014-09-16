/**
 *
 *
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.openfire.plugin.presence;

import org.apache.log4j.Logger;
import org.jivesoftware.openfire.interceptor.PacketRejectedException;
import org.jivesoftware.openfire.session.Session;
import org.sipfoundry.commons.log4j.SipFoundryAppender;
import org.sipfoundry.commons.log4j.SipFoundryLayout;
import org.xmpp.packet.Message;
import org.xmpp.packet.Packet;

public class ImLogger extends AbstractMessagePacketInterceptor {
    private static Logger imLogger;
    private static Logger log = Logger.getLogger(ImLogger.class);
    private final String imMessagesLogDirectory;

    ImLogger(String imMessagesLogDirectory) {
        this.imMessagesLogDirectory = imMessagesLogDirectory;
    }

    @Override
    public void start(SipXOpenfirePlugin plugin) {
        try {
            String logFile = this.imMessagesLogDirectory + "/sipxopenfire-im.log";
            imLogger = Logger.getLogger("ImLogger");
            imLogger.addAppender(new SipFoundryAppender(new SipFoundryLayout(), logFile));
            imLogger.setLevel(org.apache.log4j.Level.INFO);
            imLogger.info(">>>>>>starting<<<<<<");
        } catch (Exception ex) {
            log.info("caught ", ex);
        }
    }

    @Override
    public void interceptPacket(Packet packet, Session session, boolean incoming, boolean processed)
            throws PacketRejectedException {
        try {
            if (packet instanceof Message) {
                Message message = (Message) packet;
                logIm(message, incoming, processed);
            }
        } catch (Exception e) {
            log.debug("Caught: '" + e.getMessage(), e);
        }
    }

    private static void logIm(Message message, boolean incoming, boolean processed) {
        if (imLogger != null) {
            // we only log chat and multichat messages - get out
            // if we have anything different.
            if (message.getType() == Message.Type.chat || message.getType() == Message.Type.groupchat) {
                // check if the message has a text body
                if (message.getBody() != null) {
                    // we log messages that arrive before we process them
                    // and messages that leave after when have processed them.
                    String direction;
                    if (incoming && !processed) {
                        direction = ":------->INCOMING<----------:";
                    } else if (!incoming && processed) {
                        direction = ":------->OUTGOING<----------:";
                    } else {
                        return;
                    }
                    imLogger.info(direction + message);
                }
            }
        }
    }
}
