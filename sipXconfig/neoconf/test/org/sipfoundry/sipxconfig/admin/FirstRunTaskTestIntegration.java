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
import java.util.Collections;
import java.util.Set;

import org.apache.commons.codec.binary.Base64;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.Process;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Command;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessModel.ProcessName;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanActivatedEvent;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.common.AlarmContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.service.LocationSpecificService;

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
        DomainManager domainManager = EasyMock.createNiceMock(DomainManager.class);
        m_firstRun.setDomainManager(domainManager);
        AdminContext adminContext = EasyMock.createNiceMock(AdminContext.class);
        m_firstRun.setAdminContext(adminContext);
        DialPlanContext dialPlanContext = EasyMock.createNiceMock(DialPlanContext.class);
        m_firstRun.setDialPlanContext(dialPlanContext);
        CoreContext coreContext = EasyMock.createNiceMock(CoreContext.class);
        m_firstRun.setCoreContext(coreContext);
        AlarmContext alarmContext = EasyMock.createNiceMock(AlarmContext.class);
        m_firstRun.setAlarmContext(alarmContext);

        EasyMock.replay(domainManager, adminContext, dialPlanContext, coreContext, alarmContext);

        loadDataSetXml("admin/commserver/seedLocationsAndServices.xml");
        m_firstRun.setLocationsManager(m_locationsManager);

        SipxProcessContext processContext = EasyMock.createMock(SipxProcessContext.class);
        processContext.getRestartable();
        EasyMock.expectLastCall().andReturn(Collections.EMPTY_LIST);
        processContext.restartOnEvent(EasyMock.isA(Collection.class), EasyMock
                .eq(DialPlanActivatedEvent.class));
        EasyMock.expectLastCall();
        processContext.getProcess(EasyMock.isA(ProcessName.class));
        EasyMock.expectLastCall().andReturn(new Process()).anyTimes();
        processContext.manageServices(EasyMock.eq(m_locationsManager.getPrimaryLocation()),
                EasyMock.isA(Collection.class), EasyMock.eq(Command.START));
        EasyMock.expectLastCall();
        EasyMock.replay(processContext);
        m_firstRun.setProcessContext(processContext);

        m_firstRun.runTask();
        EasyMock.verify(processContext);

        Location primaryLocation = m_locationsManager.getPrimaryLocation();
        Collection<LocationSpecificService> servicesForPrimaryLocation = primaryLocation.getServices();
        for (LocationSpecificService locationSpecificService : servicesForPrimaryLocation) {
            assertFalse(locationSpecificService.getEnableOnNextUpgrade());
        }
        setDirty(m_firstRun);
    }

}
