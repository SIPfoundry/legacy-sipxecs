/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.mwi;

import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface Mwi {
    public static final LocationFeature FEATURE = new LocationFeature("mwi");
    public static final AddressType SIP_UDP = AddressType.sip("mwiSipUdp");
    public static final AddressType SIP_TCP = AddressType.sip("mwiSipTcp");
    public static final AddressType HTTP_API = new AddressType("mwiHttpApi", "https://%s:%d/cgi/StatusEvent.cgi");

    public MwiSettings getSettings();

    public void saveSettings(MwiSettings settings);
}
