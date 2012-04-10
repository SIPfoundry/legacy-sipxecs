/**
 *
 *
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
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
