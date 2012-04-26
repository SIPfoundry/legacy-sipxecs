/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.address;

import org.apache.commons.collections.Transformer;
import org.apache.commons.lang.builder.EqualsBuilder;

public class Address {
    public static final Transformer GET_IP = new Transformer() {
        public Object transform(Object o) {
            return (o == null ? null : ((Address) o).getAddress());
        }
    };
    private String m_address;
    private int m_port;
    private int m_endPort;
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

    /**
     * If an address is using standard port for that protocol, getPort will be 0 but the canonical port
     * will be the actual standard port number.
     * Example:
     *   DNS address, getPort() returns 0. getCanonicalPort() returns 53
     *
     * @return
     */
    public int getCanonicalPort() {
        return (m_port == 0 ? m_type.getCanonicalPort() : m_port);
    }

    public void setPort(int port) {
        this.m_port = port;
    }

    public String toString() {
        return m_type.format(this);
    }

    /**
     * @return address:port  or just address if port is 0
     */
    public String addressColonPort() {
        return m_port == 0 ? m_address : m_address + ':' + m_port;
    }

    /**
     * If there is a protocol at the front of toString, remove it
     *
     * foo:2  -> foo:2
     * http://foo:2/x/y  -> foo:2/x/y
     * sip:foo:2;a=b  -> foo:2;a=b
     *
     * @return
     */
    public String stripProtocol() {
        return toString().replaceFirst("^\\w{1,}:/{0,2}", "");
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

    public AddressType getType() {
        return m_type;
    }

    public int getEndPort() {
        return m_endPort;
    }

    public void setEndPort(int endPort) {
        m_endPort = endPort;
    }
}
