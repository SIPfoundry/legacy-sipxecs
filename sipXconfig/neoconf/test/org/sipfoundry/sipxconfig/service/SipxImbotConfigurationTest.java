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

import java.util.Arrays;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class SipxImbotConfigurationTest extends SipxServiceTestBase {
    private SipxImbotService m_imbotService;
    private SipxIvrService m_ivrService;
    private SipxStatusService m_statusService;
    private SipxRestService m_restService;
    private LocationsManager m_locationsManager;
    private SipxServiceManager m_sipxServiceManager;

    @Override
    public void setUp() {
        Location location = createDefaultLocation();
        m_imbotService = new SipxImbotService();
        m_imbotService.setModelDir("scsimbot");
        m_imbotService.setModelName("scsimbot.xml");
        initCommonAttributes(m_imbotService);

        m_imbotService.setDocDir("/usr/share/www/doc");

        m_ivrService = new SipxIvrService();
        m_ivrService.setBeanName(SipxIvrService.BEAN_ID);
        m_ivrService.setModelDir("sipxivr");
        m_ivrService.setModelName("sipxivr.xml");
        m_ivrService.setModelFilesContext(TestHelper.getModelFilesContext());
        m_ivrService.setSettingValue("ivr/httpsPort", "8085");

        m_statusService = new SipxStatusService();
        m_statusService.setBeanName(SipxStatusService.BEAN_ID);
        m_statusService.setHttpsPort(9910);

        m_restService = new SipxRestService();
        m_restService.setModelDir("sipxrest");
        m_restService.setModelName("sipxrest.xml");
        m_restService.setModelFilesContext(TestHelper.getModelFilesContext());
        m_restService.setSettingValue("rest-config/httpsPort", "6666");

        m_locationsManager = createMock(LocationsManager.class);
        m_locationsManager.getLocationByBundle("voicemailBundle");
        expectLastCall().andReturn(location).anyTimes();
        m_locationsManager.getLocationByBundle("managementBundle");
        expectLastCall().andReturn(location).anyTimes();
        m_locationsManager.getLocationsForService(m_restService);
        expectLastCall().andReturn(Arrays.asList(location)).anyTimes();
        replay(m_locationsManager);

        m_restService.setLocationsManager(m_locationsManager);

        m_sipxServiceManager = createMock(SipxServiceManager.class);
        m_sipxServiceManager.getServiceByBeanId(SipxIvrService.BEAN_ID);
        expectLastCall().andReturn(m_ivrService).atLeastOnce();
        m_sipxServiceManager.getServiceByBeanId(SipxImbotService.BEAN_ID);
        expectLastCall().andReturn(m_imbotService).atLeastOnce();
        m_sipxServiceManager.getServiceByBeanId(SipxRestService.BEAN_ID);
        expectLastCall().andReturn(m_restService).atLeastOnce();
        m_sipxServiceManager.isServiceInstalled(location.getId(), SipxIvrService.BEAN_ID);
        expectLastCall().andReturn(true).anyTimes();
        m_sipxServiceManager.isServiceInstalled(location.getId(), SipxImbotService.BEAN_ID);
        expectLastCall().andReturn(true).anyTimes();
        expect(m_sipxServiceManager.getServiceParam("openfire-host")).andReturn("192.168.1.10");
        expect(m_sipxServiceManager.getServiceParam("openfire-xml-rpc-port")).andReturn(49094);
        replay(m_sipxServiceManager);
    }

    public void testWrite() throws Exception {
        SipxImbotConfiguration out = new SipxImbotConfiguration();
        out.setSipxServiceManager(m_sipxServiceManager);
        out.setTemplate("scsimbot/scsimbot.properties.vm");
        out.setLocationsManager(m_locationsManager);
        assertCorrectFileGeneration(out, "expected-sipximbot.properties");

        verify(m_sipxServiceManager);
    }

    @Override
    protected Location createDefaultLocation() {
        Location location = super.createDefaultLocation();
        location.setAddress("192.168.1.2");
        location.setFqdn("puppy.org");
        return location;
    }
}
