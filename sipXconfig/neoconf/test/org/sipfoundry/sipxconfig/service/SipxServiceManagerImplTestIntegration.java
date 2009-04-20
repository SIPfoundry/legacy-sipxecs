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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.LoggingManager;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;

public class SipxServiceManagerImplTestIntegration extends IntegrationTestCase {

    private SipxServiceManagerImpl m_out;
    private ModelFilesContext m_modelFilesContext;
    private LocationsManager m_locationsManager;
    private SipxServiceBundle m_managementBundle;
    private SipxServiceBundle m_primarySipRouterBundle;

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

    public void testGetBundlesForLocation() throws Exception {
        loadDataSetXml("service/clear-service.xml");
        loadDataSet("service/location.db.xml");

        Location location = m_locationsManager.getLocation(1001);
        List<SipxServiceBundle> bundles = m_out.getBundlesForLocation(location);
        assertTrue(bundles.isEmpty());

        m_out.setBundlesForLocation(location, Arrays.asList(m_managementBundle, m_primarySipRouterBundle));
        assertFalse(m_out.isServiceInstalled(location.getId(), SipxAcdService.BEAN_ID));
        assertTrue(m_out.isServiceInstalled(location.getId(), SipxCallResolverService.BEAN_ID));

        bundles = m_out.getBundlesForLocation(location);
        assertEquals(2, bundles.size());
        assertTrue(bundles.contains(m_managementBundle));
        assertTrue(bundles.contains(m_primarySipRouterBundle));
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

        m_out.storeService(service);
        assertNotNull(m_out.getServiceByBeanId(SipxStatusService.BEAN_ID));
    }

    public void testStoreSupervisorLogLevel() {
        SipxSupervisorService service = (SipxSupervisorService)m_out.getServiceByBeanId(SipxSupervisorService.BEAN_ID);
        LoggingManager loggingManagerMock = createMock(LoggingManager.class);
        loggingManagerMock.getEntitiesToProcess();
        expectLastCall().andReturn(new ArrayList<LoggingEntity>()).atLeastOnce();
        replay(loggingManagerMock);
        service.setLoggingManager(loggingManagerMock);
        assertEquals("INFO", service.getLogLevel());
        service.setSipxServiceManager(m_out);
        service.setLogLevel("DEBUG");
        service = (SipxSupervisorService)m_out.getServiceByBeanId(SipxSupervisorService.BEAN_ID);
        assertEquals("DEBUG", service.getLogLevel());
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

    public void setManagementBundle(SipxServiceBundle managementBundle) {
        m_managementBundle = managementBundle;
    }

    public void setPrimarySipRouterBundle(SipxServiceBundle primarySipRouterBundle) {
        m_primarySipRouterBundle = primarySipRouterBundle;
    }
}
