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

import java.util.Arrays;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class SipxIvrConfigurationTest extends SipxServiceTestBase {
    private SipxIvrService m_ivrService;
    private SipxStatusService m_statusService;
    private SipxRestService m_restService;
    private SipxImbotService m_imbotService;
    private LocationsManager m_locationsManager;
    private SipxServiceManager m_sipxServiceManager;

    @Override
    public void setUp() {
        Location location = createDefaultLocation();
        m_ivrService = new SipxIvrService();
        m_ivrService.setModelDir("sipxivr");
        m_ivrService.setModelName("sipxivr.xml");
        initCommonAttributes(m_ivrService);

        m_ivrService.setMailstoreDir("/var/sipxdata/mediaserver/data/mailstore");
        m_ivrService.setPromptsDir("/var/sipxdata/mediaserver/data/prompts");
        m_ivrService.setScriptsDir("/usr/share/www/doc/aa_vxml");
        m_ivrService.setDocDir("/usr/share/www/doc");
        m_ivrService.setVxmlDir("/var/sipxdata/mediaserver/data");

        m_statusService = new SipxStatusService();
        m_statusService.setBeanName(SipxStatusService.BEAN_ID);
        m_statusService.setHttpsPort(9910);

        m_restService = new SipxRestService();
        m_restService.setModelDir("sipxrest");
        m_restService.setModelName("sipxrest.xml");

        m_restService.setModelFilesContext(TestHelper.getModelFilesContext());
        m_restService.setSettingValue("rest-config/httpsPort", "6666");

        m_imbotService = new SipxImbotService();
        m_imbotService.setModelDir("sipximbot");
        m_imbotService.setModelName("sipximbot.xml");
        m_imbotService.setModelFilesContext(TestHelper.getModelFilesContext());
        m_imbotService.setSettingValue("imbot/httpPort", "8086");

        m_locationsManager = createMock(LocationsManager.class);
        m_locationsManager.getLocationsForService(m_restService);
        expectLastCall().andReturn(Arrays.asList(location)).anyTimes();

        replay(m_locationsManager);

        m_restService.setLocationsManager(m_locationsManager);

        m_sipxServiceManager = createMock(SipxServiceManager.class);
        m_sipxServiceManager.getServiceByBeanId(SipxStatusService.BEAN_ID);
        expectLastCall().andReturn(m_statusService).atLeastOnce();
        m_sipxServiceManager.getServiceByBeanId(SipxIvrService.BEAN_ID);
        expectLastCall().andReturn(m_ivrService).atLeastOnce();
        m_sipxServiceManager.getServiceByBeanId(SipxRestService.BEAN_ID);
        expectLastCall().andReturn(m_restService).atLeastOnce();
        m_sipxServiceManager.getServiceByBeanId(SipxImbotService.BEAN_ID);
        expectLastCall().andReturn(m_imbotService).atLeastOnce();
    }

    public void testWriteWithoutOpenfireService() throws Exception {

        expect(m_sipxServiceManager.isServiceInstalled("sipxOpenfireService")).andReturn(false);

        replay(m_sipxServiceManager);

        SipxIvrConfiguration out = new SipxIvrConfiguration();
        out.setSipxServiceManager(m_sipxServiceManager);
        out.setTemplate("sipxivr/sipxivr.properties.vm");

        assertCorrectFileGeneration(out, "expected-sipxivr-without-openfire.properties");

        verify(m_sipxServiceManager);
    }

    public void testWriteWithOpenfireService() throws Exception {

        expect(m_sipxServiceManager.isServiceInstalled("sipxOpenfireService")).andReturn(true);
        expect(m_sipxServiceManager.getServiceParam("openfire-host")).andReturn("192.168.1.10").atLeastOnce();
        expect(m_sipxServiceManager.getServiceParam("openfire-xml-rpc-port")).andReturn(49094);

        replay(m_sipxServiceManager);

        SipxIvrConfiguration out = new SipxIvrConfiguration();
        out.setSipxServiceManager(m_sipxServiceManager);
        out.setTemplate("sipxivr/sipxivr.properties.vm");

        assertCorrectFileGeneration(out, "expected-sipxivr-with-openfire.properties");

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
