/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.dns;

public class DnsRecord {
    private String m_address;
    private int m_port;

    public DnsRecord(String address, int port) {
        m_address = address;
        m_port = port;
    }

    public String getAddress() {
        return m_address;
    }

    public int getPort() {
        return m_port;
    }
}
