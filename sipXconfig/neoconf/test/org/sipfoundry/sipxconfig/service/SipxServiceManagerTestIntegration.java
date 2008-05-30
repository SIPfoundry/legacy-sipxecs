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

import org.easymock.EasyMock;
import org.hibernate.SessionFactory;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;

public class SipxServiceManagerTestIntegration extends IntegrationTestCase {

    private SipxServiceManagerImpl m_out;
    private ModelFilesContext m_modelFilesContext;
    private SipxReplicationContext m_replicationContext;
    private SessionFactory m_sessionFactory;

    protected void init() {
        m_out = new SipxServiceManagerImpl();
        m_out.setSessionFactory(m_sessionFactory);
        SipxServiceManagerImpl impl = (SipxServiceManagerImpl)m_out;
        m_replicationContext = EasyMock.createMock(SipxReplicationContext.class);
        impl.setSipxReplicationContext(m_replicationContext);
    }

    public void testGetServiceByBeanId() {
        init();
        SipxService service = m_out.getServiceByBeanId(SipxProxyService.BEAN_ID);
        assertNotNull(service);
        assertEquals(SipxProxyService.BEAN_ID, service.getBeanId());
    }

    public void testStoreService() {
        init();
        SipxService service = m_out.getServiceByBeanId(SipxProxyService.BEAN_ID);
        service.setModelDir("sipxproxy");
        service.setModelName("sipxproxy.xml");
        service.setModelFilesContext(m_modelFilesContext);
        service.setConfiguration(new SipxProxyConfiguration(){
            public void generate(SipxService service) {
                // do nothing
            }
        });
        m_replicationContext.replicate(service.getConfiguration());
        EasyMock.replay(m_replicationContext);
        
        service.getSettings().getSetting("proxy-configuration").getSetting("SIPX_PROXY_DEFAULT_SERIAL_EXPIRES").setValue("99");
        
        m_out.storeService(service);
        EasyMock.verify(m_replicationContext);
    }
    
    public void setModelFilesContext(ModelFilesContext context) {
        m_modelFilesContext = context;
    }
    
    public void setSessionFactory(SessionFactory sessionFactory) {
        m_sessionFactory = sessionFactory;
    }
}
