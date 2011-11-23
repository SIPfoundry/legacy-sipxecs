/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.address;

import java.util.Collection;

import org.sipfoundry.sipxconfig.admin.commserver.Location;

public interface AddressProvider {

    public Collection<AddressType> getSupportedAddressTypes();

    public Collection<Address> getAvailableAddresses(Location location, AddressType type);
}
