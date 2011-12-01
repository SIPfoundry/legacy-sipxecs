/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.faxrx;

import java.util.Hashtable;

import org.sipfoundry.commons.freeswitch.FaxReceive;
import org.sipfoundry.sipxivr.eslrequest.AbstractEslRequestController;

public class FaxRxEslRequestController extends AbstractEslRequestController {

    private String m_mailboxid;

    @Override
    public void extractParameters(Hashtable<String, String> parameters) {
        m_mailboxid = parameters.get("mailbox");
    }

    @Override
    public void loadConfig() {
    }

    public void linger() {
        getFsEventSocket().cmdResponse("linger");
    }

    public FaxReceive receiveFax(String path) {
        FaxReceive faxReceive = new FaxReceive(getFsEventSocket(), path);
        faxReceive.go();
        return faxReceive;
    }

    public String getMailboxId() {
        return m_mailboxid;
    }

}
