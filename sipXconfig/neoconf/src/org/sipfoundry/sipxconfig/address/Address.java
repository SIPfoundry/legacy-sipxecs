/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.address;

import static java.lang.String.format;

public class Address {
    private String m_address;
    private int m_port;
    private String m_format = "%s:%d";

    public Address() {
    }

    public Address(String address) {
        m_address = address;
    }

    public Address(String address, int port) {
        m_address = address;
        m_port = port;
    }

    public Address(String address, int port, String format) {
        m_address = address;
        m_port = port;
        m_format = format;
    }

    public String getAddress() {
        return m_address;
    }

    public void setAddress(String address) {
        this.m_address = address;
    }

    public int getPort() {
        return m_port;
    }

    public void setPort(int port) {
        this.m_port = port;
    }

    public String toString() {
        return format(m_format, m_address, m_port);
    }

    public String getFormat() {
        return m_format;
    }

    public void setFormat(String format) {
        m_format = format;
    }
}
