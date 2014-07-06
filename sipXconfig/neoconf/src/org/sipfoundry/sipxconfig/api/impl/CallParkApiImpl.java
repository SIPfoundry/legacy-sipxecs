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
package org.sipfoundry.sipxconfig.api.impl;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;

import javax.servlet.http.HttpServletRequest;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.Response.Status;

import org.sipfoundry.sipxconfig.api.CallParkApi;
import org.sipfoundry.sipxconfig.api.model.CallParkBean;
import org.sipfoundry.sipxconfig.api.model.CallParkList;
import org.sipfoundry.sipxconfig.api.model.SettingsList;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbit;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbitContext;
import org.sipfoundry.sipxconfig.setting.Setting;

public class CallParkApiImpl extends PromptsApiImpl implements CallParkApi {
    private ParkOrbitContext m_context;
    private LocationsManager m_locationsManager;

    @Override
    public Response getOrbits() {
        Collection<ParkOrbit> parkOrbits = m_context.getParkOrbits();
        if (parkOrbits != null) {
            return Response.ok().entity(CallParkList.convertOrbitList(parkOrbits)).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response newOrbit(CallParkBean bean) {
        return newOrbit(bean.getServer(), bean);
    }

    @Override
    public Response getOrbitsByServer(String serverId) {
        Location location = getLocationByIdOrFqdn(serverId);
        if (location != null) {
            Collection<ParkOrbit> parkOrbits = m_context.getParkOrbits(location.getId());
            if (parkOrbits != null) {
                return Response.ok().entity(CallParkList.convertOrbitList(parkOrbits)).build();
            }
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response deleteOrbitsByServer(String serverId) {
        Location location = getLocationByIdOrFqdn(serverId);
        if (location != null) {
            Collection<ParkOrbit> parkOrbits = m_context.getParkOrbits(location.getId());
            Collection<Integer> ids = new ArrayList<Integer>();
            for (ParkOrbit orbit : parkOrbits) {
                ids.add(orbit.getId());
            }
            m_context.removeParkOrbits(ids);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response newOrbit(String serverId, CallParkBean bean) {
        Location location = getLocationByIdOrFqdn(serverId);
        if (location != null) {
            ParkOrbit orbit = m_context.newParkOrbit();
            Response response = checkPrompt(bean);
            if (response != null) {
                return response;
            }
            CallParkBean.populateOrbit(bean, orbit, location);
            m_context.storeParkOrbit(orbit);
            return Response.ok().entity(orbit.getId()).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getOrbit(Integer orbitId) {
        ParkOrbit orbit = m_context.loadParkOrbit(orbitId);
        if (orbit != null) {
            return Response.ok().entity(CallParkBean.convertOrbit(orbit)).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response deleteOrbit(Integer orbitId) {
        ParkOrbit orbit = m_context.loadParkOrbit(orbitId);
        if (orbit != null) {
            m_context.removeParkOrbits(Collections.singleton(orbit.getId()));
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response updateOrbit(Integer orbitId, CallParkBean bean) {
        ParkOrbit orbit = m_context.loadParkOrbit(orbitId);
        if (orbit != null) {
            Response response = checkPrompt(bean);
            if (response != null) {
                return response;
            }
            Location location = getLocationByIdOrFqdn(bean.getServer());
            if (location != null) {
                CallParkBean.populateOrbit(bean, orbit, location);
                m_context.storeParkOrbit(orbit);
                return Response.ok().build();
            } else {
                return Response.status(Status.NOT_FOUND).entity("No such server").build();
            }
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getOrbitSettings(Integer orbitId, HttpServletRequest request) {
        ParkOrbit orbit = m_context.loadParkOrbit(orbitId);
        if (orbit != null) {
            Setting settings = orbit.getSettings();
            return Response.ok().entity(SettingsList.convertSettingsList(settings, request.getLocale())).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getOrbitSetting(Integer orbitId, String path, HttpServletRequest request) {
        ParkOrbit orbit = m_context.loadParkOrbit(orbitId);
        if (orbit != null) {
            return ResponseUtils.buildSettingResponse(orbit, path, request.getLocale());
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response setOrbitSetting(Integer orbitId, String path, String value) {
        ParkOrbit orbit = m_context.loadParkOrbit(orbitId);
        if (orbit != null) {
            orbit.setSettingValue(path, value);
            m_context.storeParkOrbit(orbit);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response deleteOrbitSetting(Integer orbitId, String path) {
        ParkOrbit orbit = m_context.loadParkOrbit(orbitId);
        if (orbit != null) {
            Setting setting = orbit.getSettings().getSetting(path);
            setting.setValue(setting.getDefaultValue());
            m_context.storeParkOrbit(orbit);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    private Response checkPrompt(CallParkBean bean) {
        if (bean != null) {
            return checkPrompt(bean.getMusic());
        }
        return null;
    }

    private Location getLocationByIdOrFqdn(String id) {
        Location location = null;
        try {
            int locationId = Integer.parseInt(id);
            location = m_locationsManager.getLocation(locationId);
        } catch (NumberFormatException e) {
            // no id then it must be MAC
            location = m_locationsManager.getLocationByFqdn(id);
        }
        return location;
    }

    public void setParkOrbitContext(ParkOrbitContext context) {
        m_context = context;
    }

    public void setLocationsManager(LocationsManager manager) {
        m_locationsManager = manager;
    }

}
