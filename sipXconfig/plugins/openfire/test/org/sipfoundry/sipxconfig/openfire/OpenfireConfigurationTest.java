/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.openfire;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.bulk.ldap.AttrMap;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapConnectionParams;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapSystemSettings;
import org.sipfoundry.sipxconfig.test.TestUtil;

import junit.framework.TestCase;

public class OpenfireConfigurationTest extends TestCase {

    private LdapManager m_ldapManager;

    private final LdapSystemSettings m_ldapSystemSettings = new LdapSystemSettings();

    private final LdapConnectionParams m_ldapConnectionParams = new LdapConnectionParams();

    private final AttrMap m_ldapAttrMap = new AttrMap();

    private LocationsManager m_locationsManager;

    @Override
    protected void setUp() throws Exception {
        m_ldapSystemSettings.setEnableOpenfireConfiguration(false);

        m_ldapConnectionParams.setHost("localhost");
        // no port specified: defaults to 389
        m_ldapConnectionParams.setPrincipal("cn=Directory Manager");
        m_ldapConnectionParams.setSecret("secret");

        m_ldapAttrMap.setSearchBase("dc=example,dc=com");

        IMocksControl coreContextControl = EasyMock.createControl();
        m_ldapManager = coreContextControl.createMock(LdapManager.class);
        m_ldapManager.getSystemSettings();
        expectLastCall().andReturn(m_ldapSystemSettings);
        m_ldapManager.getConnectionParams();
        expectLastCall().andReturn(m_ldapConnectionParams);
        m_ldapManager.getAttrMap();
        expectLastCall().andReturn(m_ldapAttrMap);

        m_locationsManager = createMock(LocationsManager.class);
        m_locationsManager.getPrimaryLocation();
        Location location = TestUtil.createDefaultLocation();
        expectLastCall().andReturn(location).anyTimes();

        replay(m_ldapManager, m_locationsManager);
    }

    public void testGenerateOpenfireConfiguration() throws Exception {
        OpenfireConfiguration configuration = generate();
        assertCorrectFileGeneration(configuration, "expected-openfire-config.test.xml");
    }

    public void testGenerateLdapOpenfireConfiguration() throws Exception {
        m_ldapSystemSettings.setEnableOpenfireConfiguration(true);
        m_ldapSystemSettings.setConfigured(true);

        OpenfireConfiguration configuration = generate();
        assertCorrectFileGeneration(configuration, "expected-ldap-openfire-config.test.xml");
    }

    public void testGenerateLdapTlsOpenfireConfiguration() throws Exception {
        m_ldapSystemSettings.setEnableOpenfireConfiguration(true);
        m_ldapSystemSettings.setConfigured(true);
        m_ldapConnectionParams.setUseTls(true);

        OpenfireConfiguration configuration = generate();
        assertCorrectFileGeneration(configuration, "expected-ldap-tls-openfire-config.test.xml");
    }

    private OpenfireConfiguration generate() {
        OpenfireConfiguration configuration = new OpenfireConfiguration();
        configuration.setVelocityEngine(TestHelper.getVelocityEngine());
        configuration.setTemplate("openfire/openfire.vm");
        configuration.setLdapManager(m_ldapManager);

        return configuration;
    }

    private void assertCorrectFileGeneration(OpenfireConfiguration configuration, String expectedFileName)
            throws Exception {
        configuration.setVelocityEngine(TestHelper.getVelocityEngine());

        StringWriter actualConfigWriter = new StringWriter();
        configuration.write(actualConfigWriter, m_locationsManager.getPrimaryLocation());

        InputStream resourceAsStream = configuration.getClass().getResourceAsStream(expectedFileName);
        assertNotNull(resourceAsStream);

        Reader referenceConfigReader = new InputStreamReader(resourceAsStream);
        String referenceConfig = IOUtils.toString(referenceConfigReader);

        Reader actualConfigReader = new StringReader(actualConfigWriter.toString());
        String actualConfig = IOUtils.toString(actualConfigReader);

        assertEquals(referenceConfig, actualConfig);
    }
}
