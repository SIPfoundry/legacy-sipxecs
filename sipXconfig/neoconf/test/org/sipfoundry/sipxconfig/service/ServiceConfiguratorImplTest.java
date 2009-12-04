/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.service;

import static java.util.Arrays.asList;
import static java.util.Collections.singleton;

import junit.framework.TestCase;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmServerManager;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanActivationManager;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.service.SipxServiceTest.DummyConfig;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.same;
import static org.easymock.EasyMock.verify;

public class ServiceConfiguratorImplTest extends TestCase {

    public void testReplicateServiceConfigSipxService() {
        DummySipxService service = new DummySipxService();

        ConfigurationFile a = new DummyConfig("a", true);
        ConfigurationFile b = new DummyConfig("b", false);
        ConfigurationFile c = new DummyConfig("c", true);

        service.setConfigurations(asList(a, b, c));

        ServiceConfiguratorImpl sc = new ServiceConfiguratorImpl();
        SipxProcessContext pc = createMock(SipxProcessContext.class);
        SipxReplicationContext rc = createMock(SipxReplicationContext.class);

        rc.replicate(same(a));
        rc.replicate(same(b));
        rc.replicate(same(c));

        pc.markServicesForRestart(singleton(service));

        replay(pc, rc);

        sc.setSipxProcessContext(pc);
        sc.setSipxReplicationContext(rc);

        sc.replicateServiceConfig(service);

        verify(pc, rc);
        assertEquals("replicated", service.getTestString());
    }

    public void testReplicateServiceConfigSipxServiceLocation() {
        Location location = new Location();
        location.setRegistered(true);

        DummySipxService service = new DummySipxService();

        ConfigurationFile a = new DummyConfig("a", true);
        ConfigurationFile b = new DummyConfig("b", false);
        ConfigurationFile c = new DummyConfig("c", true);

        service.setConfigurations(asList(a, b, c));

        ServiceConfiguratorImpl sc = new ServiceConfiguratorImpl();
        SipxProcessContext pc = createMock(SipxProcessContext.class);
        SipxReplicationContext rc = createMock(SipxReplicationContext.class);
        ConfigVersionManager cvm = createMock(ConfigVersionManager.class);

        rc.replicate(same(location), same(a));
        rc.replicate(same(location), same(b));
        rc.replicate(same(location), same(c));
        cvm.setConfigVersion(same(service), same(location));

        pc.markServicesForRestart(singleton(service));

        replay(pc, rc, cvm);

        sc.setSipxProcessContext(pc);
        sc.setSipxReplicationContext(rc);
        sc.setConfigVersionManager(cvm);

        sc.replicateServiceConfig(location, service);

        verify(pc, rc, cvm);
        assertEquals("replicatedOnLocation", service.getTestString());
    }

    public void testReplicateServiceConfigSipxServiceBoolean() {
        DummySipxService service = new DummySipxService();

        ConfigurationFile a = new DummyConfig("a", true);
        ConfigurationFile b = new DummyConfig("b", false);
        ConfigurationFile c = new DummyConfig("c", true);

        service.setConfigurations(asList(a, b, c));

        ServiceConfiguratorImpl sc = new ServiceConfiguratorImpl();
        SipxProcessContext pc = createMock(SipxProcessContext.class);
        SipxReplicationContext rc = createMock(SipxReplicationContext.class);
        rc.replicate(same(b));

        replay(pc, rc);

        sc.setSipxProcessContext(pc);
        sc.setSipxReplicationContext(rc);

        sc.replicateServiceConfig(service, true);

        verify(pc, rc);
        assertEquals("replicated", service.getTestString());
    }

    public void testReplicateServiceConfigLocationSipxServiceOnlyNoRestart() {
        Location location = new Location();
        location.setUniqueId();

        DummySipxService service = new DummySipxService();

        ConfigurationFile a = new DummyConfig("a", true);
        ConfigurationFile b = new DummyConfig("b", false);
        ConfigurationFile c = new DummyConfig("c", true);

        service.setConfigurations(asList(a, b, c));

        ServiceConfiguratorImpl sc = new ServiceConfiguratorImpl();
        SipxProcessContext pc = createMock(SipxProcessContext.class);
        SipxReplicationContext rc = createMock(SipxReplicationContext.class);
        rc.replicate(same(location), same(b));

        replay(pc, rc);

        sc.setSipxProcessContext(pc);
        sc.setSipxReplicationContext(rc);

        sc.replicateServiceConfig(location, service, true);

        verify(pc, rc);
        assertEquals("replicatedOnLocation", service.getTestString());
    }

