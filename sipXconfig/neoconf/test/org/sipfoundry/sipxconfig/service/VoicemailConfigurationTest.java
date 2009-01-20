/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class VoicemailConfigurationTest extends SipxServiceTestBase {
    public void testWrite() throws Exception {
        SipxMediaService mediaService = new SipxMediaService();
        mediaService.setBeanId(SipxMediaService.BEAN_ID);
        mediaService.setModelDir("sipxvxml");
        mediaService.setModelName("mediaserver.xml");
        mediaService.setModelFilesContext(TestHelper.getModelFilesContext());
        Setting voicemailAgingSetting = mediaService.getSettings().getSetting("mediaserver-config/VOICEMAIL_AGE_LIMIT");
        voicemailAgingSetting.setValue("10");
        
        SipxServiceManager serviceManager = TestUtil.getMockSipxServiceManager(mediaService);
        EasyMock.replay(serviceManager);
        
        VoicemailConfiguration out = new VoicemailConfiguration();
        out.setSipxServiceManager(serviceManager);
        
        out.setTemplate("sipxvxml/voicemail-config.vm");
        assertCorrectFileGeneration(out, "expected-voicemail-config");
    }
}
