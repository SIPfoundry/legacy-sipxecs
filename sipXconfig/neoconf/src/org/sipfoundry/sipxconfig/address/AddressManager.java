/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.address;

import java.util.List;

import org.sipfoundry.sipxconfig.feature.FeatureManager;

public interface AddressManager {
    public Address getSingleAddress(AddressType type);

    public Address getSingleAddress(AddressType type, Object requester);

    public List<Address> getAddresses(AddressType type);

    public List<Address> getAddresses(AddressType type, Object requester);

    public FeatureManager getFeatureManager();
}
