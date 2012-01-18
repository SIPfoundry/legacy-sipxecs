/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.registrar;

import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface Registrar {
    public static final LocationFeature FEATURE = new LocationFeature("registrar");
    public static final AddressType TCP_ADDRESS = new AddressType("registrar-tcp");
    public static final AddressType EVENT_ADDRESS = new AddressType("registrar-event");
    public static final AddressType UDP_ADDRESS = new AddressType("registrar-udp");
    public static final AddressType XMLRPC_ADDRESS = new AddressType("registrar-xmlrpc");
    public static final AddressType PRESENCE_MONITOR_ADDRESS = new AddressType("registrar-presence");

    public RegistrarSettings getSettings();

    public void saveSettings(RegistrarSettings settings);
}
