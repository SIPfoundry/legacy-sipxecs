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

import java.util.List;

import javax.ws.rs.core.Response;
import javax.ws.rs.core.Response.Status;

import org.apache.commons.lang.time.DateUtils;
import org.sipfoundry.sipxconfig.api.RegistrationApi;
import org.sipfoundry.sipxconfig.api.model.RegistrationBean.RegistrationList;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.commserver.imdb.RegistrationItem;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.registrar.RegistrationContext;
import org.sipfoundry.sipxconfig.registrar.RegistrationMetrics;

public class RegistrationApiImpl implements RegistrationApi {
    private RegistrationContext m_context;
    private CoreContext m_coreContext;
    private PhoneContext m_phoneContext;
    private LocationsManager m_locationsManager;

    @Override
    public Response getRegistrations(Integer startId, Integer limit) {
        if (startId != null && limit != null) {
            return buildRegistrationResponse(m_context.getRegistrations(startId, limit));
        }
        return buildRegistrationResponse(m_context.getRegistrations());
    }

    @Override
    public Response getRegistrationsByUser(String userId, Integer startId, Integer limit) {
        User user = getUserByIdOrUserName(userId);
        if (user != null) {
            if (startId != null && limit != null) {
                return buildRegistrationResponse(m_context.getRegistrationsByUser(user, startId, limit));
            }
            return buildRegistrationResponse(m_context.getRegistrationsByUser(user));
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response dropRegistrationsByUser(String userId) {
        User user = getUserByIdOrUserName(userId);
        if (user != null) {
            m_context.dropRegistrationsByUser(user);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getRegistrationsByMac(String serialId) {
        Phone phone = m_phoneContext.getPhoneBySerialNumber(serialId);
        if (phone != null) {
            return buildRegistrationResponse(m_context.getRegistrationsByMac(serialId));
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response dropRegistrationsByMac(String serialId) {
        Phone phone = m_phoneContext.getPhoneBySerialNumber(serialId);
        if (phone != null) {
            m_context.dropRegistrationsByMac(serialId);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getRegistrationsByIp(String ip) {
        return buildRegistrationResponse(m_context.getRegistrationsByIp(ip));
    }

    @Override
    public Response dropRegistrationsByIp(String ip) {
        m_context.dropRegistrationsByIp(ip);
        return Response.ok().build();
    }

    @Override
    public Response getRegistrationsByCallId(String callid) {
        return buildRegistrationResponse(m_context.getRegistrationsByCallId(callid));
    }

    @Override
    public Response dropRegistrationsByCallId(String callid) {
        m_context.dropRegistrationsByCallId(callid);
        return Response.ok().build();
    }

    @Override
    public Response getRegistrationsByServer(String serverId) {
        Location location = getLocationByIdOrFqdn(serverId);
        if (location != null) {
            return buildRegistrationResponse(m_context.getRegistrationsByServer(location.getAddress()));
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response dropRegistrationsByServer(String serverId) {
        Location location = getLocationByIdOrFqdn(serverId);
        if (location != null) {
            m_context.dropRegistrationsByServer(location.getAddress());
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    private Response buildRegistrationResponse(List<RegistrationItem> items) {
        long now = System.currentTimeMillis() / DateUtils.MILLIS_PER_SECOND;
        RegistrationMetrics metrics = buildRegistrationMetrics(items, now);
        if (metrics != null) {
            return Response.ok()
                    .entity(RegistrationList.convertRegistrationList(metrics.getUniqueRegistrations(), now)).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    private RegistrationMetrics buildRegistrationMetrics(List<RegistrationItem> registrations, long now) {
        RegistrationMetrics metrics = new RegistrationMetrics();
        metrics.setStartTime(now);
        metrics.setRegistrations(registrations);
        return metrics;
    }

    private User getUserByIdOrUserName(String id) {
        User user = null;
        try {
            int userId = Integer.parseInt(id);
            user = m_coreContext.getUser(userId);
        } catch (NumberFormatException e) {
            user = null;
        }
        if (user == null) {
            user = m_coreContext.loadUserByUserNameOrAlias(id);
        }
        return user;
    }

    private Location getLocationByIdOrFqdn(String id) {
        Location location = null;
        try {
            int locationId = Integer.parseInt(id);
            location = m_locationsManager.getLocation(locationId);
        } catch (NumberFormatException e) {
            location = m_locationsManager.getLocationByFqdn(id);
        }
        return location;
    }

    public void setRegistrationContext(RegistrationContext context) {
        m_context = context;
    }

    public void setCoreContext(CoreContext context) {
        m_coreContext = context;
    }

    public void setPhoneContext(PhoneContext context) {
        m_phoneContext = context;
    }

    public void setLocationsManager(LocationsManager manager) {
        m_locationsManager = manager;
    }

}
