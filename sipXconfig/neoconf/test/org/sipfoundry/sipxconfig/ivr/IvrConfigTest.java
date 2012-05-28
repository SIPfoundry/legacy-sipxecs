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

import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.apache.ApacheManager;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.imbot.ImBot;
import org.sipfoundry.sipxconfig.mwi.Mwi;
import org.sipfoundry.sipxconfig.restserver.RestServer;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class IvrConfigTest {
    private IvrConfig m_config;
    private Location m_location;
    private Domain m_domain;
    private IvrSettings m_settings;
    private Address m_mwiApi;
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
        m_mwiApi = new Address(Mwi.HTTP_API, "mwi.example.org", 100);
        m_restApi = new Address(RestServer.HTTP_API, "rest.example.org", 101);
        m_adminApi = new Address(AdminContext.HTTP_ADDRESS, "admin.example.org", 102);
        m_apacheApi = new Address(ApacheManager.HTTPS_ADDRESS, "admin.example.org");
        m_imApi = new Address(ImManager.XMLRPC_ADDRESS, "im.example.org", 103);
        m_imbotApi = new Address(ImBot.XML_RPC, "imbot.example.org", 104);
        m_fsEvent = new Address(FreeswitchFeature.EVENT_ADDRESS, "fsevent.example.org", 105);
    }

    @Test
    public void testWriteWithOpenfireService() throws Exception {
        StringWriter actual = new StringWriter();
        m_config.write(actual, m_settings, m_domain, m_location, m_mwiApi, m_restApi, m_adminApi, m_apacheApi, m_imApi, m_imbotApi, m_fsEvent);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-sipxivr-with-openfire.properties"));
        assertEquals(expected, actual.toString());
    }

    @Test
    public void testWriteWithoutOpenfireService() throws Exception {
        StringWriter actual = new StringWriter();
        m_config.write(actual, m_settings, m_domain, m_location, m_mwiApi, m_restApi, m_adminApi, m_apacheApi, null, null, m_fsEvent);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-sipxivr-without-openfire.properties"));
        assertEquals(expected, actual.toString());
    }
}
