/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.ArrayList;
import java.util.Formatter;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.annotation.Required;

public class DnsGeneratorImpl implements DnsGenerator, DaoEventListener {
    private static final Log LOG = LogFactory.getLog(DnsGeneratorImpl.class);

    private volatile int m_serial = 1;
    private ApiProvider<ZoneAdminApi> m_zoneAdminApiProvider;

    private LocationsManager m_locationsManager;
    private SipxServiceManager m_sipxServiceManager;

    public void generate() {
        generateWithout(null);
    }

    /**
     * Prepares parameters and call XML/RPC method to generate DNS zone file.
     *
     * Extra locationToSkip parameter allows generating zone file without location that is being
     * deleted. We get delete event when location manager still has a list of all locations.
     *
     * @param locationToSkip usually location that is being deleted
     */
    public void generateWithout(Location locationToSkip) {
        Location primaryLocation = m_locationsManager.getPrimaryLocation();

        if (primaryLocation == null) {
            LOG.error("No primary location defined. OK in test environment");
            return;
        }

        Formatter cmd = new Formatter();
        SipxService proxyService = m_sipxServiceManager.getServiceByBeanId(SipxProxyService.BEAN_ID);
        List<Location> locations = new ArrayList<Location>();
        for (Location location : m_locationsManager.getLocations()) {
            if (!location.equals(locationToSkip)) {
                locations.add(location);
            }
        }

        // First list the servers that have a SIP Router role
        for (Location location : locations) {
            if (location.isServiceInstalled(proxyService)) {
                cmd.format("%s/%s ", location.getFqdn(), location.getAddress());
            }
        }

        // Next list the servers that do not have a SIP Router role
        for (Location location : locations) {
            if (!location.isServiceInstalled(proxyService)) {
                cmd.format("-o %s/%s ", location.getFqdn(), location.getAddress());
            }
        }

        // Next add the server that is running the Instant Messaging Role
        SipxService instantMessagingService = m_sipxServiceManager.getServiceByBeanId("sipxOpenfireService");
        for (Location location : locations) {
            if (location.isServiceInstalled(instantMessagingService)) {
                cmd.format("-x %s/%s ", location.getFqdn(), location.getAddress());
            }
        }

        String portTCP = ((SipxProxyService) proxyService).getSipTCPPort();
        String portUDP = ((SipxProxyService) proxyService).getSipUDPPort();

        cmd.format("--zone --serial %s --provide-dns --port-TCP %s --port-UDP %s", m_serial++, portTCP, portUDP);
        try {
            ZoneAdminApi api = m_zoneAdminApiProvider.getApi(primaryLocation.getProcessMonitorUrl());
            api.generateDns(primaryLocation.getFqdn(), cmd.toString());
        } catch (XmlRpcRemoteException e) {
            // do not re-throw - this is called during saving/deleting locations
            LOG.error("Cannot reconfigure DNS", e);
        }
    }

    public void onSave(Object entity) {
        if (entity instanceof Location) {
            generate();
        }
    }

    public void onDelete(Object entity) {
        if (entity instanceof Location) {
            // skip location that is being deleted
            generateWithout((Location) entity);
        }
    }

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    @Required
    public void setZoneAdminApiProvider(ApiProvider zoneAdminApiProvider) {
        m_zoneAdminApiProvider = zoneAdminApiProvider;
    }
}
