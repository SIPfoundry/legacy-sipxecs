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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

import java.util.List;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsTestCase;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class ConferenceTest extends BeanWithSettingsTestCase {
    private Conference m_conf;
    private Bridge m_bridge;
    private Bridge m_bridge2;
    private AddressManager m_addressManager;

    protected void setUp() throws Exception {
        super.setUp();

        m_addressManager = createMock(AddressManager.class);

        m_bridge = new Bridge();
        m_bridge.setLocation(new Location("bridge1.example.org"));
        m_bridge.setModelFilesContext(TestHelper.getModelFilesContext());
        initializeBeanWithSettings(m_bridge);

        m_bridge2 = new Bridge();
        m_bridge2.setLocation(new Location("abc.example.com"));
        m_bridge2.setModelFilesContext(TestHelper.getModelFilesContext());
        initializeBeanWithSettings(m_bridge2);

        m_conf = new Conference();
        initializeBeanWithSettings(m_conf);
        m_conf.getSettings();
        m_conf.setAddressManager(m_addressManager);
    }

    public void testGenerateAccessCodes() {
        m_conf.generateAccessCodes();
        assertNotNull(m_conf.getSettingValue(Conference.PARTICIPANT_CODE));
        assertNull(m_conf.getSettingValue(Conference.MODERATOR_CODE));        
    }

    public void testGetUri() {
        m_addressManager.getSingleAddress(FreeswitchFeature.SIP_ADDRESS);
        expectLastCall().andReturn(new Address(FreeswitchFeature.SIP_ADDRESS, "bridge1.example.org", 1111)).once();
        expectLastCall().andReturn(new Address(FreeswitchFeature.SIP_ADDRESS, "abc.example.com", 2222)).once();
        replay(m_addressManager);
        
        m_bridge.addConference(m_conf);

        m_conf.setName("weekly.marketing");
        assertEquals("sip:weekly.marketing@bridge1.example.org:1111", m_conf.getUri());

        m_conf.setBridge(m_bridge2);
        assertEquals("sip:weekly.marketing@abc.example.com:2222", m_conf.getUri());
    }

    public void testGenerateRemoteAdmitSecret() {
        m_addressManager.getSingleAddress(FreeswitchFeature.SIP_ADDRESS);
        expectLastCall().andReturn(new Address(FreeswitchFeature.SIP_ADDRESS, "bridge1.example.org", 1111)).once();
        replay(m_addressManager);
        
        m_bridge.addConference(m_conf);

        assertNull(m_conf.getRemoteAdmitSecret());
        m_conf.generateRemoteAdmitSecret();
        assertTrue(m_conf.getRemoteAdmitSecret().length() > 0);
    }

    public void testGenerateAliases() {
        m_addressManager.getSingleAddress(FreeswitchFeature.SIP_ADDRESS);
        expectLastCall().andReturn(new Address(FreeswitchFeature.SIP_ADDRESS, "bridge1.sipfoundry.org", 2222)).anyTimes();
        replay(m_addressManager);

        m_bridge.addConference(m_conf);
        // empty for disabled conference
        m_conf.setName("conf1");
        List<AliasMapping> aliasMappings = (List<AliasMapping>) m_conf.getAliasMappings("sipfoundry.org");

        assertTrue(aliasMappings.isEmpty());
        
        // 1 alias for conference without extension
        m_conf.setEnabled(true);
        aliasMappings = (List<AliasMapping>) m_conf.getAliasMappings("bridge1.sipfoundry.org");
        assertEquals(1, aliasMappings.size());

        AliasMapping am = (AliasMapping) aliasMappings.get(0);
        assertEquals("conf1", am.getIdentity());

        // 2 aliases for conference with extension
        m_conf.setExtension("1111");
        aliasMappings = (List<AliasMapping>) m_conf.getAliasMappings("sipfoundry.org");
        assertEquals(2, aliasMappings.size());

        AliasMapping am0 = (AliasMapping) aliasMappings.get(0);
        assertEquals("1111", am0.getIdentity());
        assertEquals("sip:conf1@sipfoundry.org", am0.getContact());

        AliasMapping am1 = (AliasMapping) aliasMappings.get(1);
        assertEquals(am1, am);

        // 1 alias for conference with same name as extension
        m_conf.setName("1111");
        m_conf.setExtension("1111");
        aliasMappings = (List<AliasMapping>) m_conf.getAliasMappings("sipfoundry.org");
        assertEquals(1, aliasMappings.size());
    }

    public void testGetAorRecord() {
        Conference conf1 = new Conference();
        initializeBeanWithSettings(conf1);
        conf1.getSettings();
        conf1.setAddressManager(m_addressManager);
        m_addressManager.getSingleAddress(FreeswitchFeature.SIP_ADDRESS);
        expectLastCall().andReturn(new Address(FreeswitchFeature.SIP_ADDRESS, "bridge1.sipfoundry.org", 2222)).anyTimes();
        replay(m_addressManager);
        Bridge bridge1 = new Bridge();
        bridge1.setLocation(new Location("bridge1.sipfoundry.org"));
        bridge1.setModelFilesContext(TestHelper.getModelFilesContext());
        initializeBeanWithSettings(bridge1);
        bridge1.setLocation(new Location("bridge1.sipfoundry.org"));
        conf1.setBridge(bridge1);
        // empty for disabled conference
        conf1.setName("conf1");
        assertEquals("sip:conf1@bridge1.sipfoundry.org:2222", conf1.getAorRecord());
    }
}
