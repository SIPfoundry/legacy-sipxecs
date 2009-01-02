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
import org.sipfoundry.sipxconfig.admin.commserver.Location;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class SipxMediaConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        SipxMediaService mediaService = new SipxMediaService();
        mediaService.setSettings(TestHelper.loadSettings("sipxvxml/mediaserver.xml"));
        initCommonAttributes(mediaService);
        mediaService.setVoicemailHttpPort(8090);

        SipxServiceManager sipxServiceManager = createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxMediaService.BEAN_ID);
        expectLastCall().andReturn(mediaService).atLeastOnce();
        replay(sipxServiceManager);

        SipxMediaConfiguration out = new SipxMediaConfiguration();
        out.setSipxServiceManager(sipxServiceManager);
        out.setTemplate("sipxvxml/mediaserver-config.vm");

        assertCorrectFileGeneration(out, "expected-mediaserver-config");

        verify(sipxServiceManager);
    }


    @Override
    protected Location createDefaultLocation() {
        Location location = super.createDefaultLocation();
        location.setAddress("192.168.1.2");
        return location;
    }
}
