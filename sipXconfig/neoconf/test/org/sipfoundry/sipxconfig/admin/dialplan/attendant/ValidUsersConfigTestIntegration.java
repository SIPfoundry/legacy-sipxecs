/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
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
