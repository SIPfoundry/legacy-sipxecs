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

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;

public class LocationSpecificServiceInitTriggerTestIntegration extends IntegrationTestCase {

    private LocationSpecificServiceInitTrigger m_out;
    private LocationsManager m_locationsManager;

    public void testOnInitTaskWithoutPreviousServiceMigration() throws Exception {
        loadDataSetXml("service/seedLocationsAndServicesWithoutServiceMigration.xml");
        m_out.onInitTask("init-location-specific-service");

        verifyCorrectServiceStateOnPrimaryServer();
    }

    public void testOnInitTaskWithPreviousServiceMigration() throws Exception {
        loadDataSetXml("service/seedLocationsAndServicesWithServiceMigration.xml");
        m_out.onInitTask("init-location-specic-service");

        verifyCorrectServiceStateOnPrimaryServer();

        Location distributedLocation = m_locationsManager.getLocation(102);
        assertNotNull(distributedLocation.getServices());
        assertEquals(1, distributedLocation.getServices().size());
        for (LocationSpecificService service : distributedLocation.getServices()) {
            assertFalse(service.getEnableOnNextUpgrade());
        }
    }

    private void verifyCorrectServiceStateOnPrimaryServer() {
        Location primaryLocation = m_locationsManager.getPrimaryLocation();
        assertNotNull(primaryLocation.getServices());
        assertEquals(3, primaryLocation.getServices().size());
        for (LocationSpecificService service : primaryLocation.getServices()) {
            if (service.getSipxService().getBeanId().equals(SipxConfigAgentService.BEAN_ID)) {
                assertFalse(service.getEnableOnNextUpgrade());
            } else {
                assertTrue(service.getEnableOnNextUpgrade());
            }
        }
    }

    public void setLocationSpecificServiceInitTrigger(LocationSpecificServiceInitTrigger trigger) {
        m_out = trigger;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
}
