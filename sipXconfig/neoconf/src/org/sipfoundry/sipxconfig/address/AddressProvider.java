/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.address;

import java.util.Collection;

public interface AddressProvider {

    public Collection<AddressType> getSupportedAddressTypes(AddressManager manager);

    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type,
            AddressRequester requester);
}
