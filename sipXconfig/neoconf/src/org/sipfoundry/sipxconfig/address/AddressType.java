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
}
