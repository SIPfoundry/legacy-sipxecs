/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService.Defaults;
import org.sipfoundry.sipxconfig.setting.type.MultiEnumSetting;

import junit.framework.TestCase;


public class SipxFreeswitchServiceTest extends TestCase {
    private SipxFreeswitchService m_service;

    @Override
    protected void setUp() throws Exception {
        m_service = new SipxFreeswitchService();
        m_service.setModelDir("freeswitch");
        m_service.setModelName("freeswitch.xml");
        m_service.setModelFilesContext(TestHelper.getModelFilesContext());
    }

    public void testSettings() {
        assertEquals(8080, m_service.getXmlRpcPort());
        assertEquals(15060, m_service.getFreeswitchSipPort());

        m_service.setSettingValue(SipxFreeswitchService.FREESWITCH_XMLRPC_PORT, "8081");
        m_service.setSettingValue(SipxFreeswitchService.FREESWITCH_SIP_PORT, "15061");

        assertEquals(8081, m_service.getXmlRpcPort());
        assertEquals(15061, m_service.getFreeswitchSipPort());
    }

    public void testDefaultCodecs() {
        SipxFreeswitchService.setCodecG729(true);
        m_service.initialize();
        String value = m_service.getSettingValue(SipxFreeswitchService.FREESWITCH_CODECS);
        assertEquals("G722|PCMU@20i|PCMA@20i|speex|L16|G729", value);

        SipxFreeswitchService.setCodecG729(false);
        m_service.initialize();
        value = m_service.getSettingValue(SipxFreeswitchService.FREESWITCH_CODECS);
        assertEquals("G722|PCMU@20i|PCMA@20i|speex|L16", value);
    }

    public void testGetServiceUri() {
        Location location = new Location();
        location.setFqdn("sipx.example.org");

        assertEquals("http://sipx.example.org:8080/RPC2", m_service.getServiceUri(location));
    }
}
