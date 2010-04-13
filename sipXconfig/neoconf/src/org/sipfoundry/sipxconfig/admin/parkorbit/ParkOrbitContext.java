/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.parkorbit;

import java.util.Collection;

import org.sipfoundry.sipxconfig.admin.commserver.AliasProvider;
import org.sipfoundry.sipxconfig.alias.AliasOwner;

public interface ParkOrbitContext extends AliasOwner, AliasProvider {
    public static final String CONTEXT_BEAN_NAME = "parkOrbitContext";

    ParkOrbit loadParkOrbit(Integer id);

    ParkOrbit newParkOrbit();

    void storeParkOrbit(ParkOrbit parkOrbit);

    void removeParkOrbits(Collection ids);

    Collection<ParkOrbit> getParkOrbits();

    void activateParkOrbits();

    String getDefaultMusicOnHold();

    void setDefaultMusicOnHold(String newMusic);

    void clear();
}
