/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import java.io.File;

import org.apache.commons.io.FileUtils;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class ConferenceConfigurationTest extends XMLTestCase {

    @Override
    protected void setUp() throws Exception {
        XMLUnit.setIgnoreWhitespace(true);
    }

    public void testGenerate() throws Exception {
        Bridge bridge;

        bridge = new Bridge();
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
        conf.setExtension("234");
        conf.setSettingValue(Conference.MAX_LEGS, "4");
        conf.setUniqueId();
        bridge.addConference(conf);

        Domain domain = new Domain("example.com");
        IMocksControl control = EasyMock.createControl();
        DomainManager domainManager = control.createMock(DomainManager.class);
        domainManager.getDomain();
        EasyMock.expectLastCall().andReturn(domain);
        EasyMock.replay(domainManager);

        ConferenceBridgeContext confContext = EasyMock.createMock(ConferenceBridgeContext.class);
        confContext.getBridgeByServer("test.example.com");
        EasyMock.expectLastCall().andReturn(bridge).anyTimes();
        EasyMock.replay(confContext);

        Location location = new Location();
        location.setFqdn("test.example.com");

        ConferenceConfiguration config = new ConferenceConfiguration();
        config.setDomainManager(domainManager);
        config.setConferenceBridgeContext(confContext);

        String generatedXml = config.getFileContent(location);
        System.err.println(generatedXml);
        /*
         * We use two files for reversed order of the "profile" elements of the xml because the
         * HashSet containing the conferences doesn't guarantee some specific order
         */
        String referenceXml = FileUtils.readFileToString(new File(TestUtil
                .getTestSourceDirectory(ConferenceConfigurationTest.class), "conference_config.test.xml"));
        String alternativeReferenceXml = FileUtils
                .readFileToString(new File(TestUtil.getTestSourceDirectory(ConferenceConfigurationTest.class),
                        "conference_config_alternative.test.xml"));
        /*
         * We trim all the \t \n and spaces from the xml strings in order for the comparison not
         * to be ruined by different formatting
         */
        generatedXml = generatedXml.replace(" ", "").replace("\t", "").replace("\n", "");
        referenceXml = referenceXml.replace(" ", "").replace("\t", "").replace("\n", "");
        alternativeReferenceXml = alternativeReferenceXml.replace(" ", "").replace("\t", "").replace("\n", "");
        assertTrue(generatedXml.equals(referenceXml) || generatedXml.equals(alternativeReferenceXml));
    }
}
