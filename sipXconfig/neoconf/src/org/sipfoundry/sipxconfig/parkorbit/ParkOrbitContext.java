/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.parkorbit;

import java.util.Collection;

import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface ParkOrbitContext extends AliasOwner {
    public static final String PARK_ORBIT_GROUP_ID = "park_orbit";
    public static final String CONTEXT_BEAN_NAME = "parkOrbitContext";
    public static LocationFeature FEATURE = new LocationFeature("park");
    public static AddressType SIP_TCP_PORT = AddressType.sipTcp("parkTcp");
    public static AddressType SIP_UDP_PORT = AddressType.sipUdp("parkUdp");
    public static AddressType SIP_RTP_PORT = new AddressType("parkRtp");

    ParkSettings getSettings();

    void saveSettings(ParkSettings settings);

    ParkOrbit loadParkOrbit(Integer id);

    ParkOrbit newParkOrbit();

    void storeParkOrbit(ParkOrbit parkOrbit);

    void removeParkOrbits(Collection ids);

    Collection<ParkOrbit> getParkOrbits();

    String getDefaultMusicOnHold();

    void setDefaultMusicOnHold(String newMusic);

    void clear();

    ParkOrbit loadParkOrbitByName(String name);
}
