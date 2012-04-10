/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.presence;

import org.sipfoundry.sipxconfig.acd.AcdServer;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

/**
 * ACD Presence system. Determining when an agent is available.
 */
public interface PresenceServer {
    public static final LocationFeature FEATURE = new LocationFeature("acdPresence");
    public static final AddressType HTTP_ADDRESS = new AddressType("acdPresenceApi");
    public static final AddressType SIP_TCP_ADDRESS = new AddressType("acdPresenceTcp");
    public static final AddressType SIP_UDP_ADDRESS = new AddressType("acdPresenceUdp");
    public static final String OBJECT_CLASS_KEY = "object-class";

    public PresenceSettings getSettings();

    public void saveSettings(PresenceSettings settings);

    public void signIn(User user, AcdServer acdServer);

    public void signOut(User user, AcdServer acdServer);

    public PresenceStatus getStatus(User user, AcdServer acdServer);
}
