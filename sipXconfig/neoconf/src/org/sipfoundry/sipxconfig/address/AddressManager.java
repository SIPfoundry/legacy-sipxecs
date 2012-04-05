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
package org.sipfoundry.sipxconfig.address;

import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureManager;

public interface AddressManager {

    public final class Util {
        private Util() {
        }
        public static Map<String, AddressType> typesById(Collection<AddressType> l) {
            Map<String, AddressType> ndx = new HashMap<String, AddressType>();
            for (AddressType i : l) {
                ndx.put(i.getId(), i);
            }
            return ndx;
        }
    }

//    public List<AddressType> getAddressTypes();

    public Address getSingleAddress(AddressType type);

    public Address getSingleAddress(AddressType type, Location requester);

    public Address getSingleAddress(AddressType type, AddressType backupType);

    public Address getSingleAddress(AddressType type, AddressType backupType, Location requester);

    public List<Address> getAddresses(AddressType type);

    public List<Address> getAddresses(AddressType type, Location requester);

    public FeatureManager getFeatureManager();
}
