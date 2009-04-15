/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.springframework.beans.factory.annotation.Required;

public class SipxCallResolverConfiguration extends SipxServiceConfiguration {

    private LocationsManager m_locationsManager;

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        SipxService service = getService(SipxCallResolverService.BEAN_ID);
        context.put("settings", service.getSettings().getSetting("callresolver"));
        context.put("callresolverService", service);
        int port = (Integer) service.getSettingTypedValue("callresolver/DATABASE_PORT");
        context.put("callresolverHosts", getDistributedLocations(port));
        return context;
    }

    private String getDistributedLocations(int port) {
        SipxService callResolverAgent = getService(SipxCallResolverAgentService.BEAN_ID);
        List<Location> agentLocations = m_locationsManager.getLocationsForService(callResolverAgent);
        if (agentLocations.isEmpty()) {
            return null;
        }
        List<String> callresolverHosts = new ArrayList<String>(agentLocations.size() + 1);
        // represents primary location...
        callresolverHosts.add("localhost");
        for (Location location : agentLocations) {
            callresolverHosts.add(String.format("%s:%d", location.getFqdn(), port));
        }
        return StringUtils.join(callresolverHosts, ", ");
    }

    @Override
    public boolean isReplicable(Location location) {
        return getSipxServiceManager().isServiceInstalled(location.getId(), SipxCallResolverService.BEAN_ID);
    }

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
}
