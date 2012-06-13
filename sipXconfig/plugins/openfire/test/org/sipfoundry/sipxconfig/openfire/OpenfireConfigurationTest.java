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
package org.sipfoundry.sipxconfig.openfire;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.List;

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
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.test.TestUtil;

import junit.framework.TestCase;

public class OpenfireConfigurationTest extends TestCase {

    private LdapManager m_ldapManager;

    private final LdapSystemSettings m_ldapSystemSettings = new LdapSystemSettings();

    private final LdapConnectionParams m_ldapConnectionParams = new LdapConnectionParams();

    private final AttrMap m_ldapAttrMap = new AttrMap();

    private LocationsManager m_locationsManager;

    private CoreContext m_coreContext;

    @Override
    protected void setUp() throws Exception {
        m_ldapSystemSettings.setEnableOpenfireConfiguration(false);
        m_ldapConnectionParams.setUniqueId(1);
        m_ldapConnectionParams.setHost("localhost");
        // no port specified: defaults to 389
        m_ldapConnectionParams.setPrincipal("cn=Directory Manager");
        m_ldapConnectionParams.setSecret("secret");

        m_ldapAttrMap.setUniqueId(1);
        m_ldapAttrMap.setSearchBase("dc=example,dc=com");
        m_ldapAttrMap.setObjectClass("person");
        m_ldapAttrMap.setAttribute("imId", "uid");

        IMocksControl coreContextControl = EasyMock.createControl();
        m_ldapManager = coreContextControl.createMock(LdapManager.class);
        m_ldapManager.getSystemSettings();
        expectLastCall().andReturn(m_ldapSystemSettings);
        m_ldapManager.getConnectionParams(1);
        expectLastCall().andReturn(m_ldapConnectionParams);
        m_ldapManager.getAllConnectionParams();
        List<LdapConnectionParams> list = new ArrayList<LdapConnectionParams>();
        list.add(m_ldapConnectionParams);
        expectLastCall().andReturn(list);
        m_ldapManager.getAttrMap(1);
        expectLastCall().andReturn(m_ldapAttrMap);

        m_locationsManager = createMock(LocationsManager.class);
        m_locationsManager.getPrimaryLocation();
        Location location = TestUtil.createDefaultLocation();
        expectLastCall().andReturn(location).anyTimes();

        m_coreContext = createMock(CoreContext.class);
        m_coreContext.loadUserByAdmin();
        User user1 = new User();
        user1.setUserName("123");
        User user2 = new User();
        user2.setUserName("130");
        List<User> users = new ArrayList<User>();
        users.add(user1);
        users.add(user2);
        expectLastCall().andReturn(users).anyTimes();

        replay(m_ldapManager, m_locationsManager, m_coreContext);
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

    public void testGenerateLdapAnonymousAccessOpenfireConfiguration() throws Exception {
        m_ldapConnectionParams.setPrincipal("");
        m_ldapSystemSettings.setEnableOpenfireConfiguration(true);
        m_ldapSystemSettings.setConfigured(true);

        OpenfireConfiguration configuration = generate();
        assertCorrectFileGeneration(configuration, "expected-ldap-anonymous-access-openfire-config.test.xml");
    }

    private OpenfireConfiguration generate() {
        OpenfireConfiguration configuration = new OpenfireConfiguration();
        configuration.setVelocityEngine(TestHelper.getVelocityEngine());
        configuration.setTemplate("openfire/openfire.vm");
        configuration.setLdapManager(m_ldapManager);
        configuration.setCoreContext(m_coreContext);

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
