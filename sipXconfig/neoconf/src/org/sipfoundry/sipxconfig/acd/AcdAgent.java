/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.lang.ObjectUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class AcdAgent extends AcdComponent {
    public static final String BEAN_NAME = "acdAgent";
    public static final String OBJECT_CLASS = "acd-agent";

    static final String URI = "acd-agent/uri";
    static final String NAME = "acd-agent/name";
    static final String QUEUE_LIST = "acd-agent/acd-queue-list";

    private User m_user;

    private AcdServer m_acdServer;

    private List m_queues = new ArrayList();

    public AcdAgent() {
        super("sipxacd-agent.xml", OBJECT_CLASS);
    }

    @Override
    public void initialize() {
        addDefaultBeanSettingHandler(new AcdAgentDefaults(this));
    }

    public User getUser() {
        return m_user;
    }

    public void setUser(User user) {
        m_user = user;
    }

    @Override
    public String getName() {
        return m_user.getUserName();
    }

    @Override
    public void setName(String name_) {
        throw new UnsupportedOperationException("Cannot change name for AcdAgent");
    }

    public static class AcdAgentDefaults {
        private final AcdAgent m_agent;
        AcdAgentDefaults(AcdAgent agent) {
            m_agent = agent;
        }

        @SettingEntry(path = URI)
        public String getUri() {
            return m_agent.calculateUri();
        }

        @SettingEntry(path = NAME)
        public String getName() {
            User user = m_agent.getUser();
            String acdName = (String) ObjectUtils.defaultIfNull(user.getDisplayName(), user
                    .getUserName());
            return acdName;
        }

        @SettingEntry(path = QUEUE_LIST)
        public String getQueueList() {
            String list = StringUtils.join(m_agent.getQueueUris(), ',');
            return list;
        }
    }

    private String[] getQueueUris() {
        List queues = getQueues();
        String[] queueUris = new String[queues.size()];
        for (int i = 0; i < queueUris.length; i++) {
            AcdQueue queue = (AcdQueue) queues.get(i);
            queueUris[i] = queue.calculateUri();
        }
        return queueUris;
    }

    public List getQueues() {
        return m_queues;
    }

    public void setQueues(List queues) {
        m_queues = queues;
    }

    public void moveQueues(Collection ids, int step) {
        DataCollectionUtil.moveByPrimaryKey(getQueues(), ids.toArray(), step);
    }

    public void addToQueue(AcdQueue queue) {
        List queues = getQueues();
        queues.remove(queue);
        queues.add(queue);
    }

    public void removeFromQueue(AcdQueue queue) {
        List queues = getQueues();
        if (queues.remove(queue) && queues.isEmpty()) {
            if (m_acdServer != null) {
                m_acdServer.removeAgent(this);
            }
        }
    }

    public void removeFromAllQueues() {
        // make a copy of the queues and clear the original list of queues
        List queues = new ArrayList(getQueues());
        m_queues.clear();
        for (Iterator j = queues.iterator(); j.hasNext();) {
            AcdQueue queue = (AcdQueue) j.next();
            queue.removeAgent(this);
        }
    }

    public void setAcdServer(AcdServer server) {
        m_acdServer = server;
    }

    public AcdServer getAcdServer() {
        return m_acdServer;
    }

    @Override
    public Serializable getAcdServerId() {
        return getAcdServer().getId();
    }
}
