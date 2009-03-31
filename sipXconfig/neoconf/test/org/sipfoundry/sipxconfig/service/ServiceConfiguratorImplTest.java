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

import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.service.SipxServiceTest.DummyConfig;

import junit.framework.TestCase;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.same;
import static org.easymock.EasyMock.verify;

public class ServiceConfiguratorImplTest extends TestCase {

    public void testReplicateServiceConfigSipxService() {
        SipxService service = new SipxService() {
        };

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
    }

    public void testReplicateServiceConfigSipxServiceLocation() {
        Location location = new Location();
        location.setRegistered(true);

        SipxService service = new SipxService() {
        };

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
    }

    public void testReplicateServiceConfigSipxServiceBoolean() {
        SipxService service = new SipxService() {
        };

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
    }
}
