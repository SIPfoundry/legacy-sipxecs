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

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;

public class ConferenceConfigurationTest extends XMLTestCase {

    @Override
    protected void setUp() throws Exception {
        XMLUnit.setIgnoreWhitespace(true);
    }

    public void testGenerate() throws Exception {
        List conferences;
        Bridge bridge;

        conferences = new ArrayList(2);

        bridge = new Bridge();
        bridge.setModelFilesContext(TestHelper.getModelFilesContext());
        DeviceDefaults dd = new DeviceDefaults();
        dd.setDomainManager(TestHelper.getTestDomainManager("example.com")) ;
        bridge.setSystemDefaults(dd);
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
        conf.setSettingValue(Conference.MAX_LEGS, "0") ;
        bridge.addConference(conf);

        conf = new Conference();
        conf.setModelFilesContext(TestHelper.getModelFilesContext());
        conf.initialize();
        conf.setExtension("234");
        conf.setSettingValue(Conference.MAX_LEGS, "4") ;
        bridge.addConference(conf);
        conferences.add(conf);

        ConferenceConfiguration config = new ConferenceConfiguration();
        config.generate(bridge, conferences);
        String generatedXml = config.getFileContent();
        System.err.println(generatedXml);
        InputStream referenceXml = ConferenceConfigurationTest.class
                .getResourceAsStream("conference_config.test.xml");
        assertXMLEqual(new InputStreamReader(referenceXml), new StringReader(generatedXml));
    }
}
