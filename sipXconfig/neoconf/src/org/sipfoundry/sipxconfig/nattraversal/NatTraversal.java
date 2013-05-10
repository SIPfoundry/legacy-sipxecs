/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.nattraversal;

import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;

public interface NatTraversal {
    public static final GlobalFeature FEATURE = new GlobalFeature("natTraversal");

    public static final AddressType RELAY_RTP = new AddressType("natRtp", "rtp:%s:%d", 30000,
            AddressType.Protocol.udp);

    public static final AddressType RELAY_RPC = new AddressType("natRpc", 9090);

    public NatSettings getSettings();

    public void saveSettings(NatSettings settings);
}
