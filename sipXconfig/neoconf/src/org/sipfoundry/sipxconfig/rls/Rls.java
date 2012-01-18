/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.rls;

import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface Rls {
    public static final LocationFeature FEATURE = new LocationFeature("rls");
    public static final AddressType UDP_SIP = new AddressType("rlsUdp");
    public static final AddressType TCP_SIP = new AddressType("rlsTcp");

    public RlsSettings getSettings();

    public void saveSettings(RlsSettings settings);
}
