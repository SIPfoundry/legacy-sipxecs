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
