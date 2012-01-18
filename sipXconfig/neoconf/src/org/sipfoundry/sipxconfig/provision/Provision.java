/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.provision;


import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface Provision {
    public static final LocationFeature FEATURE = new LocationFeature("provision");
    public static final AddressType PROVISION_SERVICE = new AddressType("provisionService");

    public ProvisionSettings getSettings();

    public void saveSettings(ProvisionSettings settings);
}
