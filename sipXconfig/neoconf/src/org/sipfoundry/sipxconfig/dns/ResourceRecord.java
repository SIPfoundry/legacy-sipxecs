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
package org.sipfoundry.sipxconfig.dns;

public class ResourceRecord {
    private final String m_address;
    private final int m_port;
    private Integer m_regionId;

    public ResourceRecord(String address, int port) {
        m_address = address;
        m_port = port;
    }

    public ResourceRecord(String address, int port, Integer regionId) {
        m_address = address;
        m_port = port;
        m_regionId = regionId;
    }

    public String getAddress() {
        return m_address;
    }

    public int getPort() {
        return m_port;
    }

    public Integer getRegionId() {
        return m_regionId;
    }

    @Override
    public String toString() {
        return "ResourceRecord [m_address=" + m_address + ", m_port=" + m_port + ", m_regionId=" + m_regionId + "]";
    }
}
