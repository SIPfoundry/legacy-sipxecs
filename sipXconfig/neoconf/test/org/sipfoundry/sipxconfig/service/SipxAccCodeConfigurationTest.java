/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import java.util.Arrays;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.setting.Setting;


import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class SipxAccCodeConfigurationTest extends SipxServiceTestBase {
    private SipxAccCodeService m_acccodeService;
    private SipxServiceManager m_sipxServiceManager;

    @Override
    public void setUp() {
        m_acccodeService = new SipxAccCodeService();
        m_acccodeService.setModelDir("sipxacccode");
        m_acccodeService.setModelName("sipxacccode.xml");
        Setting settings = TestHelper.loadSettings("sipxacccode/sipxacccode.xml");
        m_acccodeService.setSettings(settings);
        initCommonAttributes(m_acccodeService);

        m_acccodeService.setPromptsDir("/var/sipxdata/mediaserver/data/prompts");
        m_acccodeService.setDocDir("/usr/share/www/doc");

        m_sipxServiceManager = createMock(SipxServiceManager.class);
        m_sipxServiceManager.getServiceByBeanId(SipxAccCodeService.BEAN_ID);
        expectLastCall().andReturn(m_acccodeService).atLeastOnce();
        replay(m_sipxServiceManager);
    }

    public void testWrite() throws Exception {
        SipxAccCodeConfiguration out = new SipxAccCodeConfiguration();
        out.setTemplate("sipxacccode/sipxacccode.properties.vm");
        out.setSipxServiceManager(m_sipxServiceManager);

        assertCorrectFileGeneration(out, "expected-sipxacccode.properties");

        verify(m_sipxServiceManager);
    }
}
