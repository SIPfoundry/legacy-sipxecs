/**
 *
 * Copyright (c) 2013 Karel Electronics Corp. All rights reserved.
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
 *
 */

package org.sipfoundry.sipxconfig.systemaudit;

import java.io.Serializable;
import java.util.Date;

public class SystemAuditFilter implements Serializable {

    private Date m_startDate;
    private Date m_endDate;
    private ConfigChangeType m_type;
    private ConfigChangeAction m_action;
    private String m_userName;
    private String m_details;

    public SystemAuditFilter(Date startDate, Date endDate, ConfigChangeType type,
            ConfigChangeAction action, String userName, String details) {
        super();
        this.m_startDate = startDate;
        this.m_endDate = endDate;
        this.m_type = type;
        this.m_action = action;
        this.m_userName = userName;
        this.m_details = details;
    }

    public Date getStartDate() {
        return m_startDate;
    }

    public void setStartDate(Date startDate) {
        this.m_startDate = startDate;
    }

    public Date getEndDate() {
        return m_endDate;
    }

    public void setEndDate(Date endDate) {
        this.m_endDate = endDate;
    }

    public ConfigChangeType getType() {
        return m_type;
    }

    public void setType(ConfigChangeType type) {
        this.m_type = type;
    }

    public ConfigChangeAction getAction() {
        return m_action;
    }

    public void setAction(ConfigChangeAction action) {
        this.m_action = action;
    }

    public String getUserName() {
        return m_userName;
    }

    public void setUserName(String userName) {
        this.m_userName = userName;
    }

    public String getDetails() {
        return m_details;
    }

    public void setDetails(String details) {
        this.m_details = details;
    }
}
