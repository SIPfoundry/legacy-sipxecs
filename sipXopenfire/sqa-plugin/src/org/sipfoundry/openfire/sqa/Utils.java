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

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.commons.userdb.User;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.xmpp.packet.Presence;

public class Utils {
    private static final Logger logger = LoggerFactory.getLogger(Utils.class);

    public static String getDisplayName(User user, String sipId) {
        if (user == null) {
            return sipId;
        }
        String displayName = user.getDisplayName();
        String nameToReturn =  displayName == null ? user.getJid() : displayName;
        return nameToReturn == null ? sipId : nameToReturn;
    }

    //@returns: null if the user does not have an 'on the phone' message
    public static String generateXmppStatusMessageWithSipState(User user, User callingParty, Presence presence, String callingPartyId) {
        String xmppStatusMessageWithSipState = null;
        String currentStatusMessage = presence.getStatus();
        try {
            if (user.isAdvertiseOnCallStatus()) {
                xmppStatusMessageWithSipState = user.getOnthePhoneMessage();
                if (xmppStatusMessageWithSipState != null) {
                    if (user.isShowOnCallDetails()) {
                        xmppStatusMessageWithSipState += " (" + getDisplayName(callingParty, callingPartyId) + ") ";
                    }
                    if (!StringUtils.isEmpty(currentStatusMessage)) {
                        xmppStatusMessageWithSipState += " - " + currentStatusMessage;
                    }
                }
            }
        } catch (Exception e) {
            logger.error("SqaEventHandler: " + getDisplayName(user, callingPartyId) + ":", e);
        }
        return xmppStatusMessageWithSipState;
    }
}
