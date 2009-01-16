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

import java.util.Collection;
import java.util.Set;

import org.apache.commons.codec.binary.Base64;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanActivatedEvent;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.common.AlarmContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.service.LocationSpecificService;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.isA;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

// FIXME: it only test domain manager initialization for now
public class FirstRunTaskTestIntegration extends IntegrationTestCase {
    private DomainManager m_domainManager;
    private LocationsManager m_locationsManager;
    private FirstRunTask m_firstRun;

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setFirstRun(FirstRunTask firstRun) {
        m_firstRun = firstRun;
    }

    public void testOnInitTaskInitializeDomain() throws Exception {
        loadDataSetXml("domain/NoDomainSeed.xml");
        // InitializationTask initTask = new InitializationTask("first-run");
        // m_firstRunTask.onApplicationEvent(initTask);
        m_domainManager.initialize();

        assertEquals("example.org", m_domainManager.getDomain().getName());
        Set<String> aliases = m_domainManager.getDomain().getAliases();
        assertEquals(1, aliases.size());
        assertTrue(aliases.contains("alias.example.org"));
    }

    public void testOnInitTaskInitializeDomainSecret() throws Exception {
        loadDataSet("domain/missing-domain-secret.db.xml");
        // InitializationTask initTask = new InitializationTask("first-run");
        // m_firstRunTask.onApplicationEvent(initTask);
        m_domainManager.initialize();

        Domain domain = m_domainManager.getDomain();
        assertEquals("example.org", domain.getName());
        assertEquals(new Integer(2000), domain.getId());

        String sharedSecret = domain.getSharedSecret();
        byte[] secretBytes = new Base64().decode(sharedSecret.getBytes());
        assertEquals(18, secretBytes.length);

        // test that on a subsequent call, after domain is originally saved, we
        // we don't regenerate the secret
        // m_firstRunTask.onApplicationEvent(initTask);
        m_domainManager.initialize();
        assertEquals(sharedSecret, m_domainManager.getDomain().getSharedSecret());
    }

    public void testEnableFirstRunServices() throws Exception {
        DomainManager domainManager = createNiceMock(DomainManager.class);
        m_firstRun.setDomainManager(domainManager);
        AdminContext adminContext = createNiceMock(AdminContext.class);
        m_firstRun.setAdminContext(adminContext);
        DialPlanContext dialPlanContext = createNiceMock(DialPlanContext.class);
        m_firstRun.setDialPlanContext(dialPlanContext);
        CoreContext coreContext = createNiceMock(CoreContext.class);
        m_firstRun.setCoreContext(coreContext);
        AlarmContext alarmContext = createNiceMock(AlarmContext.class);
        m_firstRun.setAlarmContext(alarmContext);

        replay(domainManager, adminContext, dialPlanContext, coreContext, alarmContext);

        loadDataSetXml("admin/commserver/seedLocationsAndServices.xml");
        m_firstRun.setLocationsManager(m_locationsManager);

        SipxProcessContext processContext = createMock(SipxProcessContext.class);
        Location[] locations = m_locationsManager.getLocations();
        for (Location location : locations) {
            processContext.enforceRole(location);
            expectLastCall();
        }
        processContext.restartOnEvent(isA(Collection.class), eq(DialPlanActivatedEvent.class));
        expectLastCall();

        replay(processContext);

        m_firstRun.setProcessContext(processContext);
        m_firstRun.runTask();

        verify(processContext);

        Location primaryLocation = m_locationsManager.getPrimaryLocation();
        Collection<LocationSpecificService> servicesForPrimaryLocation = primaryLocation.getServices();
        assertFalse(servicesForPrimaryLocation.isEmpty());
        setDirty(m_firstRun);
    }
}
