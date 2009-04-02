/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.domain;

import java.util.Collections;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.orm.hibernate3.HibernateTemplate;

import junit.framework.TestCase;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.classextension.EasyMock.createMock;
import static org.easymock.classextension.EasyMock.replay;
import static org.easymock.classextension.EasyMock.reset;
import static org.easymock.classextension.EasyMock.verify;

public class DomainManagerImplTest extends TestCase {

    public void testSaveDomain() {
        Domain domain = new Domain("goose");

        HibernateTemplate db = createMock(HibernateTemplate.class);
        db.findByNamedQuery("domain");
        expectLastCall().andReturn(Collections.EMPTY_LIST);
        db.saveOrUpdate(domain);
        db.flush();
        SipxService registrarService = new SipxRegistrarService();
        registrarService.setSettings(TestHelper.loadSettings("sipxregistrar/sipxregistrar.xml"));

        final SipxServiceManager serviceManager = EasyMock.createMock(SipxServiceManager.class);
        serviceManager.getServiceByBeanId(SipxRegistrarService.BEAN_ID);
        EasyMock.expectLastCall().andReturn(registrarService).anyTimes();

        final SipxReplicationContext replicationContext = EasyMock.createMock(SipxReplicationContext.class);
        replicationContext.replicate(null);
        EasyMock.expectLastCall().anyTimes();

        replay(db, replicationContext, serviceManager);
        // TEST DELETE EXISTING
        DomainManagerImpl mgr = new DomainManagerImpl() {
            @Override
            protected DomainConfiguration createDomainConfiguration() {
                return new DomainConfiguration();
            }

            @Override
            protected ServiceConfigurator getServiceConfigurator() {
                return createMock(ServiceConfigurator.class);
            }

            @Override
            public void replicateDomainConfig(SipxReplicationContext rc, Location location) {
                // do not replicate anything... it's just a test
            }
        };

        mgr.setHibernateTemplate(db);
        mgr.saveDomain(domain);

        verify(db);

        reset(db);

        // TEST IGNORE EXISTING (assumes there is no other, doesn't care actually)
        domain.setUniqueId(); // isNew!

        db.saveOrUpdate(domain);
        db.flush();
        replay(db);

        mgr.saveDomain(domain);

        verify(db);
    }
}
