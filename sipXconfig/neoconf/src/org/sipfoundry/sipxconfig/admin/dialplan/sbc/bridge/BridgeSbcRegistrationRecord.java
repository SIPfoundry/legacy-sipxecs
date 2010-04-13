/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan.sbc.bridge;

import java.io.Serializable;

public class BridgeSbcRegistrationRecord implements Serializable {

    private static final long serialVersionUID = 1L;

    private String m_registeredAddress;
    private String m_registrationStatus;

    public BridgeSbcRegistrationRecord(String registeredAddress, String registrationStatus) {
        m_registeredAddress = registeredAddress;
        m_registrationStatus = registrationStatus;
    }

    /**
     * @return the registeredAddress
     */
    public String getRegisteredAddress() {
        return m_registeredAddress;
    }

    /**
     * @return the registrationStatus
     */
    public String getRegistrationStatus() {
        return m_registrationStatus;
    }
}
