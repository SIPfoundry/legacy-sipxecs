/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

public class SipxParkService extends SipxService {
    public static final String BEAN_ID = "sipxParkService";

    private String m_parkServerSipPort;

    public String getParkServerSipPort() {
        return m_parkServerSipPort;
    }

    public void setParkServerSipPort(String parkServerSipPort) {
        m_parkServerSipPort = parkServerSipPort;
    }
}
