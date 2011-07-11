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
package org.sipfoundry.sipxconfig.admin.dialplan.attendant;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile.getFileContent;

import java.io.InputStream;
import java.util.Arrays;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.commserver.AliasProvider;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;

public class ValidUsersConfigTestIntegration extends IntegrationTestCase {
    private ValidUsersConfig m_validUsers;

    public void testGenerate() throws Exception {
        loadDataSet("admin/dialplan/attendant/ValidUsersSeed.xml");
        Domain domain = new Domain();
        domain.setName("example.com");

        DomainManager domainManager = createMock(DomainManager.class);
        expect(domainManager.getDomain()).andReturn(domain).anyTimes();
        expect(domainManager.getAuthorizationRealm()).andReturn("example").anyTimes();

        AliasMapping am1 = new AliasMapping("500@example.com", "sip:500@example.com", "test");
        AliasMapping am2 = new AliasMapping("501@example.com", "sip:501@example.com", "test");

        AliasProvider aliasProvider = createMock(AliasProvider.class);
        expect(aliasProvider.getAliasMappings()).andReturn(Arrays.asList(am1, am2)).anyTimes();

        replay(domainManager, aliasProvider);

        m_validUsers.setDomainManager(domainManager);
        m_validUsers.setAliasProvider(aliasProvider);

        String generatedXml = getFileContent(m_validUsers, null);
        InputStream referenceXml = getClass().getResourceAsStream("validusers.test.xml");
        assertEquals(IOUtils.toString(referenceXml), generatedXml);

        verify(domainManager, aliasProvider);
    }

    public void setValidUsersConfig(ValidUsersConfig config) {
        m_validUsers = config;
    }
}
