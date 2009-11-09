/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.conference;

import java.util.HashMap;
import java.util.Vector;

import org.apache.commons.codec.digest.DigestUtils;

public class ConferenceBridgeItem {
    private String m_bridgename;
    private String m_bridgecontact;
    private String m_extensionname;
    private String m_ownername;
    private String m_ownerid;
    private String m_mboxserver;

    public String getBridgeName() {
        return m_bridgename;
    }

    public void setBridgeName(String bridgename) {
        m_bridgename = bridgename;
    }

    public String getBridgeContact() {
        return m_bridgecontact;
    }

    public void setBridgeContact(String bridgecontact) {
        m_bridgecontact = bridgecontact;
    }

    public String getExtensionName() {
        return m_extensionname;
    }

    public void setExtensionName(String extensionname) {
        m_extensionname = extensionname;
    }

    public String getOwnerName() {
        return m_ownername;
    }

    public void setOwnerName(String ownername) {
        m_ownername = ownername;
    }

    public String getOwnerId() {
        return m_ownerid;
    }

    public void setOwnerId(String ownerid) {
        m_ownerid = ownerid;
    }

    public String getMailboxServer() {
        return m_mboxserver;
    }

    public void setMailboxServer(String mboxserver) {
        m_mboxserver = mboxserver;
    }
}
