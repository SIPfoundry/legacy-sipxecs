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
import java.io.StringWriter;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class ConferenceConfigurationTest extends TestCase {

    private DomainManager m_domainManager;
    private final Location m_location = new Location();
    private final Domain m_domain = new Domain("example.com");
    private final ConferenceConfiguration m_config = new ConferenceConfiguration();

    @Override
    protected void setUp() throws Exception {
        m_location.setFqdn("test.example.com");
        m_config.setDomainManager(m_domainManager);
        m_config.setVelocityEngine(TestHelper.getVelocityEngine());
        m_config.setMohLocalStreamUrl("local_stream://moh");
        m_config.setPortAudioUrl("portaudio_stream://");
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

        testConference(bridge, "conference_config.test.xml");
    }

    public void testGenerateDtmf() throws Exception {
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
        bridge.setSettingValue(Bridge.CALL_CONTROL_ENERGY_RESET, "");
        bridge.setSettingValue(Bridge.CALL_CONTROL_ENERGY_DOWN, "5");
        bridge.setSettingValue(Bridge.CALL_CONTROL_VOLUME_UP, "");
        bridge.setSettingValue(Bridge.CALL_CONTROL_VOLUME_RESET, "7");
        bridge.setSettingValue(Bridge.CALL_CONTROL_VOLUME_DOWN, "");
        bridge.setSettingValue(Bridge.CALL_CONTROL_TALK_UP, "9");
        bridge.setSettingValue(Bridge.CALL_CONTROL_TALK_RESET, "");
        bridge.setSettingValue(Bridge.CALL_CONTROL_TALK_DOWN, "*");
        bridge.setSettingValue(Bridge.CALL_CONTROL_HANGUP, "0");

        testConference(bridge, "conference_config.dtmftest.xml");
    }

    private void testConference(Bridge bridge, String file) throws Exception {
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

        conf = new Conference();
        conf.setModelFilesContext(TestHelper.getModelFilesContext());
        conf.initialize();
        conf.setOwner(owner);
        conf.setExtension("345");
        conf.setSettingValue(Conference.MAX_LEGS, "4");
        conf.setUniqueId();
        conf.setAutorecorded(true);
        conf.setSettingTypedValue(Conference.QUICKSTART, false);
        conf.setSettingValue(Conference.MODERATOR_CODE, "3456");
        conf.setSettingValue(Conference.MOH, "NONE");
        bridge.addConference(conf);

        conf = new Conference();
        conf.setModelFilesContext(TestHelper.getModelFilesContext());
        conf.initialize();
        conf.setOwner(owner);
        conf.setExtension("678");
        conf.setSettingValue(Conference.MAX_LEGS, "4");
        conf.setSettingTypedValue(Conference.VIDEO, true);
        conf.setSettingTypedValue("fs-conf-conference/video-toogle-floor", true);
        conf.setUniqueId();
        conf.setAutorecorded(true);
        bridge.addConference(conf);

        ConferenceBridgeContext confContext = EasyMock.createMock(ConferenceBridgeContext.class);
        confContext.getBridgeByServer("test.example.com");
        EasyMock.expectLastCall().andReturn(bridge).once();
        EasyMock.replay(confContext);

        m_config.setConferenceBridgeContext(confContext);

        StringWriter actual = new StringWriter();
        m_config.writeXml(actual, m_location, m_domain, bridge);
        InputStream referenceXml = getClass().getResourceAsStream(file);
        assertEquals(IOUtils.toString(referenceXml).trim(), actual.toString().trim());
    }

    public void testGenerateNullBridge() throws Exception {
        ConferenceBridgeContext confContext = EasyMock.createMock(ConferenceBridgeContext.class);
        confContext.getBridgeByServer("test.example.com");
        EasyMock.expectLastCall().andReturn(null).once();
        EasyMock.replay(confContext);

        m_config.setConferenceBridgeContext(confContext);

        StringWriter actual = new StringWriter();
        m_config.writeXml(actual, m_location, m_domain, null);
        InputStream referenceXml = getClass().getResourceAsStream("conference_config_null_bridge.test.xml");
        assertEquals(actual.toString().trim(), IOUtils.toString(referenceXml).trim());
    }
}
