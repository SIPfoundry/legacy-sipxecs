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

public class SipxRegistrarConfiguration extends SipxServiceConfiguration {
    public static final String OPENFIRE_HOST = "openfire-host";
    public static final String OPENFIRE_XML_RPCPORT = "openfire-xml-rpc-port";
    private LocationsManager m_locationsManager;

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        SipxService registrarService = getService(SipxRegistrarService.BEAN_ID);
        context.put("settings", registrarService.getSettings());
        context.put("registrarService", registrarService);
        context.put("proxyService", getService(SipxProxyService.BEAN_ID));
        context.put("parkService", getService(SipxParkService.BEAN_ID));
        context.put("registrarSyncHosts", getRegistrarSyncHosts(location));
        context.put("sipxServiceManager", getSipxServiceManager());
        context.put(OPENFIRE_HOST, getSipxServiceManager().getServiceParam(OPENFIRE_HOST));
        context.put(OPENFIRE_XML_RPCPORT, getSipxServiceManager().getServiceParam(OPENFIRE_XML_RPCPORT));
        return context;
    }

    private String getRegistrarSyncHosts(Location currentLocation) {
        StringBuffer registrarSyncHosts = new StringBuffer();
        Location[] allLocations = m_locationsManager.getLocations();
        for (Location location : allLocations) {
            if (location.getFqdn().equals(currentLocation.getFqdn())) {
                continue;
            }

            if (location.getService(SipxRegistrarService.BEAN_ID) != null) {
                registrarSyncHosts.append(location.getFqdn());
                registrarSyncHosts.append(' ');
            }
        }

        return registrarSyncHosts.toString();
    }

    @Override
    public boolean isReplicable(Location location) {
        return getSipxServiceManager().isServiceInstalled(location.getId(), SipxRegistrarService.BEAN_ID);
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

}
