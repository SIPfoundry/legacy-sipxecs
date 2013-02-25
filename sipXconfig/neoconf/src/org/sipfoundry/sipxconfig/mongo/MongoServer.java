/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.mongo;

import java.util.Date;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.bson.types.BSONTimestamp;

import com.mongodb.BasicDBObject;

public class MongoServer {
    public static final String UNCONFIGURED = "UNCONFIGURED";
    public static final String SERVER = "SERVER";
    public static final String ARBITER = "ARBITER";
    public static final String UP = "UP";
    public static final String DOWN = "DOWN";
    private static final String ERR_MSG = "errmsg";
    private static final String HEALTH = "health";
    private static final String OPTIME = "optime";
    private static final String NA = "N/A";
    private int m_id;
    private String m_name;
    private String m_type;
    private String m_state = UNCONFIGURED;
    private String m_health = UNCONFIGURED;
    private String m_optimeDate = NA;
    private String m_lastHeartbeat;
    private String m_pingMs;
    private String m_errMsg = NA;
    private boolean m_configured;
    private boolean m_voting = true;

    public MongoServer(BasicDBObject dbo) {
        m_id = dbo.getInt("_id");
        m_name = dbo.getString("name");
        m_type = SERVER;
        if (StringUtils.contains(m_name, String.valueOf(MongoSettings.ARBITER_PORT))) {
            m_type = ARBITER;
        }
        m_configured = true;
        m_state = dbo.getString("stateStr");
        m_health = UP;
        if (dbo.containsField(HEALTH)) {
            if (dbo.getInt(HEALTH) == 0) {
                m_health = DOWN;
            }
        }
        if (dbo.containsField(ERR_MSG)) {
            m_errMsg = dbo.getString(ERR_MSG);
        }
        try {
            if (dbo.containsField(OPTIME)) {
                BSONTimestamp optime = (BSONTimestamp) dbo.get(OPTIME);
                if (optime.getTime() != 0) {
                    m_optimeDate = new Date((long) optime.getTime() * 1000).toString();
                }
            }
        } catch (NumberFormatException ex) {
            m_optimeDate = NA;
        }
    }

    public MongoServer() {
    }

    public void setName(String name) {
        m_name = name;
    }

    public int getId() {
        return m_id;
    }

    public String getName() {
        return m_name;
    }

    public String getType() {
        return m_type;
    }

    public String getState() {
        return m_state;
    }

    public String getHealth() {
        return m_health;
    }

    public String getOptimeDate() {
        return m_optimeDate;
    }

    public String getLastHeartbeat() {
        return m_lastHeartbeat;
    }

    public String getPingMs() {
        return m_pingMs;
    }

    public String getErrMsg() {
        return m_errMsg;
    }

    public void setVotingMember(boolean voting) {
        m_voting = voting;
    }

    public void setIsServer() {
        m_type = SERVER;
    }

    public void setIsArbiter() {
        m_type = ARBITER;
    }

    public boolean isServer() {
        return m_type.equals(SERVER);
    }

    public boolean isArbiter() {
        return m_type.equals(ARBITER);
    }

    public int getReplicaSetId() {
        return m_id;
    }

    public boolean isConfigured() {
        return m_configured;
    }

    public boolean isVotingMember() {
        return m_voting;
    }

    public int hashCode() {
        return new HashCodeBuilder().append(m_name).toHashCode();
    }

    public boolean equals(Object other) {
        if (!(other instanceof MongoServer)) {
            return false;
        }
        if (this == other) {
            return true;
        }
        MongoServer bean = (MongoServer) other;
        return new EqualsBuilder().append(m_name, bean.getName()).isEquals();
    }
}
