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

import org.sipfoundry.sipxconfig.TestHelper;

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
        
        assertCorrectFileGeneration(out, "expected-mediaserver-config");
    }
}
