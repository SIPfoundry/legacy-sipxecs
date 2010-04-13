/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.api;

import java.rmi.RemoteException;
import java.util.Collection;

import org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbitContext;

public class ParkOrbitServiceImpl implements ParkOrbitService {

    private ParkOrbitContext m_parkOrbitContext;
    private SimpleBeanBuilder m_parkOrbitBuilder;

    public void setParkOrbitContext(ParkOrbitContext parkOrbitContext) {
        m_parkOrbitContext = parkOrbitContext;
    }

    public void setParkOrbitBuilder(SimpleBeanBuilder builder) {
        m_parkOrbitBuilder = builder;
    }

    public GetParkOrbitsResponse getParkOrbits() throws RemoteException {
        Collection<org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbit> orbitsColl = m_parkOrbitContext
                .getParkOrbits();
        org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbit[] orbits = orbitsColl
                .toArray(new org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbit[orbitsColl
                        .size()]);
        ParkOrbit[] arrayOfParkOrbits = (ParkOrbit[]) ApiBeanUtil.toApiArray(m_parkOrbitBuilder,
                orbits, ParkOrbit.class);
        GetParkOrbitsResponse response = new GetParkOrbitsResponse();
        response.setParkOrbits(arrayOfParkOrbits);
        return response;
    }

    public void addParkOrbit(AddParkOrbit apo) throws RemoteException {
        org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbit myOrbit = m_parkOrbitContext
                .newParkOrbit();
        ParkOrbit apiOrbit = apo.getParkOrbit();
        ApiBeanUtil.toMyObject(m_parkOrbitBuilder, myOrbit, apiOrbit);
        m_parkOrbitContext.storeParkOrbit(myOrbit);
        m_parkOrbitContext.activateParkOrbits();
    }

}
