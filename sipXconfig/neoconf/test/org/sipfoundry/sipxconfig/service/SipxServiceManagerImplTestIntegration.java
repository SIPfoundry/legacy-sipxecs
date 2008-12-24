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

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.same;
import static org.easymock.EasyMock.verify;

public class SipxServiceManagerImplTestIntegration extends IntegrationTestCase {

    private SipxServiceManagerImpl m_out;
    private ModelFilesContext m_modelFilesContext;
    private LocationsManager m_locationsManager;

    public void testGetServiceByBeanId() {
        SipxService service = m_out.getServiceByBeanId(SipxProxyService.BEAN_ID);
        assertNotNull(service);
        assertEquals(SipxProxyService.BEAN_ID, service.getBeanId());
    }

    public void testGetAllServices() {
        Collection<SipxService> allServices = m_out.getServicesFromDb();
        assertNotNull(allServices);

        // There should be at least 5 services returned. This assertion means
        // we don't need to update the test each time we add a new service
        assertTrue(allServices.size() > 5);
    }

    public void testInstalledService() throws Exception {
        loadDataSetXml("service/seedLocationsAndServicesWithServiceMigration.xml");
        Location primary = m_locationsManager.getPrimaryLocation();
        assertTrue(m_out.isServiceInstalled(primary.getId(), SipxProxyService.BEAN_ID));
        assertFalse(m_out.isServiceInstalled(primary.getId(), SipxStatusService.BEAN_ID));
    }

    public void testStoreService() {
        SipxService service = m_out.getServiceByBeanId(SipxStatusService.BEAN_ID);
        service.setModelDir("sipxproxy");
        service.setModelName("sipxproxy.xml");
        service.setModelFilesContext(m_modelFilesContext);
        SipxStatusConfiguration sipxStatusConfiguration = new SipxStatusConfiguration();
        sipxStatusConfiguration.setName("status-config");
        StatusPluginConfiguration statusPluginConfiguration = new StatusPluginConfiguration();
        statusPluginConfiguration.setName("status-pluing.xml");

        service.setConfigurations(Arrays.asList(sipxStatusConfiguration, statusPluginConfiguration));

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

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
}
