/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.ivr;

import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface Ivr {
    public static final LocationFeature FEATURE = new LocationFeature("ivr");
    public static final GlobalFeature CALLPILOT = new GlobalFeature("callpilot");
    public static final AddressType REST_API = new AddressType("ivrRestApi");

    public IvrSettings getSettings();

    public CallPilotSettings getCallPilotSettings();

    public void saveSettings(IvrSettings settings);

    public void saveCallPilotSettings(CallPilotSettings settings);
}
