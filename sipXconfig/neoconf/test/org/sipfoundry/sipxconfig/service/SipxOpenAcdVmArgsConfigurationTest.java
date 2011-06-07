/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.service;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class SipxOpenAcdVmArgsConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        SipxOpenAcdService openAcdService = new SipxOpenAcdService();
        initCommonAttributes(openAcdService);
        Setting settings = TestHelper.loadSettings("openacd/sipxopenacd.xml");
        openAcdService.setSettings(settings);

        LocationsManager locationsManager = createMock(LocationsManager.class);
        Location primaryLocation = TestUtil.createDefaultLocation();
        locationsManager.getPrimaryLocation();
        expectLastCall().andReturn(primaryLocation).anyTimes();

        SipxServiceManager sipxServiceManager = createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxOpenAcdService.BEAN_ID);
        expectLastCall().andReturn(openAcdService).atLeastOnce();

        replay(locationsManager, sipxServiceManager);

        SipxOpenAcdVmArgsConfiguration out = new SipxOpenAcdVmArgsConfiguration();
        out.setEtcDir("/home/sipxecs/etc");
        out.setLocationsManager(locationsManager);
        out.setSipxServiceManager(sipxServiceManager);
        out.setTemplate("openacd/vm.args.vm");

        assertCorrectFileGeneration(out, "expected-vm-args-config");

    }
}
