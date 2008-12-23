/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class VoicemailConfigurationTest extends SipxServiceTestBase {
    public void testWrite() throws Exception {
        SipxMediaService mediaService = new SipxMediaService();
        mediaService.setModelId(SipxMediaService.BEAN_ID);
        mediaService.setSettings(TestHelper.loadSettings("sipxvxml/mediaserver.xml"));
        initCommonAttributes(mediaService);
        mediaService.setHttpPort(9905);
        mediaService.setVoicemailHttpsPort("9906");

        SipxStatusService statusService= new SipxStatusService();
        statusService.setModelId(SipxStatusService.BEAN_ID);
        statusService.setHttpsPort(9910);

        SipxServiceManager sipxServiceManager = TestUtil.getMockSipxServiceManager(mediaService, statusService);
        replay(sipxServiceManager);

        VoicemailConfiguration out = new VoicemailConfiguration();
        out.setSipxServiceManager(sipxServiceManager);
        out.setTemplate("sipxvxml/voicemail.xml.vm");

        assertCorrectFileGeneration(out, "expected-voicemail.xml");

        verify(sipxServiceManager);
    }
}
