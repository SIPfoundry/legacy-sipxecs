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


public class SipxParkConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        SipxParkConfiguration out = new SipxParkConfiguration();
        out.setVelocityEngine(TestHelper.getVelocityEngine());
        out.setTemplate("sipxpark/sipxpark-config.vm");
        
        SipxParkService parkService = new SipxParkService();
        initCommonAttributes(parkService);
        parkService.setParkServerSipPort("9999");
        Setting settings = TestHelper.loadSettings("sipxpark/sipxpark.xml");
        parkService.setSettings(settings);
        
        Setting parkSettings = parkService.getSettings().getSetting("park-config");
        parkSettings.getSetting("SIP_PARK_LOG_LEVEL").setValue("CRIT");
        
        out.generate(parkService);
        
        Location location = new Location();
        location.setName("localLocation");
        location.setFqdn("mysystem.realm.example.org");
        location.setAddress("192.168.1.2");

        StringWriter actualConfigWriter = new StringWriter();
        out.write(actualConfigWriter, location);
        
        Reader referenceConfigReader = new InputStreamReader(SipxParkConfigurationTest.class
                .getResourceAsStream("expected-park-config"));
        String referenceConfig = IOUtils.toString(referenceConfigReader);
        
        Reader actualConfigReader = new StringReader(actualConfigWriter.toString());
        String actualConfig = IOUtils.toString(actualConfigReader);

        assertEquals(referenceConfig, actualConfig);
    }
}
