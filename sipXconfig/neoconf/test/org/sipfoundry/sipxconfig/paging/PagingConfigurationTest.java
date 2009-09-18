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

import java.util.Arrays;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.service.SipxPageService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.service.SipxServiceTestBase;
import org.sipfoundry.sipxconfig.setting.Setting;

public class PagingConfigurationTest extends SipxServiceTestBase {
    private PagingConfiguration m_pagingConfiguration;
    private PagingServer m_pagingServer;
    private List<PagingGroup> m_pagingGroups;

    @Override
    public void setUp() throws Exception {
        m_pagingServer = new PagingServer();
        m_pagingServer.setLogLevel("NOTICE");
        m_pagingServer.setSipTraceLevel("NONE");

        m_pagingConfiguration = new PagingConfiguration();
        m_pagingConfiguration.setVelocityEngine(TestHelper.getVelocityEngine());
        m_pagingConfiguration.setTemplate("sipxpage/sipxpage.properties.vm");

        initPagingGroups();

        PagingContext pagingContext= EasyMock.createMock(PagingContext.class);
        pagingContext.getPagingServer();
        EasyMock.expectLastCall().andReturn(m_pagingServer).anyTimes();
        pagingContext.getPagingGroups();
        EasyMock.expectLastCall().andReturn(m_pagingGroups).anyTimes();
        EasyMock.replay(pagingContext);
        m_pagingConfiguration.setPagingContext(pagingContext);

        SipxPageService sipxPageService = new SipxPageService();
        sipxPageService.setModelDir("sipxpage");
        sipxPageService.setModelName("sipxpage.xml");
        initCommonAttributes(sipxPageService);

        Setting pageSettings = sipxPageService.getSettings().getSetting("page-config");
        pageSettings.getSetting("PAGE_SERVER_SIP_PORT").setValue("9898");
        pageSettings.getSetting("PAGE_SERVER_SIP_SECURE_PORT").setValue("9899");
        pageSettings.getSetting("SIP_PAGE_LOG_LEVEL").setValue("CRIT");

        sipxPageService.setAudioDir("media");

        SipxServiceManager sipxServiceManager = EasyMock.createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxPageService.BEAN_ID);
        EasyMock.expectLastCall().andReturn(sipxPageService).anyTimes();
        EasyMock.replay(sipxServiceManager);
        m_pagingConfiguration.setSipxServiceManager(sipxServiceManager);

    }

    private void initPagingGroups() {
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
        assertCorrectFileGeneration(m_pagingConfiguration, "expected-sipxpage.properties");
    }
}
