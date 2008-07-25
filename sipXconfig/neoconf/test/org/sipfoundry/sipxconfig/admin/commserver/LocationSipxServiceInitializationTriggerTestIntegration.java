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

import org.sipfoundry.sipxconfig.IntegrationTestCase;

public class LocationSipxServiceInitializationTriggerTestIntegration extends IntegrationTestCase {
    
    private LocationSipxServiceInitializationTrigger m_out;
    private LocationsManager m_locationsManager;
    
    public void testOnInitTask() throws Exception {
        loadDataSetXml("admin/commserver/seedLocations.xml");
        m_out.onInitTask("initialize-location-service-mapping");
        
        Location testLocation = m_locationsManager.getLocations()[0];
        assertFalse(testLocation.getSipxServices().isEmpty());
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
    
    public void setLocationSipxServiceInitializationTrigger(LocationSipxServiceInitializationTrigger trigger) {
        m_out = trigger;
    }
}
