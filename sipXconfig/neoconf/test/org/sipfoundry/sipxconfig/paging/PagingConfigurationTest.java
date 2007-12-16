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
import java.util.Map;
import java.util.Set;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.common.User;

public class PagingConfigurationTest extends TestCase {
    private PagingConfiguration m_pagingGroup;
    private List<PagingGroup> m_pagingGroups;

    public void setUp() throws Exception {
        m_pagingGroup = new PagingConfiguration();

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
        Map<String, String> cp = m_pagingGroup.generateConfigProperties(
                m_pagingGroups,  "NOTICE", "media", "example.org");
        // FIXME: expected values should be first
        assertEquals(cp.get("log.level"), "NOTICE");
        assertEquals(cp.get("log.file"),
                "${PAGE_LOG_DIR}/sipxpage.log");
        assertEquals(cp.get("sip.address"), "${PAGE_SERVER_ADDR}");
        assertEquals(cp.get("sip.udpPort"),
                "${PAGE_SERVER_SIP_PORT}");
        assertEquals(cp.get("sip.tcpPort"),
                "${PAGE_SERVER_SIP_PORT}");
        assertEquals(cp.get("sip.tlsPort"),
                "${PAGE_SERVER_SIP_SECURE_PORT}");
        assertEquals(cp.get("rtp.port"), "8500");

        assertEquals(cp.get("page.group.1.description"),
                "All the phones in the east side of the building");
        assertEquals(cp.get("page.group.1.beep"), "file://media/TadaTada.wav");
        assertEquals(cp.get("page.group.1.user"), "42");
        assertEquals(cp.get("page.group.1.urls"), "201@example.org");
    }
}
