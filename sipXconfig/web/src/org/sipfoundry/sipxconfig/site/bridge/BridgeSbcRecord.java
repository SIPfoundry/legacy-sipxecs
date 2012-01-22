/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.bridge;

import java.io.Serializable;

public class BridgeSbcRecord implements Serializable {

    private static final long serialVersionUID = 1L;

    private Integer m_id;
    private String m_name;
    private String m_description;
    private String m_address;

    public BridgeSbcRecord(Integer id, String name, String description, String address) {
        m_id = id;
        m_name = name;
        m_description = description;
        m_address = address;
    }

    public Integer getId() {
        return m_id;
    }

    public String getDescription() {
        return m_description;
    }

    public String getAddress() {
        return m_address;
    }

    public String getName() {
        return m_name;
    }
}
