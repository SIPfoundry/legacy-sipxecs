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
package org.sipfoundry.commons.userdb;

public class UserGroup {
    private String m_sysId;
    private String m_groupName;
    private String m_description;
    private boolean m_isImbotEnabled = false;

    public String getSysId() {
        return m_sysId;
    }

    public void setSysId(String id) {
        m_sysId = id;
    }

    public String getGroupName() {
        return m_groupName;
    }

    public void setGroupName(String groupName) {
        m_groupName = groupName;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public boolean isImbotEnabled() {
        return m_isImbotEnabled;
    }

    public void setImBotEnabled(boolean enabled) {
        m_isImbotEnabled = enabled;
    }

}