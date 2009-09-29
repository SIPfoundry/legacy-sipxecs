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
import java.util.List;

import org.apache.commons.lang.ObjectUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class AcdLine extends AcdComponent {
    public static final String BEAN_NAME = "acdLine";
    public static final String OBJECT_CLASS = "acd-line";

    static final String URI = "acd-line/uri";
    static final String LINE_NAME = "acd-line/name";
    static final String EXTENSION = "acd-line/extension";
    static final String ACD_QUEUE = "acd-line/acd-queue";

    private AcdQueue m_queue;

    private AcdServer m_acdServer;

    private String m_extension;

    public AcdLine() {
        super("sipxacd-line.xml", OBJECT_CLASS);
    }

    public AcdServer getAcdServer() {
        return m_acdServer;
    }

    public void setAcdServer(AcdServer acdServer) {
        m_acdServer = acdServer;
    }

    public AcdQueue getAcdQueue() {
        return m_queue;
    }

    public void setAcdQueue(AcdQueue queue) {
        m_queue = queue;
    }

    public String getExtension() {
        return m_extension;
    }

    public void setExtension(String extension) {
        m_extension = extension;
    }

    void setLineName(String lineName) {
        setSettingValue(LINE_NAME, lineName);
    }

    /**
     * Associate this line with a particular queue
     *
     * @param queue to associate or null to remove association
     * @return previously associated queue, null if there was not any
     */
    public AcdQueue associateQueue(AcdQueue queue) {
        if (m_queue == queue) {
            return m_queue;
        }
        AcdQueue oldQueue = m_queue;
        if (m_queue != null) {
            m_queue.removeLine(this);
        }
        m_queue = queue;
        if (m_queue != null) {
            queue.addLine(this);
        }
        return oldQueue;
    }

    @Override
    public String calculateUri() {
        Location location = getAcdServer().getLocation();
        String domainName = location.getFqdn();
        return SipUri.format(getName(), domainName, false);
    }

    public static class AcdLineDefaults {
        private AcdLine m_line;

        AcdLineDefaults(AcdLine line) {
            m_line = line;
        }

        @SettingEntry(path = URI)
        public String getUri() {
            return m_line.calculateUri();
        }

        @SettingEntry(path = LINE_NAME)
        public String getName() {
            String name = (String) ObjectUtils.defaultIfNull(m_line.getDescription(), m_line
                    .getName());
            return name;
        }

        @SettingEntry(path = ACD_QUEUE)
        public String getQueueUri() {
            String queueUri = StringUtils.EMPTY;
            if (m_line.m_queue != null) {
                queueUri = m_line.m_queue.calculateUri();
            }
            return queueUri;
        }

        @SettingEntry(path = EXTENSION)
        public String getExtension() {
            return m_line.getExtension();
        }
    }

    public void appendAliases(List aliases) {
        String extension = getExtension();
        if (StringUtils.isBlank(extension)) {
            return;
        }
        // TODO: remove localhost trick when we have real host information
        String domainName = getCoreContext().getDomainName();
        String identity = AliasMapping.createUri(extension, domainName);

        String server = StringUtils.defaultIfEmpty(m_acdServer.getLocation().getFqdn(), "localhost");
        String contact = SipUri.format(getName(), server, m_acdServer.getSipPort());

        aliases.add(new AliasMapping(identity, contact));
    }

    public Serializable getAcdServerId() {
        return getAcdServer().getId();
    }

    @Override
    public void initialize() {
        addDefaultBeanSettingHandler(new AcdLineDefaults(this));
    }
}
