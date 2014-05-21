/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
