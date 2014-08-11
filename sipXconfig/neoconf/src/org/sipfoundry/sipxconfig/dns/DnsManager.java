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
    public static final String DEFAULT_VIEW_NAME = "default";

    public DnsSettings getSettings();

    public void saveSettings(DnsSettings settings);

    public Address getSingleAddress(AddressType t, Collection<Address> addresses, Location whoIsAsking);

    public Collection<DnsSrvRecord> getResourceRecords(DnsView view);

    public AddressManager getAddressManager();

    public Collection<DnsFailoverPlan> getPlans();

    public DnsFailoverPlan getPlan(Integer planId);

    public void savePlan(DnsFailoverPlan plan);

    public void deletePlan(DnsFailoverPlan plan);

    public Collection<DnsView> getViews();

    public DnsView getViewById(Integer viewId);

    public void saveView(DnsView view);

    public void deleteView(DnsView view);

    public void moveViewById(Integer[] viewIds, int step);

    public String[] getViewNamesUsingRegion(Region region);

    public String[] getPlanNamesUsingRegion(Region region);

    public String[] getPlanNamesUsingLocation(Location location);

    public String[] getViewNamesUsingPlan(DnsFailoverPlan plan);

    public Collection<DnsCustomRecords> getCustomRecords();

    public DnsCustomRecords getCustomRecordsById(Integer customId);

    public void saveCustomRecords(DnsCustomRecords custom);

    public void deleteCustomRecords(DnsCustomRecords custom);

    public Collection<DnsCustomRecords> getCustomRecordsByIds(Collection<Integer> customIds);

    Collection<ResourceRecords> getResourceRecords();

    DnsView getDefaultView();
}
