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

import static org.junit.Assert.assertEquals;

import java.io.InputStream;
import java.io.StringWriter;
import java.util.Arrays;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class PagingConfigurationTest {
    private PagingConfiguration m_pagingConfiguration;
    private List<PagingGroup> m_pagingGroups;

    @Before
    public void setUp() {
        initPagingGroups();
        m_pagingConfiguration = new PagingConfiguration();
        m_pagingConfiguration.setAudioDirectory("media");
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

    @Test
    public void testGenerateConfigProperties() throws Exception {
        StringWriter actual = new StringWriter();
        Location location = new Location();
        location.setAddress("192.168.1.1");
        PagingSettings settings = new PagingSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        m_pagingConfiguration.write(actual, location, m_pagingGroups, settings, "example.org");
        InputStream expected = getClass().getResourceAsStream("expected-sipxpage.properties");
        assertEquals(IOUtils.toString(expected), actual.toString());
    }
}
