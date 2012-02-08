/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.address;

import org.apache.commons.lang.builder.EqualsBuilder;

public class Address {
    private String m_address;
    private int m_port;
    private AddressType m_type;

    public Address(AddressType t) {
        m_type = t;
    }

    public Address(AddressType t, String address) {
        this(t);
        m_address = address;
    }

    public Address(AddressType t, String address, int port) {
        this(t, address);
        m_port = port;
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
        return m_type.format(this);
    }

    @Override
    public int hashCode() {
        return super.hashCode();
    }

    @Override
    public boolean equals(Object obj) {
        if (obj == null) {
            return false;
        }
        if (obj == this) {
            return true;
        }
        if (obj.getClass() != getClass()) {
            return false;
        }
        Address rhs = (Address) obj;
        return new EqualsBuilder().append(m_address, rhs.m_address)
                .append(m_port, rhs.m_port).append(m_type, rhs.m_type).isEquals();

    }
}
