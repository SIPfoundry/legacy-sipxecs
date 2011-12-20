/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.address;

import java.util.Collection;

import org.sipfoundry.sipxconfig.feature.FeatureManager;

public interface AddressManager {
    public static final AddressType NTP_ADDRESS = new AddressType("ntp");

    public Address getSingleAddress(AddressType type);

    public Address getSingleAddress(AddressType type, Object requester);

    public Collection<Address> getAddresses(AddressType type);

    public Collection<Address> getAddresses(AddressType type, Object requester);

    public FeatureManager getFeatureManager();
}
