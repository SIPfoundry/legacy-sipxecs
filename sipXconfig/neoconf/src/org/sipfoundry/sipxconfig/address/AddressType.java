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
    private String m_id;
    private String m_format;

    public AddressType(String uniqueId) {
        m_id = uniqueId;
    }

    public AddressType(String uniqueId, String format) {
        this(uniqueId);
        m_format = format;
    }

    /**
     * Convenience method to format address as a sip type address
     */
    public static AddressType sip(String uniqueId) {
        return new AddressType(uniqueId, "sip:%s:%d");
    }

    public String getId() {
        return m_id;
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
        for (int i = 0; i < types.length; i++) {
            if (this.equals(types[i])) {
                return true;
            }
        }
        return false;
    }

    public String getFormat() {
        return m_format;
    }
}
