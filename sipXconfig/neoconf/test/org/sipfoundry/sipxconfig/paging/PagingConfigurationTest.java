/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.paging;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Properties;
import java.util.Set;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.domain.DomainManagerImpl;

import junit.framework.TestCase;

public class PagingConfigurationTest extends TestCase {
    private DomainManager m_domainManager;
    private PagingConfiguration m_pagingGroup;
    private List<PagingGroup> m_pagingGroups;

    public void setUp() throws Exception {
        m_pagingGroup = new PagingConfiguration();
        m_pagingGroup.setEtcDirectory(TestHelper.getTestDirectory());
        m_pagingGroup.setAudioDirectory(TestHelper.getTestDirectory());

        m_domainManager = new DomainManagerImplTest();
        m_pagingGroup.setDomainManager(m_domainManager);

        PagingGroup group = new PagingGroup();
        group.setEnabled(true);
        group.setDescription("All the phones in the east side of the building");
        group.setSound("TadaTada.wav");
        group.setPageGroupNumber(new Long(42));

        Set<User> users = new HashSet<User>();
        User user1 = new User();
        user1.setUserName("201");
        users.add(user1);
        group.setUsers(users);

        m_pagingGroups = new ArrayList<PagingGroup>();
        m_pagingGroups.add(group);
    }

    public void testGenerateConfigProperties() throws Exception {
        Properties expectedConfigProperties = m_pagingGroup.generateConfigProperties(
                m_pagingGroups, "NOTICE");
        assertEquals(expectedConfigProperties.getProperty("log.level"), "NOTICE");
        assertEquals(expectedConfigProperties.getProperty("log.file"),
                "${PAGE_LOG_DIR}/sipxpage.log");
        assertEquals(expectedConfigProperties.getProperty("sip.address"), "${PAGE_SERVER_ADDR}");
        assertEquals(expectedConfigProperties.getProperty("sip.udpPort"),
                "${PAGE_SERVER_SIP_PORT}");
        assertEquals(expectedConfigProperties.getProperty("sip.tcpPort"),
                "${PAGE_SERVER_SIP_PORT}");
        assertEquals(expectedConfigProperties.getProperty("sip.tlsPort"),
                "${PAGE_SERVER_SIP_SECURE_PORT}");
        assertEquals(expectedConfigProperties.getProperty("rtp.port"), "8500");

        assertEquals(expectedConfigProperties.getProperty("page.group.1.description"),
                "All the phones in the east side of the building");
        assertEquals(expectedConfigProperties.getProperty("page.group.1.beep"), "file://"
                + TestHelper.getTestDirectory() + "/TadaTada.wav");
        assertEquals(expectedConfigProperties.getProperty("page.group.1.user"), "42");
        assertEquals(expectedConfigProperties.getProperty("page.group.1.urls"), "201@pingtel.com");
    }

    private class DomainManagerImplTest extends DomainManagerImpl {
        public Domain getDomain() {
            Domain domain = new Domain();
            domain.setName("pingtel.com");
            return domain;
        }
    }
}
