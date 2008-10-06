/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.service;

import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;

public class SipxMediaConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        SipxMediaService mediaService = new SipxMediaService();
        mediaService.setSettings(TestHelper.loadSettings("sipxvxml/mediaserver.xml"));
        initCommonAttributes(mediaService);
        mediaService.setIpAddress("192.168.1.1");
        mediaService.setHttpPort(8090);
        
        SipxMediaConfiguration out = new SipxMediaConfiguration();
        out.setTemplate("sipxvxml/mediaserver-config.vm");
        out.setVelocityEngine(TestHelper.getVelocityEngine());
        
        out.generate(mediaService);

        Location location = new Location();
        location.setName("localLocation");
        location.setFqdn("sipx.example.org");
        location.setAddress("192.168.1.2");
        StringWriter actualConfigWriter = new StringWriter();
        out.write(actualConfigWriter, location);

        Reader referenceConfigReader = new InputStreamReader(SipxProxyConfigurationTest.class
                .getResourceAsStream("expected-mediaserver-config"));
        String referenceConfig = IOUtils.toString(referenceConfigReader);

        Reader actualConfigReader = new StringReader(actualConfigWriter.toString());
        String actualConfig = IOUtils.toString(actualConfigReader);

        assertEquals(referenceConfig, actualConfig);
        
        //assertCorrectFileGeneration(out, "expected-mediaserver-config");
    }
}
