/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.vcard.provider;

import org.apache.log4j.Logger;
import org.dom4j.Element;
import org.jivesoftware.openfire.vcard.VCardManager;
import org.sipfoundry.openfire.vcard.synchserver.ContactInfoHandler;
import org.sipfoundry.openfire.vcard.synchserver.Util;

public class ContactInfoHandlerImp implements ContactInfoHandler {
    private static Logger logger = Logger.getLogger(ContactInfoHandlerImp.class);

    @Override
    public void notifyContactChange(String userName) {
        if (VCardManager.getProvider() instanceof SipXVCardProvider) {
            logger.info("Contact Change Notification received for user " + userName);
            SipXVCardProvider prov = (SipXVCardProvider) VCardManager.getProvider();
            Element vCard = prov.cacheVCard(userName);
            try {
                VCardManager.getInstance().reset();
                Util.updateAvatar(userName, vCard);
                // Sending announcement to the client
                Util.notify(userName);
            } catch (Exception e) {
                logger.error("In ContactInfoHandlerImp set/update VCard failed! " + e.getMessage());
            }
        }
    }
}
