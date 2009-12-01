/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.conference;

import org.sipfoundry.sipxconfig.service.SipxImbotService;

import junit.framework.TestCase;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.service.SipxIvrService;
import org.sipfoundry.sipxconfig.service.SipxRecordingService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.test.TestUtil;
import org.springframework.orm.hibernate3.HibernateTemplate;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.classextension.EasyMock.createMock;
import static org.easymock.classextension.EasyMock.createNiceMock;
import static org.easymock.classextension.EasyMock.replay;
import static org.easymock.classextension.EasyMock.verify;

public class ConferenceBridgeProvisioningtImplTest extends TestCase {
    public void testGenerateConfigurationData() throws Exception {
        final Location location = new Location();
        location.setFqdn("conf.example.com");

        final SipxFreeswitchService service = createNiceMock(SipxFreeswitchService.class);
        service.getBeanId();
        expectLastCall().andReturn(SipxFreeswitchService.BEAN_ID);
        service.reloadXml(location);
        expectLastCall().andReturn(true);

        SipxService ivrService = new SipxIvrService();
        ivrService.setBeanId(SipxIvrService.BEAN_ID);

        SipxService recordingService = new SipxRecordingService();
        recordingService.setBeanId(SipxRecordingService.BEAN_ID);

        SipxService imBotService = new SipxImbotService();
        imBotService.setBeanId(SipxImbotService.BEAN_ID);

        Bridge bridge = new Bridge() {
            @Override
            public Location getLocation() {
                return location;
            }

            @Override
            public SipxFreeswitchService getFreeswitchService() {
                return service;
            }
        };

        HibernateTemplate ht = createMock(HibernateTemplate.class);
        ht.load(Bridge.class, 0);
        expectLastCall().andReturn(bridge);

        ServiceConfigurator sc = createMock(ServiceConfigurator.class);
        sc.replicateServiceConfig(location, service, true);
        sc.replicateServiceConfig(ivrService, true);
        sc.replicateServiceConfig(location, recordingService, true);
        sc.replicateServiceConfig(location, imBotService, true);

        SipxReplicationContext rc = createMock(SipxReplicationContext.class);
        rc.generate(DataSet.ALIAS);

        replay(ht, rc, sc, service);
        SipxServiceManager sm = TestUtil.getMockSipxServiceManager(true, service, ivrService, recordingService, imBotService);

        ConferenceBridgeProvisioningImpl impl = new ConferenceBridgeProvisioningImpl();
        impl.setHibernateTemplate(ht);
        impl.setReplicationContext(rc);
        impl.setServiceConfigurator(sc);
        impl.setSipxServiceManager(sm);

        impl.deploy(0);

        verify(ht, rc, sc, service);
    }
}
