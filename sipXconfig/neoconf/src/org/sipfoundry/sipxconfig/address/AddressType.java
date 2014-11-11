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

public class AddressType {
    private static final String SIP_FORMAT = "sip:%s:%d";
    private final String m_id;
    private String m_label;
    private String m_format;
    private Protocol m_protocol = Protocol.tcp;
    private int m_canonicalPort;
    private boolean m_external;
    private boolean m_sip;

    /**
     * There are a lot more types (see /etc/protocol) but these are the only imaginable ones
     * we need to support.  This is not for application level protocols like SMTP or SIP, but
     * rather network protocols.  Ids here go straight into iptables config so unrecognized
     * ones will break firewall support.
     */
    public enum Protocol {
        tcp, udp, tlsp, icmp, tcp_udp
    }

    public AddressType(String uniqueId) {
        m_id = uniqueId;
    }

    public AddressType(String uniqueId, Protocol protocol) {
        this(uniqueId);
        m_protocol = protocol;
    }

    public AddressType(String uniqueId, String format) {
        this(uniqueId);
        m_format = format;
    }

    public AddressType(String uniqueId, String format, Protocol protocol) {
        this(uniqueId, format);
        m_protocol = protocol;
    }

    public AddressType(String uniqueId, String format, Protocol protocol, boolean externalSip) {
        this(uniqueId, format);
        m_protocol = protocol;
        m_external = externalSip;
        m_sip = externalSip;
    }

    public AddressType(String uniqueId, String format, int canonicalPort) {
        this(uniqueId, format);
        m_canonicalPort = canonicalPort;
    }

    public AddressType(String uniqueId, String format, int canonicalPort, Protocol protocol) {
        this(uniqueId, format, canonicalPort);
        m_protocol = protocol;
    }

    public AddressType(String uniqueId, int canonicalPort) {
        this(uniqueId);
        m_canonicalPort = canonicalPort;
    }

    public AddressType(String uniqueId, int canonicalPort, Protocol protocol) {
        this(uniqueId, canonicalPort);
        m_protocol = protocol;
    }

    /**
     * Convenience method to format address as a sip type address
     */
    public static AddressType sipTcp(String uniqueId) {
        return sip(uniqueId, Protocol.tcp);
    }

    public static AddressType externalSipTcp(String uniqueId) {
        return externalsip(uniqueId, Protocol.tcp);
    }

    public static AddressType sipUdp(String uniqueId) {
        return sip(uniqueId, Protocol.udp);
    }

    public static AddressType externalSipUdp(String uniqueId) {
        return externalsip(uniqueId, Protocol.udp);
    }

    public static AddressType sipTls(String uniqueId) {
        return sip(uniqueId, Protocol.tlsp);
    }

    public static AddressType externalSipTls(String uniqueId) {
        return externalsip(uniqueId, Protocol.tlsp);
    }

    public static AddressType sip(String uniqueId, Protocol protocol) {
        return new AddressType(uniqueId, SIP_FORMAT, protocol);
    }

    public static AddressType externalsip(String uniqueId, Protocol protocol) {
        return new AddressType(uniqueId, SIP_FORMAT, protocol, true);
    }

    public String getId() {
        return m_id;
    }

    public boolean isExternal() {
        return m_external;
    }

    public boolean isExternalSip() {
        return m_external && m_sip;
    }

    public String format(Address address) {
        if (m_format != null) {
            return String.format(m_format, address.getAddress(), address.getPort());
        }
        return address.getPort() == 0 ? address.getAddress() : address.getAddress() + ':' + address.getPort();
    }

    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof AddressType)) {
            return false;
        }
        if (this == obj) {
            return true;
        }
        AddressType rhs = (AddressType) obj;
        return m_id.equals(rhs.m_id);
    }

    @Override
    public int hashCode() {
        return m_id.hashCode();
    }

    public boolean equalsAnyOf(AddressType... types) {
        for (AddressType type : types) {
            if (this.equals(type)) {
                return true;
            }
        }
        return false;
    }

    public String getFormat() {
        return m_format;
    }

    public int getCanonicalPort() {
        return m_canonicalPort;
    }

    public Protocol getProtocol() {
        return m_protocol;
    }

    public void setProtocol(Protocol protocol) {
        m_protocol = protocol;
    }

    public String getLabel() {
        return m_label;
    }

    public void setLabel(String label) {
        m_label = label;
    }

    @Override
    public String toString() {
        return "AddressType [m_id=" + m_id + ", m_format=" + m_format + ", m_protocol=" + m_protocol
                + ", m_canonicalPort=" + m_canonicalPort + ", m_sip=" + m_sip
                + ", m_external=" + m_external + "]";
    }
}
