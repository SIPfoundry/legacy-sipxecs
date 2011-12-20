/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
        addressManager.getSingleAddress(AdminContext.HTTPS_ADDRESS);
        expectLastCall().andReturn(new Address("admin.example.org", 100)).once();
        replay(addressManager);        
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        settings.setAddressManager(addressManager);
        settings.setCoreContext(coreContext);
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        StringWriter actual = new StringWriter();
        config.write(actual, settings);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-sipxprovision-config"));
        assertEquals(expected, actual.toString());
    }
}
