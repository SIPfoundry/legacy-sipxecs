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

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;

public class SipxCallResolverConfiguration extends SipxServiceConfiguration {

    private LocationsManager m_locationsManager;

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        SipxService service = getService(SipxCallResolverService.BEAN_ID);
        context.put("settings", service.getSettings().getSetting("callresolver"));
        context.put("callresolverService", service);
        context.put("callresolverHosts", getDistributedLocations(location));
        return context;
    }

    private String getDistributedLocations(Location currentlocation) {
        StringBuffer callresolverHosts = new StringBuffer();
        Location[] allLocations = m_locationsManager.getLocations();
        for (Location location : allLocations) {
            if (location.isPrimary()) {
                continue;
            } else {
                //Add the distributed system fqdn to the string.
                if (callresolverHosts.length() == 0) {
                    // string is empty.  Add the localhost first.
                    callresolverHosts.append("localhost");
                }

                callresolverHosts.append(", ");
                callresolverHosts.append(location.getFqdn() + ":5433");
            }
        }

        return callresolverHosts.toString();

    }

    @Override
    public boolean isReplicable(Location location) {
        return getSipxServiceManager().isServiceInstalled(location.getId(), SipxCallResolverService.BEAN_ID);
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
}
