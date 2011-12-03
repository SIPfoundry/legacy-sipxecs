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
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.BeanWithId;

public abstract class OpenAcdAgentWithSkills extends BeanWithId {
    private static final String DELIM = ", ";

    private Set<OpenAcdSkill> m_skills = new LinkedHashSet<OpenAcdSkill>();
    private Set<OpenAcdQueue> m_queues = new LinkedHashSet<OpenAcdQueue>();
    private Set<OpenAcdClient> m_clients = new LinkedHashSet<OpenAcdClient>();

    public Set<OpenAcdSkill> getSkills() {
        return m_skills;
    }

    public void setSkills(Set<OpenAcdSkill> skills) {
        m_skills = skills;
    }

    public void addSkill(OpenAcdSkill skill) {
        m_skills.add(skill);
    }

    public void removeSkill(OpenAcdSkill skill) {
        m_skills.remove(skill);
    }

    public Set<OpenAcdQueue> getQueues() {
        return m_queues;
    }

    public void setQueues(Set<OpenAcdQueue> queues) {
        m_queues = queues;
    }

    public void addQueue(OpenAcdQueue queue) {
        m_queues.add(queue);
    }

    public void removeQueue(OpenAcdQueue queue) {
        m_queues.remove(queue);
    }

    public Set<OpenAcdClient> getClients() {
        return m_clients;
    }

    public void setClients(Set<OpenAcdClient> clients) {
        m_clients = clients;
    }

    public void addClient(OpenAcdClient client) {
        m_clients.add(client);
    }

    public void removeClient(OpenAcdClient client) {
        m_clients.remove(client);
    }

    public String getSkillsAtoms() {
        List<String> atoms = new ArrayList<String>();
        for (OpenAcdSkill skill : m_skills) {
            atoms.add(skill.getAtom());
        }
        return StringUtils.join(atoms, DELIM);
    }

    public String getQueuesName() {
        List<String> queues = new ArrayList<String>();
        for (OpenAcdQueue queue : m_queues) {
            queues.add(queue.getName());
        }
        return StringUtils.join(queues, DELIM);
    }

    public String getClientsName() {
        List<String> clients = new ArrayList<String>();
        for (OpenAcdClient client : m_clients) {
            clients.add(client.getName());
        }
        return StringUtils.join(clients, DELIM);
    }

    public String getAllSkillsAsString() {
        return StringUtils.join(getAllSkillNames(), DELIM);
    }

    public List<String> getAllSkillNames() {
        List<String> skills = new ArrayList<String>();
        for (OpenAcdSkill skill : getSkills()) {
            skills.add(skill.getName());
        }
        for (OpenAcdQueue queue : getQueues()) {
            skills.add(queue.getName());
        }
        for (OpenAcdClient client : getClients()) {
            skills.add(client.getName());
        }
        return skills;
    }
}
