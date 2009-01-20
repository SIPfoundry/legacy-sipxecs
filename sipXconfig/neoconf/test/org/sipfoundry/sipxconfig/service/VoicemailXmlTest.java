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
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class VoicemailXmlTest extends SipxServiceTestBase {
    public void testWrite() throws Exception {
        SipxMediaService mediaService = new SipxMediaService();
        mediaService.setModelId(SipxMediaService.BEAN_ID);
        mediaService.setSettings(TestHelper.loadSettings("sipxvxml/mediaserver.xml"));
        initCommonAttributes(mediaService);
        mediaService.setVoicemailHttpPort(9905);
        mediaService.setVoicemailHttpsPort(9906);
        Setting logSetting = mediaService.getSettings().getSetting("mediaserver-config/SIPX_MEDIA_SERVER_LOG_LEVEL");
        logSetting.setValue("CRIT");

        SipxStatusService statusService= new SipxStatusService();
        statusService.setModelId(SipxStatusService.BEAN_ID);
        statusService.setHttpsPort(9910);

        SipxServiceManager sipxServiceManager = TestUtil.getMockSipxServiceManager(mediaService, statusService);
        replay(sipxServiceManager);

        VoicemailXml out = new VoicemailXml();
        out.setSipxServiceManager(sipxServiceManager);
        out.setTemplate("sipxvxml/voicemail.xml.vm");

        assertCorrectFileGeneration(out, "expected-voicemail.xml");

        verify(sipxServiceManager);
    }
}
