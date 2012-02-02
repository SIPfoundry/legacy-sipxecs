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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.junit.Assert.assertEquals;

import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class OpenAcdConfigurationTest {
    private OpenAcdConfiguration m_config;
    private OpenAcdSettings m_settings;
    private Domain m_domain;
    private DomainManager m_domainManager;
    private Location m_location;
    
    @Before
    public void setUp() {
        m_config = new OpenAcdConfiguration();
        m_settings = new OpenAcdSettings();
        m_domain = new Domain("example.org");
        m_domainManager = createMock(DomainManager.class);
        m_domainManager.getDomain();
        expectLastCall().andReturn(m_domain).anyTimes();
        replay(m_domainManager);
        m_settings.setDomainManager(m_domainManager);
        m_settings.setModelFilesContext(TestHelper.getModelFilesContext());
        m_config.setVelocityEngine(TestHelper.getVelocityEngine());
        m_location = TestHelper.createDefaultLocation();
    }

    @Test
    public void testAppConfig() throws Exception {
        StringWriter actual = new StringWriter();
        m_config.writeAppConfig(actual, m_settings);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-app-config"));
        assertEquals(expected, actual.toString());
    }
}
