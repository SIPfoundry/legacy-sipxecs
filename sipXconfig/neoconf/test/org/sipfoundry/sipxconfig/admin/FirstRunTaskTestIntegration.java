/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

import java.util.Collection;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanActivationManager;
import org.sipfoundry.sipxconfig.common.AlarmContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.service.LocationSpecificService;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.springframework.test.annotation.DirtiesContext;

public class FirstRunTaskTestIntegration extends IntegrationTestCase {
    private LocationsManager m_locationsManager;
    private FirstRunTask m_firstRun;

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setFirstRun(FirstRunTask firstRun) {
        m_firstRun = firstRun;
    }

    @DirtiesContext
    public void testEnableFirstRunServices() throws Exception {
        DomainManager domainManager = createNiceMock(DomainManager.class);
        m_firstRun.setDomainManager(domainManager);
        AdminContext adminContext = createNiceMock(AdminContext.class);
        m_firstRun.setAdminContext(adminContext);
        DialPlanActivationManager dpam = createNiceMock(DialPlanActivationManager.class);
        m_firstRun.setDialPlanActivationManager(dpam);
        CoreContext coreContext = createNiceMock(CoreContext.class);
        m_firstRun.setCoreContext(coreContext);
        AlarmContext alarmContext = createNiceMock(AlarmContext.class);
        m_firstRun.setAlarmContext(alarmContext);

        replay(domainManager, adminContext, dpam, coreContext, alarmContext);

        loadDataSetXml("admin/commserver/seedLocationsAndServices.xml");
        m_firstRun.setLocationsManager(m_locationsManager);

        ServiceConfigurator serviceConfigurator = createMock(ServiceConfigurator.class);
        Location[] locations = m_locationsManager.getLocations();
        for (Location location : locations) {
            serviceConfigurator.enforceRole(location);
            expectLastCall();
        }
        expectLastCall();

        replay(serviceConfigurator);

        m_firstRun.setServiceConfigurator(serviceConfigurator);
        m_firstRun.runTask();

        verify(serviceConfigurator);

        Location primaryLocation = m_locationsManager.getPrimaryLocation();
        Collection<LocationSpecificService> servicesForPrimaryLocation = primaryLocation.getServices();
        assertFalse(servicesForPrimaryLocation.isEmpty());
        //auto-enabled bundles are set for primary location
        assertEquals(3, primaryLocation.getInstalledBundles().size());
    }
}
