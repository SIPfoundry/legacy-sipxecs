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
import java.util.Collections;

import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.RunRequest;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.sipfoundry.sipxconfig.presence.PresenceServer;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcProxyFactoryBean;
import org.springframework.beans.factory.annotation.Required;

public class AcdProvisioningContextImpl implements AcdProvisioningContext {
    private JobContext m_jobContext;
    private ConfigManager m_configManager;
    private AcdContext m_acdContext;
    private AddressManager m_addressManager;

    public void deploy(Serializable id) {
        boolean success = false;
        AcdServer server = m_acdContext.loadServer(id);
        Serializable jobId = m_jobContext.schedule("ACD Server Configuration");
        try {
            // ENG-494 very first command in try block to ensure job state machine
            // goes from start --> (failure|success)
            m_jobContext.start(jobId);
            // TODO: it would be nice if we could use Spring to set it up somehow
            XmlRpcProxyFactoryBean factory = new XmlRpcProxyFactoryBean();
            factory.setServiceInterface(Provisioning.class);
            String serviceUri = m_addressManager.getSingleAddress(PresenceServer.HTTP_ADDRESS).toString();
            factory.setServiceUrl(serviceUri);
            factory.afterPropertiesSet();
            Provisioning provisioning = (Provisioning) factory.getObject();
            XmlRpcSettings xmlRpc = new XmlRpcSettings(provisioning);
            server.deploy(xmlRpc);
            success = true;
            m_configManager.configureEverywhere(Acd.FEATURE);
        } finally {
            // XML-RPC deploy operation doesn't automatically restart the acd service
            // Make sure that the acd service is marked for restart
            RunRequest r = new RunRequest("restart ACD", Collections.singleton(server.getLocation()));
            r.setDefines("restart_sipxacd");
            m_configManager.run(r);
            if (success) {
                m_jobContext.success(jobId);
            } else {
                m_jobContext.failure(jobId, null, null);
            }
        }
    }

    @Required
    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    @Required
    public void setJobContext(JobContext jobContext) {
        m_jobContext = jobContext;
    }

    public void setAcdContext(AcdContext acdContext) {
        m_acdContext = acdContext;
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }
}
