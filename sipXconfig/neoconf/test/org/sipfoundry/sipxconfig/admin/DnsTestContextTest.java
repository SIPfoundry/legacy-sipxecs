/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import junit.framework.TestCase;

import org.easymock.classextension.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.configdiag.ExternalCommandContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.setting.Setting;

import static java.util.Collections.singleton;

public class DnsTestContextTest extends TestCase {
    DnsTestContextImpl m_dnsTestContext;

    @Override
    protected void setUp() {
        ExternalCommandContext commandContext = EasyMock.createMock(ExternalCommandContext.class);
        String command = getClass().getClassLoader().getResource(
                "org/sipfoundry/sipxconfig/admin/mock-dns-test.sh").getFile();
        m_dnsTestContext = new DnsTestContextImpl(commandContext, command);

        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.getDomainName();
        expectLastCall().andReturn("example.org").anyTimes();

        Location l1 = new Location();
        l1.setFqdn("test1.example.org");
        l1.setAddress("1.2.3.4");
        Location l2 = new Location();
        l2.setFqdn("test2.example.org");
        l2.setAddress("4.3.2.1");
        LocationsManager locationsManager = createMock(LocationsManager.class);
        locationsManager.getLocations();
        expectLastCall().andReturn(new Location[] {l1,l2}).anyTimes();

        SipxService proxyService = new SipxProxyService();
        proxyService.setProcessName("SIPXProxy");
        proxyService.setBeanName(SipxProxyService.BEAN_ID);
        proxyService.setModelDir("sipxproxy");
        proxyService.setModelName("sipxproxy.xml");
        proxyService.setModelFilesContext(TestHelper.getModelFilesContext());
        Setting proxySettings = proxyService.getSettings();
        proxySettings.getSetting("proxy-configuration/SIP_PORT").setValue("5061");
        l1.setServiceDefinitions(singleton(proxyService));
        SipxServiceManager serviceManager = createMock(SipxServiceManager.class);
        serviceManager.getServiceByBeanId(SipxProxyService.BEAN_ID);
        expectLastCall().andReturn(proxyService).anyTimes();
        serviceManager.getServiceByBeanId("sipxOpenfireService");
        expectLastCall().andReturn(null).anyTimes();

        replay(coreContext,locationsManager, serviceManager);

        m_dnsTestContext.setCoreContext(coreContext);
        m_dnsTestContext.setLocationsManager(locationsManager);
        m_dnsTestContext.setSipxServiceManager(serviceManager);
    }

    public void testValid() {
        m_dnsTestContext.execute(false);
        assertEquals("DNS Records", m_dnsTestContext.getResult());
    }

    public void testInvalid() {
        m_dnsTestContext.execute(true);
        assertEquals("DNS Configuration ERROR", m_dnsTestContext.getResult());
    }
}
