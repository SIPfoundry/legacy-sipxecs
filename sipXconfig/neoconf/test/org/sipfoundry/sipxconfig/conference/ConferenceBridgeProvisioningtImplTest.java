/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.conference;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.classextension.EasyMock.createMock;
import static org.easymock.classextension.EasyMock.replay;
import static org.easymock.classextension.EasyMock.verify;
import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.service.SipxIvrService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.orm.hibernate3.HibernateTemplate;

public class ConferenceBridgeProvisioningtImplTest extends TestCase {
    public void testGenerateConfigurationData() throws Exception {
        final Location location = new Location();
        location.setFqdn("conf.example.com");

        final SipxFreeswitchService service = createMock(SipxFreeswitchService.class);
        service.reloadXml(location);
        expectLastCall().andReturn(true);

        SipxIvrService ivrService = new SipxIvrService();
        ivrService.setBeanId(SipxIvrService.BEAN_ID);

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

        SipxReplicationContext rc = createMock(SipxReplicationContext.class);
        rc.generate(DataSet.ALIAS);

        SipxServiceManager sm = createMock(SipxServiceManager.class);
        sm.isServiceInstalled(SipxIvrService.BEAN_ID);
        expectLastCall().andReturn(true);
        sm.getServiceByBeanId(SipxIvrService.BEAN_ID);
        expectLastCall().andReturn(ivrService);

        replay(ht, rc, sc, sm, service);

        ConferenceBridgeProvisioningImpl impl = new ConferenceBridgeProvisioningImpl();
        impl.setHibernateTemplate(ht);
        impl.setReplicationContext(rc);
        impl.setServiceConfigurator(sc);
        impl.setSipxServiceManager(sm);

        impl.deploy(0);

        verify(ht, rc, sc, service);
    }
}
