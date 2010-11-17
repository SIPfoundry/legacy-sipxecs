/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.conference;

import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.classextension.EasyMock.createMock;
import static org.easymock.classextension.EasyMock.createNiceMock;
import static org.easymock.classextension.EasyMock.replay;
import static org.easymock.classextension.EasyMock.verify;

import java.util.ArrayList;
import java.util.Collections;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.service.LocationSpecificService;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.ServiceConfiguratorImpl;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.service.SipxImbotService;
import org.sipfoundry.sipxconfig.service.SipxIvrService;
import org.sipfoundry.sipxconfig.service.SipxRecordingService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class ConferenceBridgeProvisioningtImplTest extends TestCase {
    public void testGenerateConfigurationData() throws Exception {
        final Location location = new Location();
        location.setFqdn("conf.example.com");

        final SipxFreeswitchService service = createNiceMock(SipxFreeswitchService.class);
        service.getBeanId();
        expectLastCall().andReturn(SipxFreeswitchService.BEAN_ID);

        final LocationSpecificService locationService = createNiceMock(LocationSpecificService.class);

        SipxService ivrService = new SipxIvrService();
        ivrService.setBeanId(SipxIvrService.BEAN_ID);

        SipxService recordingService = new SipxRecordingService();
        recordingService.setBeanId(SipxRecordingService.BEAN_ID);

        SipxService imBotService = new SipxImbotService();
        imBotService.setBeanId(SipxImbotService.BEAN_ID);

        SipxProcessContext processContext = createNiceMock(SipxProcessContext.class);
        processContext.markServicesForReload(Collections.singleton(service));

        Bridge bridge = new Bridge() {
            @Override
            public Location getLocation() {
                return location;
            }

            @Override
            public SipxFreeswitchService getFreeswitchService() {
                return service;
            }

            @Override
            public LocationSpecificService getService() {
                return locationService;
            }
        };

        ServiceConfigurator sc = createMock(ServiceConfigurator.class);
        sc.replicateServiceConfig(location, service, true, false);
        processContext.markServicesForReload(Collections.singleton(service));
        sc.replicateServiceConfig(ivrService, true);
        sc.replicateServiceConfig(location, recordingService, true);
        sc.replicateServiceConfig(location, imBotService, true);

        SipxReplicationContext rc = createMock(SipxReplicationContext.class);
        rc.generate(DataSet.ALIAS);

        replay(rc, sc, service, processContext);
        SipxServiceManager sm = TestUtil.getMockSipxServiceManager(true, service, ivrService, recordingService, imBotService);

        ConferenceBridgeProvisioningImpl impl = new ConferenceBridgeProvisioningImpl();
        impl.setReplicationContext(rc);
        impl.setServiceConfigurator(sc);
        impl.setSipxServiceManager(sm);
        impl.setSipxProcessContext(processContext);

        impl.deploy(bridge);

        verify(rc, sc, service);
    }

    public void testReloadXml() {
        Location location = new Location();

        SipxFreeswitchService service = createNiceMock(SipxFreeswitchService.class);
        expect(service.getConfigurations(true)).andReturn(new ArrayList());
        service.afterReplication(location);
        expectLastCall().once();
        replay(service);

        SipxProcessContext sp = createNiceMock(SipxProcessContext.class);

        ServiceConfiguratorImpl sc = new ServiceConfiguratorImpl();
        sc.setSipxProcessContext(sp);
        sc.replicateServiceConfig(location, service, true);

        verify(service);
    }
}
