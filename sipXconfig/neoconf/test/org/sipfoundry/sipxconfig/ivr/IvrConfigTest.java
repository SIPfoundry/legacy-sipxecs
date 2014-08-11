/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.ivr;

import static org.junit.Assert.assertEquals;

import java.io.StringWriter;
import java.util.LinkedList;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.apache.ApacheManager;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.attendant.AutoAttendantSettings;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.restserver.RestServer;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class IvrConfigTest {
    private IvrConfig m_config;
    private Location m_location;
    private Domain m_domain;
    private IvrSettings m_settings;
    private AutoAttendantSettings m_aaSettings;
    private Address m_restApi;
    private Address m_adminApi;
    private Address m_apacheApi;
    private Address m_imApi;
    private Address m_imbotApi;
    private Address m_fsEvent;

    @Before
    public void setUp() {
        m_config = new IvrConfig();
        m_location = TestHelper.createDefaultLocation();
        m_domain = new Domain("example.org");
        m_domain.setSipRealm("grapefruit");
        m_settings = new IvrSettings();
        m_settings.setModelFilesContext(TestHelper.getModelFilesContext());
        m_restApi = new Address(RestServer.HTTP_API, "rest.example.org", 101);
        m_adminApi = new Address(AdminContext.HTTP_ADDRESS, "admin.example.org", 102);
        m_apacheApi = new Address(ApacheManager.HTTPS_ADDRESS, "admin.example.org");
        m_imApi = new Address(ImManager.XMLRPC_ADDRESS, "im.example.org", 103);
        m_fsEvent = new Address(FreeswitchFeature.EVENT_ADDRESS, "fsevent.example.org", 105);
        m_aaSettings = new AutoAttendantSettings();
        m_aaSettings.setModelFilesContext(TestHelper.getModelFilesContext());
        m_aaSettings.setSettingTypedValue("liveAttendant/did", "1234567");
    }

    @Test
    public void testWriteWithOpenfireService() throws Exception {
        StringWriter actual = new StringWriter();
        m_config.write(actual, m_settings, m_domain, m_location, "192.168.0.1,192.168.0.2", 8100, m_restApi,
                m_adminApi, m_apacheApi, m_imApi, m_fsEvent, m_aaSettings);
        String expected = IOUtils.toString(getClass().getResourceAsStream(
                "expected-sipxivr-with-openfire.properties"));
        assertEquals(expected, actual.toString());
    }

    @Test
    public void testWriteWithoutOpenfireService() throws Exception {
        StringWriter actual = new StringWriter();
        m_config.write(actual, m_settings, m_domain, m_location, "192.168.0.1,192.168.0.2", 8100, m_restApi,
                m_adminApi, m_apacheApi, null, m_fsEvent, m_aaSettings);
        String expected = IOUtils.toString(getClass().getResourceAsStream(
                "expected-sipxivr-without-openfire.properties"));
        assertEquals(expected, actual.toString());
    }

    @Test
    public void testGetMwiLocation() throws Exception {
        Location location1 = new Location();
        location1.setAddress("192.168.0.1");
        location1.setUniqueId(1);

        Location location2 = new Location();
        location2.setAddress("192.168.0.2");
        location2.setUniqueId(2);

        Location location3 = new Location();
        location3.setAddress("192.168.0.3");
        location3.setRegionId(1);
        location3.setUniqueId(3);

        List<Location> locations = new LinkedList<Location>();
        locations.add(location1);
        locations.add(location2);
        locations.add(location3);

        assertEquals("192.168.0.1,192.168.0.2,192.168.0.3@1", m_config.getMwiLocations(locations, location1));

        assertEquals("192.168.0.2,192.168.0.1,192.168.0.3@1", m_config.getMwiLocations(locations, location2));

        assertEquals("192.168.0.3@1,192.168.0.1,192.168.0.2", m_config.getMwiLocations(locations, location3));
    }
}
