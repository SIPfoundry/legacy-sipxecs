/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.api.model;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.sipfoundry.sipxconfig.parkorbit.ParkOrbit;

@XmlRootElement(name = "Orbits")
public class CallParkList {

    private List<CallParkBean> m_orbits;

    public void setOrbits(List<CallParkBean> orbits) {
        m_orbits = orbits;
    }

    @XmlElement(name = "Orbit")
    public List<CallParkBean> getOrbits() {
        if (m_orbits == null) {
            m_orbits = new ArrayList<CallParkBean>();
        }
        return m_orbits;
    }

    public static CallParkList convertOrbitList(Collection<ParkOrbit> parkOrbits) {
        List<CallParkBean> orbits = new ArrayList<CallParkBean>();
        for (ParkOrbit orbit : parkOrbits) {
            orbits.add(CallParkBean.convertOrbit(orbit));
        }
        CallParkList list = new CallParkList();
        list.setOrbits(orbits);
        return list;
    }
}
