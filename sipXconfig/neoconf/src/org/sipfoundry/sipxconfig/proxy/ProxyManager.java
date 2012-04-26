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
package org.sipfoundry.sipxconfig.proxy;

import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.alarm.AlarmDefinition;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface ProxyManager {
    public static final LocationFeature FEATURE = new LocationFeature("proxy");
    public static final AddressType TCP_ADDRESS = new AddressType("proxyTcp");
    public static final AddressType UDP_ADDRESS = new AddressType("proxyUdp");
    public static final AddressType TLS_ADDRESS = new AddressType("proxyTls");
    public static final AlarmDefinition EMERG_NUMBER_DIALED = new AlarmDefinition("EMERG_NUMBER_DIALED");

    public ProxySettings getSettings();

    public void saveSettings(ProxySettings settings);
}
