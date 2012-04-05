/**
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
package org.sipfoundry.sipxconfig.dhcp;

import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.io.StringWriter;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class DhcpConfigTest {
    
    @Test
    public void config() throws IOException {
        DhcpConfig config = new DhcpConfig();
        DhcpSettings settings = new DhcpSettings();
        settings.setPrimary(TestHelper.createDefaultLocation());
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        AddressType t = new AddressType("t");
        Address tftp = new Address(t, "tftp");
        Address admin = new Address(t, "admin");
        List<Address> dns = Arrays.asList(new Address(t, "dns1"), new Address(t, "dns2"));
        List<Address> ntp = Arrays.asList(new Address(t, "ntp1"), new Address(t, "ntp2"));
        StringWriter actual = new StringWriter();
        config.writeConfig(actual, settings, tftp, admin, dns, ntp);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-dhcpd.conf"));
        assertEquals(expected, actual.toString());
    }
}
