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

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.setting.Setting;

import junit.framework.TestCase;

public class SipxPresenceServiceTest extends TestCase {

    private SipxPresenceService m_out;

    public void setUp() {
        m_out = new SipxPresenceService();
        m_out.setModelDir("sipxpresence");
        m_out.setModelName("sipxpresence.xml");
        m_out.setModelFilesContext(TestHelper.getModelFilesContext());

        Setting presenceSettings = m_out.getSettings();
        presenceSettings.getSetting("presence-config/SIP_PRESENCE_SIGN_OUT_CODE").setValue("*123");
        presenceSettings.getSetting("presence-config/SIP_PRESENCE_SIGN_IN_CODE").setValue("*122");
        presenceSettings.getSetting("presence-config/PRESENCE_SERVER_SIP_PORT").setValue("1234");
        presenceSettings.getSetting("presence-config/SIP_PRESENCE_HTTP_PORT").setValue("8888");
    }

    public void testValidateDuplicateCodes() {
        SipxRegistrarService registrarService = new SipxRegistrarService();
        registrarService.setModelDir("sipxregistrar");
        registrarService.setModelName("sipxregistrar.xml");
        registrarService.setModelFilesContext(TestHelper.getModelFilesContext());
        IMocksControl domainManagerControl = EasyMock.createControl();
        DomainManager domainManager = domainManagerControl.createMock(DomainManager.class);
        domainManager.getDomain();
        EasyMock.expectLastCall().andReturn(new Domain()).anyTimes();
        EasyMock.replay(domainManager);
        registrarService.setDomainManager(domainManager);


        SipxServiceManager serviceManager = EasyMock.createMock(SipxServiceManager.class);
        serviceManager.getServiceByBeanId(SipxRegistrarService.BEAN_ID);
        EasyMock.expectLastCall().andReturn(registrarService).anyTimes();
        EasyMock.replay(serviceManager);
        m_out.setSipxServiceManager(serviceManager);

        Setting registrarSettings = registrarService.getSettings();
        registrarSettings.getSetting(
                "call-pick-up/SIP_REDIRECT.100-PICKUP.DIRECTED_CALL_PICKUP_CODE")
                .setValue("*123");

        try {
            m_out.validate();
            fail("Expected validation exception due to duplicate codes.");
        } catch (UserException e) {
            // expected
        }
    }

    public void testGetPresenceServerPort() {
        assertEquals(1234, m_out.getPresenceServerPort());
    }

    public void testGetPresenceApiPort() {
        assertEquals(8888, m_out.getPresenceApiPort());
    }
}
