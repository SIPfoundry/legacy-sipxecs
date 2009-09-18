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
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class SipxRestConfigurationTest extends SipxServiceTestBase {
    public void testWrite() throws Exception {
        SipxRestService restService = new SipxRestService();
        restService.setBeanId(SipxRestService.BEAN_ID);
        restService.setModelDir("sipxrest");
        restService.setModelName("sipxrest.xml");
        restService.setModelFilesContext(TestHelper.getModelFilesContext());
        initCommonAttributes(restService);

        DomainManager domainManager = TestUtil.getMockDomainManager();
        domainManager.getAuthorizationRealm();
        EasyMock.expectLastCall().andReturn("example.org");
        EasyMock.replay(domainManager);
        restService.setDomainManager(domainManager);

        Location primary = new Location();
        primary.setPrimary(true);
        primary.setFqdn("localhost");
        primary.setAddress("192.168.1.1");
        primary.setName("primary");

        LocationsManager locationManager = EasyMock.createMock(LocationsManager.class);

        locationManager.getPrimaryLocation();
        EasyMock.expectLastCall().andReturn(primary).anyTimes();
        locationManager.getLocationsForService(restService);
        EasyMock.expectLastCall().andReturn(Arrays.asList()).anyTimes();

        EasyMock.replay(locationManager);

        Setting logSetting = restService.getSettings().getSetting("rest-config/SIPX_REST_LOG_LEVEL");
        logSetting.setValue("CRIT");

        Setting sipPortSetting = restService.getSettings().getSetting("rest-config/sipPort");
        sipPortSetting.setValue("6050");

        Setting httpsPortSetting = restService.getSettings().getSetting("rest-config/httpsPort");
        httpsPortSetting.setValue("6666");

        Setting extHttpsPortSetting = restService.getSettings().getSetting("rest-config/extHttpPort");
        extHttpsPortSetting.setValue("6667");

        SipxRestConfiguration out = new SipxRestConfiguration();

        SipxServiceManager sipxServiceManager = TestUtil.getMockSipxServiceManager(true, restService);
        out.setSipxServiceManager(sipxServiceManager);

        out.setTemplate("sipxrest/sipxrest-config.vm");

        assertCorrectFileGeneration(out, "expected-sipxrest-config");
    }
}
