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
package org.sipfoundry.sipxconfig.dns;


import java.util.Collection;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.region.Region;

public interface DnsManager {
    public static final LocationFeature FEATURE = new LocationFeature("sipxdns");
    public static final AddressType DNS_ADDRESS = new AddressType("dnsAddress", 53, AddressType.Protocol.udp);

    public DnsSettings getSettings();

    public void saveSettings(DnsSettings settings);

    public Address getSingleAddress(AddressType t, Collection<Address> addresses, Location whoIsAsking);

    /*
     * @parameter region - null acceptable for primary region or if regions are not defined
     */
    public Collection<DnsSrvRecord> getResourceRecords(Region region);

    public AddressManager getAddressManager();
}
