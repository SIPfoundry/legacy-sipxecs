/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin;

import java.util.Collection;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.commserver.RestartNeededService;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class CertificateManagerTestIntegration extends IntegrationTestCase {
    private CertificateManagerImpl m_certificateManagerImpl;
    private SipxProcessContext m_sipxProcessContext;
    private SipxServiceManager m_sipxServiceManager;

    @Override
    protected void onSetUpInTransaction() throws Exception {
        m_certificateManagerImpl.setLibExecDirectory(TestUtil.getTestSourceDirectory(this.getClass()));
        m_certificateManagerImpl.setProcessContext(m_sipxProcessContext);
        m_certificateManagerImpl.setSipxServiceManager(m_sipxServiceManager);
    }

    public void setCertificateManagerImpl(CertificateManagerImpl certificateManagerImpl) {
        m_certificateManagerImpl = certificateManagerImpl;
    }

    public void setSipxProcessContext(SipxProcessContext sipxProcessContext) {
        m_sipxProcessContext = sipxProcessContext;
    }

    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    /**
     * Only services from primary location are prompted to restart
     * @throws Exception
     */
    public void testMarkForRestartAll() throws Exception {
        loadDataSetXml("admin/commserver/clearLocations.xml");
        loadDataSetXml("admin/caLocationsAndServices1.xml");
        m_sipxProcessContext.clear();
        m_certificateManagerImpl.generateKeyStores();
        Collection<RestartNeededService> servicesToRestart = m_sipxProcessContext.getRestartNeededServices();
        assertEquals(1, servicesToRestart.size());
    }

    public void testMarkForRestart() throws Exception {
        loadDataSetXml("admin/commserver/clearLocations.xml");
        loadDataSetXml("admin/caLocationsAndServices2.xml");
        m_sipxProcessContext.clear();
        m_certificateManagerImpl.generateKeyStores();
        Collection<RestartNeededService> servicesToRestart = m_sipxProcessContext.getRestartNeededServices();
        assertEquals(1, servicesToRestart.size());
    }
}
