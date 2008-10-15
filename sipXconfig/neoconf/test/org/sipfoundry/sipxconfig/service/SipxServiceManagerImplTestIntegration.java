/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import java.util.Arrays;
import java.util.Collection;

import org.hibernate.SessionFactory;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.same;
import static org.easymock.EasyMock.verify;
import static org.easymock.EasyMock.expectLastCall;

public class SipxServiceManagerImplTestIntegration extends IntegrationTestCase {

    private SipxServiceManagerImpl m_out;
    private ModelFilesContext m_modelFilesContext;
    private SessionFactory m_sessionFactory;

    @Override
    protected void onSetUpInTransaction() {
        m_out.setSessionFactory(m_sessionFactory);
    }

    public void testGetServiceByBeanId() {
        SipxService service = m_out.getServiceByBeanId(SipxProxyService.BEAN_ID);
        assertNotNull(service);
        assertEquals(SipxProxyService.BEAN_ID, service.getBeanId());
    }

    public void testGetAllServices() {
        Collection<SipxService> allServices = m_out.getAllServices();
        assertNotNull(allServices);

        // There should be at least 5 services returned.  This assertion means
        // we don't need to update the test each time we add a new service
        assertTrue(allServices.size() > 5);
    }

    public void testStoreService() {
        SipxService service = m_out.getServiceByBeanId(SipxStatusService.BEAN_ID);
        service.setModelDir("sipxproxy");
        service.setModelName("sipxproxy.xml");
        service.setModelFilesContext(m_modelFilesContext);
        SipxStatusConfiguration sipxStatusConfiguration = new SipxStatusConfiguration() {
            @Override
            public void generate(SipxService s) {
                // do nothing
            }
        };
        sipxStatusConfiguration.setName("status-config");
        StatusPluginConfiguration statusPluginConfiguration = new StatusPluginConfiguration() {
            @Override
            public void generate(SipxService s) {
                // do nothing
            }
        };
        statusPluginConfiguration.setName("status-pluing.xml");

        service.setConfigurations(Arrays.asList(new SipxServiceConfiguration[] {sipxStatusConfiguration, statusPluginConfiguration}));

        SipxReplicationContext replicationContext = createMock(SipxReplicationContext.class);
        m_out.setSipxReplicationContext(replicationContext);
        replicationContext.replicate(same(sipxStatusConfiguration));
        expectLastCall();
        replicationContext.replicate(same(statusPluginConfiguration));
        expectLastCall();
        replay(replicationContext);

        m_out.storeService(service);
        verify(replicationContext);
    }

    public void setSipxServiceManagerImpl(SipxServiceManagerImpl sipxServiceManagerImpl) {
        m_out = sipxServiceManagerImpl;
    }

    public void setModelFilesContext(ModelFilesContext context) {
        m_modelFilesContext = context;
    }

    @Override
    public void setSessionFactory(SessionFactory sessionFactory) {
        m_sessionFactory = sessionFactory;
    }
}
