/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd;

import java.io.Serializable;

import static java.util.Collections.singleton;

import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxAcdService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcProxyFactoryBean;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;


public class AcdProvisioningContextImpl extends HibernateDaoSupport implements
        AcdProvisioningContext {
    private SipxReplicationContext m_sipxReplicationContext;
    private ServiceConfigurator m_serviceConfigurator;
    private SipxServiceManager m_sipxServiceManager;

    private JobContext m_jobContext;

    public void deploy(Serializable id) {
        boolean success = false;
        AcdServer server = (AcdServer) getHibernateTemplate().load(AcdServer.class, id);
        SipxService acdService = getAcdService();
        Serializable jobId = m_jobContext.schedule("ACD Server Configuration");
        try {
            // ENG-494 very first command in try block to ensure job state machine
            // goes from start --> (failure|success)
            m_jobContext.start(jobId);
            // TODO: it would be nice if we could use Spring to set it up somehow
            XmlRpcProxyFactoryBean factory = new XmlRpcProxyFactoryBean();
            factory.setServiceInterface(Provisioning.class);
            factory.setServiceUrl(server.getServiceUri());
            factory.afterPropertiesSet();
            Provisioning provisioning = (Provisioning) factory.getObject();
            XmlRpcSettings xmlRpc = new XmlRpcSettings(provisioning);
            server.deploy(xmlRpc);
            success = true;
            m_sipxReplicationContext.generate(DataSet.ALIAS);
        } finally {
            //XML-RPC deploy operation doesn't automatically restart the acd service
            //Make sure that the acd service is marked for restart
            m_serviceConfigurator.markServiceForRestart(server.getLocation(),
                    singleton(acdService));
            if (success) {
                m_jobContext.success(jobId);
            } else {
                m_jobContext.failure(jobId, null, null);
            }
        }
    }

    private SipxService getAcdService() {
        return m_sipxServiceManager.getServiceByBeanId(SipxAcdService.BEAN_ID);
    }

    @Required
    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }

    @Required
    public void setServiceConfigurator(ServiceConfigurator serviceConfigurator) {
        m_serviceConfigurator = serviceConfigurator;
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    @Required
    public void setJobContext(JobContext jobContext) {
        m_jobContext = jobContext;
    }
}
