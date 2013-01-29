/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.openacd;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;

public class OpenAcdAgent extends OpenAcdAgentWithSkills implements Replicable {
    public static final String EMPTY_STRING = "";

    public static enum Security {
        AGENT {
            public String toString() {
                return "AGENT";
            }
        },
        SUPERVISOR {
            public String toString() {
                return "SUPERVISOR";
            }
        },
        ADMIN {
            public String toString() {
                return "ADMIN";
            }
        }
    }

    private OpenAcdAgentGroup m_group;
    private User m_user;
    private String m_security = Security.AGENT.toString(); // default 'AGENT'
    private String m_oldName;

    public OpenAcdAgentGroup getGroup() {
        return m_group;
    }

    public void setGroup(OpenAcdAgentGroup group) {
        m_group = group;
    }

    public User getUser() {
        return m_user;
    }

    public void setUser(User user) {
        m_user = user;
    }

    public String getSecurity() {
        return m_security;
    }

    public void setSecurity(String security) {
        m_security = security;
    }

    public String getName() {
        return m_user.getUserName();
    }

    public void setName(String name) {
        m_user.setName(name);
    }

    public String getFirstName() {
        return StringUtils.defaultString(m_user.getFirstName(), StringUtils.EMPTY);
    }

    public String getLastName() {
        return StringUtils.defaultString(m_user.getLastName(), StringUtils.EMPTY);
    }

    public String getAgentGroup() {
        if (m_group != null) {
            return m_group.getName();
        }
        return null;
    }

    public String getOldName() {
        return m_oldName;
    }

    public void setOldName(String oldName) {
        m_oldName = oldName;
    }

    @Override
    public Map<String, Object> getMongoProperties(String domain) {
        Map<String, Object> props = new HashMap<String, Object>();
        List<String> skills = new ArrayList<String>();
        for (OpenAcdSkill skill : getSkills()) {
            skills.add(skill.getAtom());
        }
        props.put(OpenAcdContext.SKILLS, skills);
        List<String> queues = new ArrayList<String>();
        for (OpenAcdQueue queue : getQueues()) {
            queues.add(queue.getName());
        }
        props.put(OpenAcdContext.QUEUES, queues);
        List<String> clients = new ArrayList<String>();
        for (OpenAcdClient client : getClients()) {
            clients.add(client.getName());
        }
        props.put(OpenAcdContext.CLIENTS, clients);
        props.put(OpenAcdContext.FIRST_NAME, getFirstName());
        props.put(OpenAcdContext.LAST_NAME, getLastName());
        props.put(OpenAcdContext.OLD_NAME, getOldName());
        props.put(OpenAcdContext.SECURITY, getSecurity());
        props.put(OpenAcdContext.AGENT_GROUP, getAgentGroup());
        props.put(MongoConstants.CONTACT, getUser().getIdentity(domain));
        props.put(MongoConstants.PINTOKEN, getUser().getPintoken());

        return props;
    }

    public int hashCode() {
        return new HashCodeBuilder().append(m_group).append(m_user).toHashCode();
    }

    public boolean equals(Object other) {
        if (!(other instanceof OpenAcdAgent)) {
            return false;
        }
        if (this == other) {
            return true;
        }
        OpenAcdAgent bean = (OpenAcdAgent) other;
        return new EqualsBuilder().append(m_group, bean.getGroup()).append(m_user, bean.getUser()).isEquals();
    }

    @Override
    public Set<DataSet> getDataSets() {
        return Collections.singleton(DataSet.OPENACD);
    }

    @Override
    public String getIdentity(String domainName) {
        return null;
    }

    @Override
    public Collection<AliasMapping> getAliasMappings(String domainName) {
        return null;
    }

    @Override
    public boolean isValidUser() {
        return false;
    }

    @Override
    public String getEntityName() {
        return getClass().getSimpleName();
    }
}
