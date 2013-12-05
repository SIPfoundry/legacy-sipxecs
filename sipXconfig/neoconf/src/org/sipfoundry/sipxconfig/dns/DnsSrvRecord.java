/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.dns;

import org.apache.commons.lang.StringUtils;
import org.codehaus.jackson.annotate.JsonPropertyOrder;

// order is
@JsonPropertyOrder(alphabetic = true)
public class DnsSrvRecord {
    private String m_host;
    private int m_priority = 30;
    private int m_weight = 10;
    private String m_destination;
    private String m_protocol;
    private String m_resource;
    private int m_port;
    private boolean m_internal;

    public DnsSrvRecord(String protocol, String resource, String host, int port, String destination) {
        m_destination = destination;
        m_protocol = protocol;
        m_resource = resource;
        m_host = host;
        m_port = port;
    }

    /**
     * Useful for external traditional SRV records e.g. _sip._tcp.example.org
     */
    public static final DnsSrvRecord domainLevel(String protocol, String resource, int port, String destination) {
        return new DnsSrvRecord(protocol, resource, "", port, destination);
    }

    /**
     * Useful for internal "RR" records that select a local host first e.g. _sip._tcp.host1.example.org
     */
    public static final DnsSrvRecord hostLevel(String protocol, String resource, String host, int port,
            String destination) {
        return new DnsSrvRecord(protocol, resource, host, port, destination);
    }

    public String getLeftHandSide() {
        StringBuilder sb = new StringBuilder(m_protocol);
        if (StringUtils.isNotBlank(m_resource)) {
            sb.append('.').append(m_resource);
        }
        if (StringUtils.isNotBlank(m_host)) {
            sb.append('.').append(m_host);
        }
        return sb.toString();
    }

    public int getPriority() {
        return m_priority;
    }

    public void setPriority(int priority) {
        m_priority = priority;
    }

    public int getWeight() {
        return m_weight;
    }

    public void setWeight(int weight) {
        m_weight = weight;
    }

    public String getHost() {
        return m_host;
    }

    public String getDestination() {
        return m_destination;
    }

    public String getProtocol() {
        return m_protocol;
    }

    public String getResource() {
        return m_resource;
    }

    public int getPort() {
        return m_port;
    }

    public boolean isInternal() {
        return m_internal;
    }

    public void setInternal(boolean internal) {
        m_internal = internal;
    }
}
