/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.address;

public class AddressType {
    private String m_id;

    public AddressType(String uniqueId) {
        m_id = uniqueId;
    }

    public String getId() {
        return m_id;
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
}
