/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.conference;

import java.io.InputStream;
import junit.framework.TestCase;
import org.apache.commons.io.IOUtils;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;

public class ConferenceConfigurationTest extends TestCase {

    private DomainManager m_domainManager;
    private final Location m_location = new Location();
    private final ConferenceConfiguration m_config = new ConferenceConfiguration();

    @Override
    protected void setUp() throws Exception {
        m_location.setFqdn("test.example.com");

        Domain domain = new Domain("example.com");
        IMocksControl control = EasyMock.createControl();
        m_domainManager = control.createMock(DomainManager.class);
        m_domainManager.getDomain();
        EasyMock.expectLastCall().andReturn(domain);
        EasyMock.replay(m_domainManager);

        m_config.setDomainManager(m_domainManager);
        m_config.setVelocityEngine(TestHelper.getVelocityEngine());
        m_config.setMohLocalStreamUrl("local_stream://moh");
        m_config.setPortAudioUrl("portaudio_stream://");
        m_config.setTemplate("sipxconference/conference.conf.xml.vm");
    }

    public void testGenerate() throws Exception {
        Bridge bridge = new Bridge() {
            @Override
            public String getAudioDirectory() {
                return "/audioDirectory";
            }
        };
        bridge.setModelFilesContext(TestHelper.getModelFilesContext());
        bridge.getSettings();
        bridge.setSettingValue(Bridge.CALL_CONTROL_MUTE, "1");
        bridge.setSettingValue(Bridge.CALL_CONTROL_DEAF_MUTE, "2");
        bridge.setSettingValue(Bridge.CALL_CONTROL_ENERGY_UP, "3");
        bridge.setSettingValue(Bridge.CALL_CONTROL_ENERGY_RESET, "4");
        bridge.setSettingValue(Bridge.CALL_CONTROL_ENERGY_DOWN, "5");
        bridge.setSettingValue(Bridge.CALL_CONTROL_VOLUME_UP, "6");
        bridge.setSettingValue(Bridge.CALL_CONTROL_VOLUME_RESET, "7");
        bridge.setSettingValue(Bridge.CALL_CONTROL_VOLUME_DOWN, "8");
        bridge.setSettingValue(Bridge.CALL_CONTROL_TALK_UP, "9");
        bridge.setSettingValue(Bridge.CALL_CONTROL_TALK_RESET, "#");
        bridge.setSettingValue(Bridge.CALL_CONTROL_TALK_DOWN, "*");
        bridge.setSettingValue(Bridge.CALL_CONTROL_HANGUP, "0");

        User owner = new User();

        Conference conf = new Conference();
        conf.setModelFilesContext(TestHelper.getModelFilesContext());
        conf.initialize();
        conf.getSettings();
        conf.setExtension("123");
        conf.setSettingValue(Conference.MAX_LEGS, "0");
        conf.setUniqueId();
        bridge.addConference(conf);

        conf = new Conference();
        conf.setModelFilesContext(TestHelper.getModelFilesContext());
        conf.initialize();
        conf.setOwner(owner);
        conf.setExtension("234");
        conf.setSettingValue(Conference.MAX_LEGS, "4");
        conf.setUniqueId();
        conf.setAutorecorded(true);
        bridge.addConference(conf);

        ConferenceBridgeContext confContext = EasyMock.createMock(ConferenceBridgeContext.class);
        confContext.getBridgeByServer("test.example.com");
        EasyMock.expectLastCall().andReturn(bridge).once();
        EasyMock.replay(confContext);

        m_config.setConferenceBridgeContext(confContext);

        String generatedXml = AbstractConfigurationFile.getFileContent(m_config, m_location);
        InputStream referenceXml = getClass().getResourceAsStream("conference_config.test.xml");
        assertEquals(IOUtils.toString(referenceXml), generatedXml);
    }

    public void testGenerateNullBridge() throws Exception {
        ConferenceBridgeContext confContext = EasyMock.createMock(ConferenceBridgeContext.class);
        confContext.getBridgeByServer("test.example.com");
        EasyMock.expectLastCall().andReturn(null).once();
        EasyMock.replay(confContext);

        m_config.setConferenceBridgeContext(confContext);

        String generatedXml = AbstractConfigurationFile.getFileContent(m_config, m_location);
        InputStream referenceXml = getClass().getResourceAsStream("conference_config_null_bridge.test.xml");
        assertEquals(IOUtils.toString(referenceXml), generatedXml);
    }
}
