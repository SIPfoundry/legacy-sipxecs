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

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.address.Address;

public class ResourceRecords {
    private String m_proto;
    private String m_resource;
    private List<DnsRecord> m_records;

    public ResourceRecords(String proto, String resource) {
        m_resource = resource;
        m_proto = proto;
        m_records = new ArrayList<DnsRecord>();
    }

    public List<DnsRecord> getRecords() {
        return m_records;
    }

    public void addRecord(DnsRecord record) {
        m_records.add(record);
    }

    public void addAddresses(Collection<Address> addresses) {
        if (addresses != null) {
            for (Address a : addresses) {
                addAddress(a);
            }
        }
    }

    public void addAddress(Address address) {
        addRecord(new DnsRecord(address.getAddress(), address.getPort()));
    }

    public String getProto() {
        return m_proto;
    }

    public String getResource() {
        return m_resource;
    }
}