    public void testReplicateServiceConfigLocationSipxServiceAll() {
        Location location = new Location();
        location.setUniqueId();

        DummySipxService service = new DummySipxService();

        ConfigurationFile a = new DummyConfig("a", true);
        ConfigurationFile b = new DummyConfig("b", false);
        ConfigurationFile c = new DummyConfig("c", true);

        service.setConfigurations(asList(a, b, c));

        ServiceConfiguratorImpl sc = new ServiceConfiguratorImpl();
        SipxProcessContext pc = createMock(SipxProcessContext.class);
        SipxReplicationContext rc = createMock(SipxReplicationContext.class);
        rc.replicate(same(location), same(a));
        rc.replicate(same(location), same(b));
        rc.replicate(same(location), same(c));

        pc.markServicesForRestart(singleton(service));

        replay(pc, rc);

        sc.setSipxProcessContext(pc);
        sc.setSipxReplicationContext(rc);

        sc.replicateServiceConfig(location, service, false);

        verify(pc, rc);
        assertEquals("replicatedOnLocation", service.getTestString());
    }

    public void testReplicateAllServiceConfig() {

        DummySipxService service = new DummySipxService();

        Location location1 = new Location();
        Location location2 = new Location();
        Location location3 = new Location();
        location1.setRegistered(true);
        location2.setRegistered(true);
        location3.setRegistered(false);
        location1.setServices(singleton(new LocationSpecificService(service)));
        location2.setServices(singleton(new LocationSpecificService(service)));

        ConfigurationFile a = new DummyConfig("a", true);
        ConfigurationFile b = new DummyConfig("b", false);
        ConfigurationFile c = new DummyConfig("c", true);

        service.setConfigurations(asList(a, b, c));

        ServiceConfiguratorImpl sc = new ServiceConfiguratorImpl();
        SipxServiceManager sm = createMock(SipxServiceManager.class);
        SipxProcessContext pc = createMock(SipxProcessContext.class);
        SipxReplicationContext rc = createMock(SipxReplicationContext.class);
        ConfigVersionManager cvm = createMock(ConfigVersionManager.class);
        LocationsManager lm = createMock(LocationsManager.class);
        DialPlanActivationManager dm = createMock(DialPlanActivationManager.class);
        DomainManager domainManager = createMock(DomainManager.class);
        AlarmServerManager ac = createMock(AlarmServerManager.class);

        lm.getLocations();
        expectLastCall().andReturn(new Location[] {
            location1, location2, location3
        }).anyTimes();

        SipxSupervisorService sipxSupervisorService = new SipxSupervisorService();

        sipxSupervisorService.setConfigurations(asList(a, b, c));

        sm.getServiceByBeanId(SipxSupervisorService.BEAN_ID);
        expectLastCall().andReturn(sipxSupervisorService).anyTimes();

        rc.generateAll();
        expectLastCall().anyTimes();

        dm.replicateDialPlan(false);
        expectLastCall().times(1);

        rc.replicate(same(location1), same(a));
        expectLastCall().times(2);
        rc.replicate(same(location1), same(b));
        expectLastCall().times(2);
        rc.replicate(same(location1), same(c));
        expectLastCall().times(2);
        cvm.setConfigVersion(same(service), same(location1));
        cvm.setConfigVersion(same(sipxSupervisorService), same(location1));

        rc.replicate(same(location2), same(a));
        expectLastCall().times(2);
        rc.replicate(same(location2), same(b));
        expectLastCall().times(2);
        rc.replicate(same(location2), same(c));
        expectLastCall().times(2);
        cvm.setConfigVersion(same(service), same(location2));
        cvm.setConfigVersion(same(sipxSupervisorService), same(location2));

        pc.markServicesForRestart(singleton(service));
        expectLastCall().times(2);
        pc.markServicesForRestart(singleton(sipxSupervisorService));
        expectLastCall().times(2);

        domainManager.replicateDomainConfig(rc, location1);
        domainManager.replicateDomainConfig(rc, location2);

        ac.replicateAlarmServer(rc, location1);
        ac.replicateAlarmServer(rc, location2);

        replay(lm, pc, rc, cvm, dm, sm, domainManager, ac);

        sc.setSipxProcessContext(pc);
        sc.setSipxReplicationContext(rc);
        sc.setConfigVersionManager(cvm);
        sc.setLocationsManager(lm);
        sc.setDialPlanActivationManager(dm);
        sc.setSipxServiceManager(sm);
        sc.setDomainManager(domainManager);
        sc.setAlarmServerManager(ac);

        sc.replicateAllServiceConfig();

        verify(lm, pc, rc, cvm, dm, sm, domainManager, ac);
        assertEquals("replicatedOnLocation", service.getTestString());
    }

    static class DummySipxService extends SipxService {
        private String m_test = "test";

        public String getTestString() {
            return m_test;
        }

        @Override
        public void afterReplication(Location location) {
            if (location != null) {
                m_test = "replicatedOnLocation";
            } else {
                m_test = "replicated";
            }
        }
    }
}
