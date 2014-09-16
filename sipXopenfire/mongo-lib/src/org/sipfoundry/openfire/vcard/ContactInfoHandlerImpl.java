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
package org.sipfoundry.openfire.vcard;

import org.apache.log4j.Logger;
import org.dom4j.Element;
import org.jivesoftware.openfire.provider.VCardProvider;
import org.jivesoftware.openfire.vcard.VCardManager;
import org.sipfoundry.openfire.vcard.synchserver.ContactInfoHandler;
import org.sipfoundry.openfire.vcard.synchserver.Util;

public class ContactInfoHandlerImpl implements ContactInfoHandler {
    private static Logger logger = Logger.getLogger(ContactInfoHandlerImpl.class);

    @Override
    public void notifyContactChange(String userName) {
        VCardProvider provider = VCardManager.getProvider();
        Element userVCard = provider.loadVCard(userName);
        try {
            logger.debug("Start synchronizing vcard");
            VCardManager.getInstance().setVCard(userName, userVCard);
            Util.updateAvatar(userName, userVCard);
            // Sending announcement to the client
            Util.notify(userName);
            logger.debug("Finished synchronizing vcard");
        } catch (Exception e) {
            logger.error("Cannot synchronize VCard for: " + userName);
        }
    }
}
