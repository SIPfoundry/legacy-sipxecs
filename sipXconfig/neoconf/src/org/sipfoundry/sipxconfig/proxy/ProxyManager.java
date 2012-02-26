/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.proxy;

import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.alarm.AlarmDefinition;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface ProxyManager {
    public static final LocationFeature FEATURE = new LocationFeature("proxy");
    public static final AddressType TCP_ADDRESS = new AddressType("proxyTcp");
    public static final AddressType UDP_ADDRESS = new AddressType("procyUdp");
    public static final AddressType TLS_ADDRESS = new AddressType("proxyTls");
    public static final AlarmDefinition EMERG_NUMBER_DIALED = new AlarmDefinition("EMERG_NUMBER_DIALED");

    public ProxySettings getSettings();

    public void saveSettings(ProxySettings settings);
}
