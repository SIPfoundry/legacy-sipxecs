/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.ArrayList;
import java.util.Collection;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.InitTaskListener;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

public class LocationSipxServiceInitializationTrigger extends InitTaskListener {
    private static final Log LOG = LogFactory
            .getLog(LocationSipxServiceInitializationTrigger.class);
    private LocationsManager m_locationsManager;
    private SipxServiceManager m_sipxServiceManager;

    @Override
    public void onInitTask(String task) {
        if (m_locationsManager.getLocations().length < 1) {
            LOG.warn("No available locations to initialize services for");
        }
        
        Location defaultLocation = m_locationsManager.getLocations()[0];
        LOG.info("Initializing SipX Services for default location '" + defaultLocation + "'");
        Collection<SipxService> allServices = new ArrayList<SipxService>(m_sipxServiceManager.getAllServices());
        defaultLocation.setSipxServices(allServices);
        m_locationsManager.storeLocation(defaultLocation);
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
    
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }
}
