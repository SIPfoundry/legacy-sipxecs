/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.conference;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.springframework.orm.hibernate3.HibernateTemplate;

import junit.framework.TestCase;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.isA;
import static org.easymock.EasyMock.same;
import static org.easymock.classextension.EasyMock.createMock;
import static org.easymock.classextension.EasyMock.replay;
import static org.easymock.classextension.EasyMock.verify;

public class ConferenceBridgeProvisioningtImplTest extends TestCase {
    public void testGenerateConfigurationData() throws Exception {
        final Location location = new Location();
        location.setFqdn("conf.example.com");

        final SipxFreeswitchService service = createMock(SipxFreeswitchService.class);
        service.reloadXml(location);
        expectLastCall().andReturn(true);

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

        SipxReplicationContext rc = createMock(SipxReplicationContext.class);
        rc.replicate(same(location), isA(ConferenceConfiguration.class));
        rc.generate(DataSet.ALIAS);

        replay(ht, rc, sc, service);

        ConferenceBridgeProvisioningImpl impl = new ConferenceBridgeProvisioningImpl() {
            @Override
            public ConferenceConfiguration createConferenceConfiguration() {
                return new ConferenceConfiguration();
            }
        };

        impl.setHibernateTemplate(ht);
        impl.setReplicationContext(rc);
        impl.setServiceConfigurator(sc);

        impl.deploy(0);

        verify(ht, rc, sc, service);
    }
}
