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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class SipxRecordingConfigurationTest extends SipxServiceTestBase {
    private SipxRecordingService m_recordingService;
    private SipxServiceManager m_sipxServiceManager;

    @Override
    public void setUp() {
        m_recordingService = new SipxRecordingService();
        m_recordingService.setModelDir("sipxrecording");
        m_recordingService.setModelName("sipxrecording.xml");
        initCommonAttributes(m_recordingService);

        m_recordingService.setDocDir("/usr/share/www/doc");

        m_sipxServiceManager = createMock(SipxServiceManager.class);
        m_sipxServiceManager.getServiceByBeanId(SipxRecordingService.BEAN_ID);
        expectLastCall().andReturn(m_recordingService).atLeastOnce();
    }

    public void testWrite() throws Exception {
        replay(m_sipxServiceManager);

        SipxRecordingConfiguration out = new SipxRecordingConfiguration();
        out.setSipxServiceManager(m_sipxServiceManager);
        out.setTemplate("sipxrecording/sipxrecording.properties.vm");

        assertCorrectFileGeneration(out, "expected-sipxrecording.properties");

        verify(m_sipxServiceManager);
    }
}
