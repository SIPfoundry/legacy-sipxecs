/**
 *
 *
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import static org.easymock.EasyMock.anyObject;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.sipfoundry.sipxconfig.test.TestHelper.getMockDomainManager;

import java.util.Arrays;
import java.util.List;
import java.util.TimeZone;

import org.easymock.classextension.EasyMock;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.dns.DnsManager;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.test.TestHelper;

public final class AudioCodesGatewayDefaultsMock {

    public static final String SIPFOUNDRY_ORG = "sipfoundry.org";

    public static DeviceDefaults getDeviceDefaults() {
        DeviceDefaults defaults = new DeviceDefaults();

        TimeZone tz = TimeZone.getTimeZone("Etc/GMT+5");
        DeviceTimeZone dtz = new DeviceTimeZone();
        dtz.setTimeZone(tz);
        defaults.setTimeZoneManager(TestHelper.getTimeZoneManager(dtz));
        defaults.setDomainManager(TestHelper.getTestDomainManager(SIPFOUNDRY_ORG));

        DomainManager domainManager = getMockDomainManager();
        replay(domainManager);
        domainManager.getDomain().setSipRealm("realm." + SIPFOUNDRY_ORG);
        domainManager.getDomain().setName(SIPFOUNDRY_ORG);
        defaults.setDomainManager(domainManager);

        List<Address> addresses = Arrays.asList(new Address[] {
            new Address(DnsManager.DNS_ADDRESS)
        });
        AddressManager addressManager = EasyMock.createMock(AddressManager.class);
        addressManager.getAddresses(DnsManager.DNS_ADDRESS);
        expectLastCall().andReturn(addresses).anyTimes();
        addressManager.getSingleAddress((AddressType) anyObject());
        expectLastCall().andReturn(new Address(DnsManager.DNS_ADDRESS)).anyTimes();
        replay(addressManager);
        defaults.setAddressManager(addressManager);

        return defaults;
    }
}
