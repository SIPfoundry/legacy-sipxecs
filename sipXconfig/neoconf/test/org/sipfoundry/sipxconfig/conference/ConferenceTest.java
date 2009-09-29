/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import java.util.List;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.acd.BeanWithSettingsTestCase;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.service.LocationSpecificService;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;

public class ConferenceTest extends BeanWithSettingsTestCase {
    private Conference m_conf;
    private Bridge m_bridge;

    protected void setUp() throws Exception {
        super.setUp();

        Location location = new Location();
        SipxFreeswitchService sipxService = new SipxFreeswitchService();
        sipxService.setSettings(TestHelper.loadSettings("freeswitch/freeswitch.xml"));
        LocationSpecificService service = new LocationSpecificService(sipxService);
        service.setLocation(location);

        m_bridge = new Bridge();
        m_bridge.setService(service);
        m_bridge.setModelFilesContext(TestHelper.getModelFilesContext());
        initializeBeanWithSettings(m_bridge);

        m_conf = new Conference();
        initializeBeanWithSettings(m_conf);
        m_conf.getSettings();
    }

    public void testGenerateAccessCodes() {
        m_conf.generateAccessCodes();
        assertTrue(m_conf.getSettingValue(Conference.PARTICIPANT_CODE)
                .toString().length() > 0);
    }

    public void testGetUri() {
        m_bridge.getService().getLocation().setFqdn("bridge1.sipfoundry.org");
        m_bridge.addConference(m_conf);

        m_conf.setName("weekly.marketing");
        assertEquals("sip:weekly.marketing@bridge1.sipfoundry.org:15060", m_conf.getUri());

        m_bridge.getService().getLocation().setFqdn("abc.domain.com");
        assertEquals("sip:weekly.marketing@abc.domain.com:15060", m_conf.getUri());
    }

    public void testGenerateRemoteAdmitSecret() {
        m_bridge.getService().getLocation().setFqdn("bridge1.sipfoundry.org");
        m_bridge.addConference(m_conf);

        assertNull(m_conf.getRemoteAdmitSecret());
        m_conf.generateRemoteAdmitSecret();
        assertTrue(m_conf.getRemoteAdmitSecret().length() > 0);
    }

    public void testGenerateAliases() {
        m_bridge.getService().getLocation().setFqdn("bridge1.sipfoundry.org");
        m_bridge.addConference(m_conf);

        // empty for disabled conference
        m_conf.setName("conf1");
        List aliasMappings = m_conf.generateAliases("sipfoundry.org");

        assertTrue(aliasMappings.isEmpty());

        // 1 alias for conference without extension
        m_conf.setEnabled(true);
        aliasMappings = m_conf.generateAliases("sipfoundry.org");
        assertEquals(1, aliasMappings.size());

        AliasMapping am = (AliasMapping) aliasMappings.get(0);
        assertEquals("conf1@sipfoundry.org", am.getIdentity());

        // 2 aliases for conference with extension
        m_conf.setExtension("1111");
        aliasMappings = m_conf.generateAliases("sipfoundry.org");
        assertEquals(2, aliasMappings.size());

        AliasMapping am0 = (AliasMapping) aliasMappings.get(0);
        assertEquals("1111@sipfoundry.org", am0.getIdentity());
        assertEquals("sip:conf1@sipfoundry.org", am0.getContact());

        AliasMapping am1 = (AliasMapping) aliasMappings.get(1);
        assertEquals(am1, am);
    }
}
