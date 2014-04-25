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
package org.sipfoundry.sipxconfig.rest;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.classextension.EasyMock.createMock;

import java.io.StringWriter;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.restlet.data.ChallengeResponse;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.resource.Representation;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapSystemSettings;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.security.TestAuthenticationToken;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.context.SecurityContextHolder;

public class LoginDetailsResourceTest extends TestCase {

    private User m_user;
    private CoreContext m_coreContext;
    private LdapManager m_ldapManager;
    private ConfigManager m_configManager;
    private LocationsManager m_locationsManager;
    private FeatureManager m_featureManager;

    @Override
    protected void setUp() throws Exception {
        m_user = new User();
        m_user.setUniqueId();
        m_user.setUserName("200");
        m_user.setFirstName("John");
        m_user.setLastName("Doe");
        m_user.setImId("JohnIM");
        m_user.setSipPassword("12345678");
        m_user.setPin("userpin");
        PermissionManager pManager = createMock(PermissionManager.class);
        pManager.getPermissionModel();
        expectLastCall().andReturn(TestHelper.loadSettings("commserver/user-settings.xml")).anyTimes();
        replay(pManager);
        m_user.setPermissionManager(pManager);

        Authentication token = new TestAuthenticationToken(m_user, false, false).authenticateToken();
        SecurityContextHolder.getContext().setAuthentication(token);

        m_coreContext = createMock(CoreContext.class);
        m_coreContext.loadUser(m_user.getId());
        expectLastCall().andReturn(m_user).times(2);
        m_ldapManager = createMock(LdapManager.class);
        LdapSystemSettings settings = new LdapSystemSettings();
        settings.setEnableOpenfireConfiguration(false);
        m_ldapManager.getSystemSettings();
        expectLastCall().andReturn(settings).times(2);
        m_configManager = createMock(ConfigManager.class);
        m_locationsManager = createMock(LocationsManager.class);
        m_featureManager = createMock(FeatureManager.class);
        m_configManager.getLocationManager();
        expectLastCall().andReturn(m_locationsManager).times(1);
        Location location1 = new Location();
        location1.setFqdn("location1.example.com");
        Location location2 = new Location();
        location2.setFqdn("location2.example.com");
        Location location3 = new Location();
        location3.setFqdn("location3.example.com");
        Location[] locations = new Location[] {location1, location2, location3};
        m_locationsManager.getLocations();
        expectLastCall().andReturn(locations).times(1);
        m_configManager.getFeatureManager();
        expectLastCall().andReturn(m_featureManager).times(1);
        m_featureManager.isFeatureEnabled(ImManager.FEATURE, location1);
        expectLastCall().andReturn(false).times(1);
        m_featureManager.isFeatureEnabled(ImManager.FEATURE, location2);
        expectLastCall().andReturn(true).times(1);
        m_featureManager.isFeatureEnabled(ImManager.FEATURE, location3);
        expectLastCall().andReturn(false).times(1);

        replay(m_coreContext, m_ldapManager, m_locationsManager, m_configManager, m_featureManager);
    }

    @Override
    protected void tearDown() throws Exception {
        SecurityContextHolder.getContext().setAuthentication(null);
    }

    public void testRepresentXml() throws Exception {
        LoginDetailsResource resource = new LoginDetailsResource();
        setContexts(resource);
        assertEqualsXML(resource, "logindetails.rest.test.xml");

        LoginDetailsResourceWithPin resourceWithPin = new LoginDetailsResourceWithPin();
        resourceWithPin.setConfigManager(m_configManager);
        setContexts(resourceWithPin);
        assertEqualsXML(resourceWithPin, "logindetailswithpin.rest.test.xml");
    }

    public void testRepresentJson() throws Exception {
        LoginDetailsResource resource = new LoginDetailsResource();
        setContexts(resource);
        assertEqualsJSON(resource, "logindetails.rest.test.json");

        LoginDetailsResourceWithPin resourceWithPin = new LoginDetailsResourceWithPin();
        resourceWithPin.setConfigManager(m_configManager);
        setContexts(resourceWithPin);
        assertEqualsJSON(resourceWithPin, "logindetailswithpin.rest.test.json");
    }

    protected void assertEqualsXML(LoginDetailsResource resource, String fileName) throws Exception {
        ChallengeResponse challengeResponse = new ChallengeResponse(null, "200", new char[0]);
        Request request = new Request();
        request.setChallengeResponse(challengeResponse);
        resource.init(null, request, null);

        Representation representation = resource.represent(new Variant(MediaType.TEXT_XML));
        StringWriter writer = new StringWriter();
        representation.write(writer);
        String generated = writer.toString();
        String expected = IOUtils.toString(getClass().getResourceAsStream(fileName));
        assertEquals(expected, generated);
    }

    protected void assertEqualsJSON(LoginDetailsResource resource, String fileName) throws Exception {
        ChallengeResponse challengeResponse = new ChallengeResponse(null, "200", new char[0]);
        Request request = new Request();
        request.setChallengeResponse(challengeResponse);
        resource.init(null, request, null);

        Representation representation = resource.represent(new Variant(MediaType.APPLICATION_JSON));
        StringWriter writer = new StringWriter();
        representation.write(writer);
        String generated = writer.toString();
        String expected = IOUtils.toString(getClass().getResourceAsStream(fileName));
        assertEquals(expected, generated);
    }

    protected void setContexts(LoginDetailsResource resource) {
        resource.setCoreContext(m_coreContext);
        resource.setLdapManager(m_ldapManager);
    }

}
