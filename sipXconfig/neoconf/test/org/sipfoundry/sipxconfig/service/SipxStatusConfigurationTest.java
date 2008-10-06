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

public class SipxStatusConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        SipxStatusService statusService = new SipxStatusService();
        initCommonAttributes(statusService);
        statusService.setSettings(TestHelper.loadSettings("sipxstatus/sipxstatus.xml"));
        statusService.setHttpPort(8100);
        statusService.setHttpsPort(8101);
        statusService.setStatusServerSipPort(5110);
        
        Setting statusConfigSettings = statusService.getSettings().getSetting("status-config");
        statusConfigSettings.getSetting("SIP_STATUS_LOG_LEVEL").setValue("CRIT");
        
        SipxStatusConfiguration out = new SipxStatusConfiguration();
        out.setVelocityEngine(TestHelper.getVelocityEngine());
        out.setTemplate("sipxstatus/status-config.vm");
        out.generate(statusService);

        Location location = new Location();
        location.setName("localLocation");
        location.setFqdn("sipx.example.org");
        location.setAddress("192.168.1.2");
        StringWriter actualConfigWriter = new StringWriter();
        out.write(actualConfigWriter, location);
        
        Reader referenceConfigReader = new InputStreamReader(SipxStatusConfigurationTest.class
                .getResourceAsStream("expected-status-config"));
        String referenceConfig = IOUtils.toString(referenceConfigReader);

        Reader actualConfigReader = new StringReader(actualConfigWriter.toString());
        String actualConfig = IOUtils.toString(actualConfigReader);

        assertEquals(referenceConfig, actualConfig);
    }
}
