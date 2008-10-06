/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.service;

import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxPresenceConfigurationTest extends SipxServiceTestBase {
    
    public void testWrite() throws Exception {
        SipxPresenceConfiguration out = new SipxPresenceConfiguration();
        out.setVelocityEngine(TestHelper.getVelocityEngine());
        out.setTemplate("sipxpresence/sipxpresence-config.vm");
        
        SipxPresenceService presenceService = new SipxPresenceService();
        initCommonAttributes(presenceService);
        Setting settings = TestHelper.loadSettings("sipxpresence/sipxpresence.xml");
        presenceService.setSettings(settings);

        Setting presenceSettings = presenceService.getSettings().getSetting("presence-config");
        presenceSettings.getSetting("SIP_PRESENCE_LOG_LEVEL").setValue("CRIT");
        presenceSettings.getSetting("SIP_PRESENCE_SIGN_IN_CODE").setValue("*76");
        presenceSettings.getSetting("SIP_PRESENCE_SIGN_OUT_CODE").setValue("*77");
        presenceSettings.getSetting("PRESENCE_SERVER_SIP_PORT").setValue("5131");
        presenceSettings.getSetting("PRESENCE_SERVER_HTTP_PORT").setValue("8112");

        out.generate(presenceService);
        
        Location location = new Location();
        location.setName("localLocation");
        location.setFqdn("mysystem.realm.example.org");
        location.setAddress("192.168.1.1");

        StringWriter actualConfigWriter = new StringWriter();
        out.write(actualConfigWriter, location);
        
        Reader referenceConfigReader = new InputStreamReader(SipxPresenceConfigurationTest.class
                .getResourceAsStream("expected-presence-config"));
        String referenceConfig = IOUtils.toString(referenceConfigReader);
        
        Reader actualConfigReader = new StringReader(actualConfigWriter.toString());
        String actualConfig = IOUtils.toString(actualConfigReader);

        assertEquals(referenceConfig, actualConfig);
    }
}
