/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.provision;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.junit.Assert.assertEquals;

import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.test.TestHelper;


public class ProvisionConfigurationTest {

    @Test
    public void testConfig() throws Exception {
        ProvisionConfiguration config = new ProvisionConfiguration();
        ProvisionSettings settings = new ProvisionSettings();
        User provisionUser = new User();
        provisionUser.setUserName("provisioner");
        provisionUser.setSipPassword("1234Password");
        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.getDomainName();
        expectLastCall().andReturn("example.org").anyTimes();
        coreContext.getSpecialUser(SpecialUserType.PHONE_PROVISION);
        expectLastCall().andReturn(provisionUser).once();
        replay(coreContext);
        AddressManager addressManager = createMock(AddressManager.class);
        addressManager.getSingleAddress(AdminContext.HTTP_ADDRESS);
        expectLastCall().andReturn(new Address(AdminContext.HTTP_ADDRESS, "admin.example.org", 12000)).once();
        replay(addressManager);
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        settings.setAddressManager(addressManager);
        settings.setCoreContext(coreContext);
        StringWriter actual = new StringWriter();
        config.write(actual, settings);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-sipxprovision-config"));
        assertEquals(expected, actual.toString());
    }
}
