/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.dns;

import java.util.Collection;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.commserver.Location;

public interface DnsProvider {

    public Address getAddress(DnsManager manager, AddressType t, Collection<Address> addresses, Location whoIsAsking);

    public ResourceRecords getResourceRecords(DnsManager manager, Location whoIsAsking);
}
