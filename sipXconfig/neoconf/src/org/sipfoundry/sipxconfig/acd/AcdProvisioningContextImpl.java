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
import java.net.MalformedURLException;

import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcProxyFactoryBean;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class AcdProvisioningContextImpl extends HibernateDaoSupport implements
        AcdProvisioningContext {
    private SipxReplicationContext m_sipxReplicationContext;

    private JobContext m_jobContext;

    public void deploy(Serializable id) {
        boolean success = false;
        Serializable jobId = m_jobContext.schedule("ACD Server Configuration");
        try {
            // ENG-494 very first command in try block to ensure job state machine
            // goes from start --> (failure|success)
            m_jobContext.start(jobId);
            
            AcdServer server = (AcdServer) getHibernateTemplate().load(AcdServer.class, id);
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
        } catch (MalformedURLException e) {
            throw new RuntimeException(e);
        } finally {
            if (success) {
                m_jobContext.success(jobId);
            } else {
                m_jobContext.failure(jobId, null, null);
            }
        }
    }

    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }

    public void setJobContext(JobContext jobContext) {
        m_jobContext = jobContext;
    }
}
