/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.imbot;

import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface ImBot {
    public static final LocationFeature FEATURE = new LocationFeature("imbot");
    public static final AddressType XML_RPC = new AddressType("imbotXmlRpc");

    public ImBotSettings getSettings();

    public void saveSettings(ImBotSettings settings);
}
