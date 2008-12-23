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

import java.util.Collection;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.InitTaskListener;

public class LocationSpecificServiceInitTrigger extends InitTaskListener {

    private static final Log LOG = LogFactory.getLog(LocationSpecificServiceInitTrigger.class);

    private LocationsManager m_locationsManager;
    private SipxServiceManager m_sipxServiceManager;

    @Override
    public void onInitTask(String task) {
        LOG.info("Initializing location-specific service info");
        Location primaryServer = m_locationsManager.getPrimaryLocation();

        // added as a check for a condition that should only occur in UI unit tests
        if (primaryServer == null) {
            return;
        }

        if (primaryServer.getServices() != null
                && primaryServer.getServices().size() > 0) {
            // Config agent is not needed on primary server, but was added due to database
            // upgrade patch
            primaryServer.removeServiceByBeanId(SipxConfigAgentService.BEAN_ID);
            for (LocationSpecificService service : primaryServer.getServices()) {
                service.setEnableOnNextUpgrade(true);
            }
        } else {
            Collection<SipxService> allServiceDefinitions = m_sipxServiceManager.getServiceDefinitions();
            for (SipxService sipxService : allServiceDefinitions) {
                if (!sipxService.getBeanId().equals(SipxConfigAgentService.BEAN_ID)) {
                    LocationSpecificService newService = new LocationSpecificService();
                    newService.setSipxService(sipxService);
                    newService.setEnableOnNextUpgrade(true);
                    primaryServer.addService(newService);
                }
            }
        }
        m_locationsManager.storeLocation(primaryServer);
    }

    public void setLocationManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }
}
