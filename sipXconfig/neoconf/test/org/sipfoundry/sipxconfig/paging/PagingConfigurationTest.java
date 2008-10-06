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

import java.io.InputStream;
import java.io.StringWriter;
import java.util.Arrays;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Server;
import org.sipfoundry.sipxconfig.common.User;

import static org.easymock.EasyMock.createControl;
import static org.easymock.EasyMock.expect;

public class PagingConfigurationTest extends TestCase {
    private PagingConfiguration m_pagingConfiguration;
    private PagingServer m_pagingServer;
    private List<PagingGroup> m_pagingGroups;

    public void setUp() throws Exception {
        m_pagingServer = new PagingServer();
        m_pagingServer.setLogLevel("NOTICE");
        m_pagingServer.setSipTraceLevel("NONE");

        IMocksControl sipxServerCtrl = createControl();
        Server sipxServer = sipxServerCtrl.createMock(Server.class);
        expect(sipxServer.getPagingLogLevel()).andReturn("NOTICE").once();
        sipxServer.resetSettings();
        sipxServerCtrl.once();
        sipxServerCtrl.replay();
        m_pagingServer.setSipxServer(sipxServer);

        m_pagingConfiguration = new PagingConfiguration();
        m_pagingConfiguration.setVelocityEngine(TestHelper.getVelocityEngine());
        m_pagingConfiguration.setTemplate("commserver/sipxpage.properties.in.vm");

        PagingGroup g1 = new PagingGroup();
        g1.setEnabled(true);
        g1.setDescription("All the phones in the east side of the building");
        g1.setSound("TadaTada.wav");
        g1.setPageGroupNumber(42);
        g1.setTimeout(60);

        Set<User> users = new LinkedHashSet<User>();
        for (int i = 0; i < 3; i++) {
            User u = new User();
            u.setUniqueId();
            u.setUserName(Integer.toString(200 + i));
            users.add(u);
        }
        g1.setUsers(users);

        PagingGroup g2 = new PagingGroup();
        g2.setEnabled(true);
        g2.setSound("Tada.wav");
        g2.setPageGroupNumber(45);
        g2.setTimeout(600);

        Set<User> users2 = new LinkedHashSet<User>();
        for (int i = 0; i < 2; i++) {
            User u = new User();
            u.setUniqueId();
            u.setUserName(Integer.toString(200 + 2 * i));
            users2.add(u);
        }
        g2.setUsers(users2);

        PagingGroup g3 = new PagingGroup();
        g3.setEnabled(false);

        m_pagingGroups = Arrays.asList(g1, g2, g3);
    }

    public void testGenerateConfigProperties() throws Exception {
        m_pagingConfiguration.generate(m_pagingServer, m_pagingGroups, "media", "example.org");

        StringWriter output = new StringWriter();
        m_pagingConfiguration.write(output, null);

        InputStream expectedProfile = getClass().getResourceAsStream("sipxpage.properties.in");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);

        assertEquals(expected, output.toString());
    }
}
