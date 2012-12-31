/**
 *
 *
 * Copyright (c) 2010 / 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.bulk.ldap;
/**
 * This bean maps a registration in setting_value table and it is used on LDAP imports to
 * overwrite PIN or not when users get updated during imports.
 */
public class OverwritePinBean {
    private int m_id;
    private boolean m_value;

    public OverwritePinBean(int id, boolean value) {
        m_id = id;
        m_value = value;
    }

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }

    public boolean isValue() {
        return m_value;
    }

    public void setValue(boolean value) {
        m_value = value;
    }
}
