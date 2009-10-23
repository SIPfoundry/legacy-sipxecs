/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import java.util.Arrays;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class SipxProvisionConfigurationTest extends SipxServiceTestBase {
    public void testWrite() throws Exception {
        SipxProvisionService provisionService = new SipxProvisionService();
        provisionService.setBeanId(SipxProvisionService.BEAN_ID);
        provisionService.setModelDir("sipxprovision");
        provisionService.setModelName("sipxprovision.xml");
        provisionService.setModelFilesContext(TestHelper.getModelFilesContext());
        provisionService.setTftproot(TestHelper.getTestDirectory() + "/tftproot");
        initCommonAttributes(provisionService);

        DomainManager domainManager = TestUtil.getMockDomainManager();
        domainManager.getAuthorizationRealm();
        EasyMock.expectLastCall().andReturn("example.org");
        EasyMock.replay(domainManager);
        provisionService.setDomainManager(domainManager);

        Location primary = new Location();
        primary.setPrimary(true);
        primary.setFqdn("localhost");
        primary.setAddress("192.168.1.1");
        primary.setName("primary");

        LocationsManager locationsManager = EasyMock.createMock(LocationsManager.class);
        provisionService.setLocationsManager(locationsManager);

        locationsManager.getPrimaryLocation();
        EasyMock.expectLastCall().andReturn(primary).anyTimes();
        locationsManager.getLocationsForService(provisionService);
        EasyMock.expectLastCall().andReturn(Arrays.asList()).anyTimes();

        EasyMock.replay(locationsManager);

        CoreContext coreContext = EasyMock.createMock(CoreContext.class);
        provisionService.setCoreContext(coreContext);
        User phoneProvisionUser = new User();
        phoneProvisionUser.setSipPassword("1234Password");
        coreContext.getSpecialUser(SpecialUserType.PHONE_PROVISION);
        EasyMock.expectLastCall().andReturn(phoneProvisionUser).anyTimes();

        EasyMock.replay(coreContext);

        Setting logSetting = provisionService.getSettings().getSetting("provision-config/SIPX_PROV_LOG_LEVEL");
        logSetting.setValue("CRIT");

        Setting servletPortSetting = provisionService.getSettings().getSetting("provision-config/servletPort");
        servletPortSetting.setValue("6050");

        SipxProvisionConfiguration out = new SipxProvisionConfiguration();

        SipxServiceManager sipxServiceManager = TestUtil.getMockSipxServiceManager(true, provisionService);
        out.setSipxServiceManager(sipxServiceManager);

        out.setTemplate("sipxprovision/sipxprovision-config.vm");

        assertCorrectFileGeneration(out, "expected-sipxprovision-config");
    }
}
