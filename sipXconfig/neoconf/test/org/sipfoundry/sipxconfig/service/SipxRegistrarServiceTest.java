/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxRegistrarServiceTest extends TestCase {

    private SipxRegistrarService m_out;

    @Override
    public void setUp() throws Exception {
        m_out = new SipxRegistrarService();
        m_out.setModelDir("sipxregistrar");
        m_out.setModelName("sipxregistrar.xml");
        m_out.setModelFilesContext(TestHelper.getModelFilesContext());

        IMocksControl domainManagerControl = EasyMock.createControl();
        DomainManager domainManager = domainManagerControl.createMock(DomainManager.class);
        domainManager.getDomain();
        EasyMock.expectLastCall().andReturn(new Domain()).anyTimes();
        EasyMock.replay(domainManager);

        m_out.setDomainManager(domainManager);
    }

    public void testValidateDuplicateCodes() {
        SipxService presenceService = new SipxPresenceService();
        presenceService.setModelDir("sipxpresence");
        presenceService.setModelName("sipxpresence.xml");
        presenceService.setModelFilesContext(TestHelper.getModelFilesContext());

        SipxServiceManager serviceManager = EasyMock.createMock(SipxServiceManager.class);
        serviceManager.getServiceByBeanId(SipxPresenceService.BEAN_ID);
        EasyMock.expectLastCall().andReturn(presenceService).anyTimes();
        EasyMock.replay(serviceManager);
        m_out.setSipxServiceManager(serviceManager);

        Setting registrarSettings = m_out.getSettings();
        Setting presenceSettings = presenceService.getSettings();

        presenceSettings.getSetting("presence-config/SIP_PRESENCE_SIGN_IN_CODE").setValue("*123");
        registrarSettings.getSetting("call-pick-up/SIP_REDIRECT.100-PICKUP.DIRECTED_CALL_PICKUP_CODE").setValue("*123");

        try {
            m_out.validate();
            fail("Expected validation exception due to duplicate codes.");
        } catch (UserException e) {
            // expected
        }
    }

    public void testFormatMappingLocation() {
        Location l = new Location();
        l.setAddress("10.1.1.1");
        l.setFqdn("a.example.org");

        m_out.setSipPort("5070");

        assertEquals("a.example.org:5070;transport=tcp", m_out.formatMappingLocation(l, false));
        assertEquals("rr.a.example.org", m_out.formatMappingLocation(l, true));
    }
}
