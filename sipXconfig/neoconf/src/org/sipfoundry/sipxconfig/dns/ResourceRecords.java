/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
